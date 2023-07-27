#include "server.h"
#include "thread_pool.hpp"
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
namespace fs = boost::filesystem;
/* create thread pool, set up server and call handleRequest once receive request
 * from client */
void Server::setupServe() {
    // set up server
    int server_fd;
    try {
        server_fd = startServer();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        closefd(server_fd);
        return;
    }
    db.initDB("Table_setup.sql");
    boost::asio::thread_pool pool(4);
    while (true) {
        int client_fd = acceptConnection(server_fd);
        try {
            while (true){
                // vector<char> buffer(MAX_RECEIVE, 0);
                // int len = recv(client_fd, buffer.data(), MAX_RECEIVE, 0);
                // if (len <= 0) {
                //     close(client_fd);
                //     throw myException("Receive next client.");
                // }
                // string temp(buffer.data(), len);
                string temp = receiveReq(client_fd);
                // get rid of first line
                size_t pos = temp.find('\n');
                string xmlReq = temp.substr(pos + 1);
                //cout << "receive request:" << xmlReq << endl;
                Args * arg = new Args(xmlReq, client_fd);
                auto postHandler = [this, &arg, client_fd]() -> void {
                    handleRequest(arg);
                };
                boost::asio::post(pool, postHandler);
            }
        } catch (const exception &e) {
            cerr << e.what() << endl;
            continue;
        }
    }
}

/* Handle one request: process raw request, execute, convert string to XML and
 * sent to client */
void Server::handleRequest(void * info) {
    Args * arg = (Args *)info;
    vector<Request *> reqlines;
    string rawRequest = arg->xml;
    int socket_fd = arg->socket_fd;
    //cout << rawRequest << endl;
    try {
        processXML(rawRequest, reqlines);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return;
    }
    // execute: put class into database, match, generate response
    XMLDocument response;
    XMLDeclaration *declaration = response.NewDeclaration();
    response.InsertFirstChild(declaration);
    // generate root :<results> </results>
    XMLElement *root = response.NewElement("results");
    response.InsertEndChild(root);
    auto it = reqlines.begin();
    while (it != reqlines.end()) {
        Request *req = *it;
        req->execute(response, db);
        it++;
    }
    // convert xml to string
    XMLPrinter printer;
    response.Print(&printer);
    std::string xmlString = printer.CStr();
    //response.Print();
    // send back response
    try {
        sendRes(socket_fd, xmlString);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        closefd(socket_fd);
        return;
    }
}
void Server::sendRes(int socket_fd, string response) {
    int len = send(socket_fd, response.c_str(), response.length(), 0);
    if (len <= 0) {
        throw myException("Error: cannot send response.");
    }
}

string Server::receiveReq(int socket_fd) {
    vector<char> buffer(MAX_RECEIVE, 0);
    string wholeReq;
    // receive the first part
    int len = recv(socket_fd, buffer.data(), MAX_RECEIVE, 0);
    if (len <= 0) {
        close(socket_fd);
        throw myException("Receive next client.");
    }
    string partReq(buffer.data(), len);
    wholeReq = partReq;
    // get total len of message from client
    size_t pos = partReq.find('\n');
    int totoalLen = stoi(partReq.substr(0, pos));
    int remainLen = totoalLen - partReq.length();
    while (remainLen > 0) {
        int len1 = recv(socket_fd, buffer.data(), MAX_RECEIVE, 0);
        if (len1 <= 0) {
            close(socket_fd);
            throw myException("Error: cannot accept request.");
        }
        string tmp(buffer.data(), len1);
        wholeReq += tmp;
        remainLen = totoalLen - wholeReq.length();
    }
    return wholeReq;
}

/* process raw request, convert to class, put requests to request vector*/
void Server::processXML(string &rawRequest, vector<Request *> &requests) {
    XMLDocument xml;
    if (xml.Parse(rawRequest.c_str()) != XML_SUCCESS) {
        throw myException("Error: cannot parse XML string.");
    }
    XMLElement *root = xml.RootElement();
    if (root == nullptr) {
        throw myException("Error: cannot get root element.");
    }
    const char *rootName = root->Name();
    // cout << "xml rootname: " << rootName << endl;
    if (strcmp(rootName, "create") == 0) {
        XMLElement *curr = root->FirstChildElement();
        while (curr != nullptr) {
            if (strcmp(curr->Name(), "account") == 0) {
                int id = stoi(curr->Attribute("id"));
                double balance = stod(curr->Attribute("balance"));
                Account *newAccount = new Account(id, balance);
                requests.push_back(newAccount);
            } else if (strcmp(curr->Name(), "symbol") == 0) {
                string sym(curr->Attribute("sym"));
                XMLElement *currSym = curr->FirstChildElement();
                while (currSym != nullptr) {
                    if (strcmp(currSym->Name(), "account") == 0) {
                        int accountId = stoi(currSym->Attribute("id"));
                        double amount = stod(currSym->GetText());
                        Position *newPosition =
                            new Position(accountId, sym, amount);
                        requests.push_back(newPosition);
                    } else {
                        throw myException("Error: invalid request.");
                    }
                    currSym = currSym->NextSiblingElement();
                }
            } else {
                throw myException("Error: invalid request.");
            }
            curr = curr->NextSiblingElement();
        }
    } else if (strcmp(rootName, "transactions") == 0) {
        int account_id = stoi(root->Attribute("id"));
        XMLElement *curr = root->FirstChildElement();
        while (curr != nullptr) {
            if (strcmp(curr->Name(), "order") == 0) {
                string sym(curr->Attribute("sym"));
                double amount = stod(curr->Attribute("amount"));
                double limit = stod(curr->Attribute("limit"));
                // cout << "order sym: " << sym << endl;
                OpenOrder *newOrder =
                    new OpenOrder(account_id, sym, amount, limit);
                requests.push_back(newOrder);
            } else if (strcmp(curr->Name(), "query") == 0) {
                int transId = stoi(curr->Attribute("id"));
                Query *aQuery = new Query(account_id, transId);
                requests.push_back(aQuery);
            } else if (strcmp(curr->Name(), "cancel") == 0) {
                int transId = stoi(curr->Attribute("id"));
                Cancel *aCancel = new Cancel(account_id, transId);
                requests.push_back(aCancel);
            } else {
                throw myException("Error: invalid request.");
            }
            curr = curr->NextSiblingElement();
        }
    } else {
        throw myException("Error: invalid request.");
    }
    // cout << "finish process xml " << endl;
    // for(int i = 0; i < requests.size(); i++){
    //     requests[i]->print();
    // }
    //cout << "finish process" << endl;
}

int main() {
    Server myServer;
    myServer.setupServe();
    return EXIT_SUCCESS;
}