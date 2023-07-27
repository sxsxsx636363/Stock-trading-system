#include "tinyxml2.h"
#include <iostream>
#include "exception.h"
#include "request.h"
#include "vector"
using namespace tinyxml2;

int main() {
    XMLDocument xml;
    xml.LoadFile("test_database.xml");
    vector<Request*> requests;
    XMLElement* root = xml.RootElement();
    if (root == nullptr) {
        throw myException("Error: cannot get root element.");
    }
    const char* rootName = root->Name();
    cout << "xml rootname: " << rootName << endl;
    if (strcmp(rootName, "create") == 0){
        XMLElement* curr = root->FirstChildElement();
        while (curr != nullptr){
            if(strcmp(curr->Name(), "account") == 0){
                int id = stoi(curr->Attribute("id"));
                double balance = stod(curr->Attribute("balance"));
                cout << "account id: " << id << endl;
                cout << "account balance: " << balance << endl;
                Account* newAccount = new Account(id, balance);
                requests.push_back(newAccount);
            }else if(strcmp(curr->Name(), "symbol") == 0){
                string sym(curr->Attribute("sym"));
                XMLElement* currSym = curr->FirstChildElement();
                while(currSym!= nullptr){
                    if(strcmp(currSym->Name(), "account") == 0){
                        int accountId = stoi(currSym->Attribute("id"));
                        double amount = stod(currSym->GetText());
                        cout << "symbol name: " << sym << endl;
                        cout << "symbol id: " << accountId << endl;
                        cout << "symbol amount: " << amount << endl;
                        Position* newPosition = new Position(accountId, sym, amount);
                        requests.push_back(newPosition);
                    }else{
                        throw myException("Error: invalid request.");
                    }
                    currSym =currSym->NextSiblingElement();
                }
            }else{
                throw myException("Error: invalid request.");
            }
            curr = curr->NextSiblingElement();
        }
    }else if(strcmp(rootName, "transactions") == 0){
        int account_id = stoi(root->Attribute("id"));
        XMLElement* curr = root->FirstChildElement();
        while (curr != nullptr){
            if(strcmp(curr->Name(), "order") == 0){
                string sym(curr->Attribute("sym"));
                double amount = stod(curr->Attribute("amount"));
                double limit = stod(curr->Attribute("limit"));
                cout << "order sym: " << sym << endl;
                cout << "order amount: " << amount << endl;
                cout << "order limit: " << limit << endl;
                Order* newOrder = new Order(account_id, sym, amount, limit);
                requests.push_back(newOrder);
            }else if(strcmp(curr->Name(), "query") == 0){
                int transId = stoi(curr->Attribute("id"));
                Query* aQuery = new Query(account_id, transId);
                cout << "query transId: " << transId << endl;
                requests.push_back(aQuery);
            }else if(strcmp(curr->Name(), "cancel") == 0){
                int transId = stoi(curr->Attribute("id"));
                Cancel* aCancel = new Cancel(account_id, transId);
                cout << "cancel transId: " << transId << endl;
                requests.push_back(aCancel);
            }else{
                throw myException("Error: invalid request.");
            }
            curr = curr->NextSiblingElement();
        }
    }else{
        throw myException("Error: invalid request.");
    }
}
