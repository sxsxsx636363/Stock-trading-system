#include "client.h"
#include <fstream>
#include <iostream>
#include <sstream>

ostringstream Client::createAccount(int account_id, double balance,
                                    string symbol_name, double amount) {

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration *decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);
    tinyxml2::XMLElement *root = doc.NewElement("create");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement *account1 = doc.NewElement("account");
    account1->SetAttribute("id", account_id);
    account1->SetAttribute("balance", balance);
    root->InsertEndChild(account1);

    tinyxml2::XMLElement *symbol = doc.NewElement("symbol");
    symbol->SetAttribute("sym", symbol_name.c_str());
    root->InsertEndChild(symbol);

    tinyxml2::XMLElement *account2 = doc.NewElement("account");
    account2->SetAttribute("id", account_id);
    account2->SetText(amount);
    symbol->InsertEndChild(account2);

    // Convert XML document to string
    std::ostringstream oss;
    tinyxml2::XMLPrinter printer;
    doc.Print(&printer);
    oss << printer.CStr();

    // Get length of output string
    std::string outputStr = oss.str();
    int outputLength = outputStr.length();

    // Add length to first line of output XML
    std::ostringstream outputOss;
    outputOss << outputLength << " \n" << outputStr;

    // Write output to file
    // std::ofstream output("outputCreate.xml");
    // output << outputOss.str();
    // output.close();
    //  tinyxml2::XMLDocument new_doc;
    //  new_doc.Parse(outputOss.str().c_str());
    //  new_doc.SaveFile("new_doc.xml");
    return outputOss;
}

ostringstream Client::createTransaction(int account_id, string symbol_name,
                                        double amount, double limit) {

    // Create XML document
    tinyxml2::XMLDocument doc;

    // Create root element
    tinyxml2::XMLElement *transactions = doc.NewElement("transactions");
    transactions->SetAttribute("id", account_id);
    doc.InsertEndChild(transactions);

    // Create child element 'order'
    tinyxml2::XMLElement *order = doc.NewElement("order");
    order->SetAttribute("sym", symbol_name.c_str());
    order->SetAttribute("amount", amount);
    order->SetAttribute("limit", limit);
    transactions->InsertEndChild(order);

    // Create child element 'query'
    tinyxml2::XMLElement *query = doc.NewElement("query");
    query->SetAttribute("id", account_id);
    transactions->InsertEndChild(query);

    // Create child element 'cancel'
    tinyxml2::XMLElement *cancel = doc.NewElement("cancel");
    cancel->SetAttribute("id", account_id);
    transactions->InsertEndChild(cancel);

    // Convert XML document to string
    std::ostringstream oss;
    tinyxml2::XMLPrinter printer;
    doc.Print(&printer);
    oss << printer.CStr();

    // Get length of output string
    std::string outputStr = oss.str();
    int outputLength = outputStr.length();

    // Add length to first line of output XML
    std::ostringstream outputOss;
    outputOss << outputLength << " \n" << outputStr;

    // Write output to file
    // std::ofstream output("outputTrans.xml");
    // output << outputOss.str();
    // output.close();
    //  tinyxml2::XMLDocument new_doc;
    //  new_doc.Parse(outputOss.str().c_str());
    //  new_doc.SaveFile("new_doc.xml");
    return outputOss;
}

void Client::runClient(const char *hostname, int test_num) { // test scalability
    int client_fd;
    try {
        client_fd = startClient(hostname, SERVERPORT);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        closefd(client_fd);
        return;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    for (int i = 1; i <= test_num; i++) {
        /* ----------generate account-----------  */
        ostringstream outputOss =
            createAccount(i, 100.5, "zhumi", 10.0);
        string xmlString = outputOss.str();
        // cout << "Send to server:" << endl;
        // cout << xmlString << endl;

        // send to server
        int len = send(client_fd, xmlString.c_str(), xmlString.length(), 0);
        if (len <= 0) {
            cerr << "Error: cannot send request to server." << endl;
        }
        // receive from server
        vector<char> buffer(MAX_RECEIVE, 0);
        int len2 = recv(client_fd, &(buffer.data()[0]), MAX_RECEIVE, 0);
        if (len2 <= 0) {
            cerr << "Error: cannot receive response from server." << endl;
        }
        string response(buffer.data(), len2);
        // cout << response << endl;
        // cout << "------------------------------------------ " << endl;

        /* ----------generate transaction-----------  */
        ostringstream outputOss2 =
            createTransaction(i, "zhumi", 10.0, 1.0);
        XMLDocument doc2;
        string xmlString2 = outputOss2.str();

        // send to server
        int len3 = send(client_fd, xmlString2.c_str(), xmlString2.length(), 0);
        if (len3 <= 0) {
            cerr << "Error: cannot send request to server." << endl;
        }

        // receive from server
        vector<char> buffer2(MAX_RECEIVE, 0);
        int len4 = recv(client_fd, &(buffer2.data()[0]), MAX_RECEIVE, 0);
        if (len4 <= 0) {
            cerr << "Error: cannot receive response from server." << endl;
        }
        string response2(buffer2.data(), len4);
        // cout << response2 << endl;
        // cout << "------------------------------------------ " << endl;
    }
    cout << "Total request number: " << test_num * 2 << endl;
    gettimeofday(&end_time, NULL);
    double timecost = end_time.tv_sec - start_time.tv_sec +
                      (end_time.tv_usec - start_time.tv_usec) / 1e6;
    cout << "Latency: " << timecost << " seconds("<< test_num * 2 <<" requests)" << endl;
    cout << "Throughput: " << test_num * 2 / timecost << " request/sec" << endl;
    closefd(client_fd);
}

void Client::testXML(
    const char *hostname) { // test our own testcase from xmlFile directory
    int client_fd;
    try {
        client_fd = startClient(hostname, SERVERPORT);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        closefd(client_fd);
        return;
    }
    // XMLDocument doc;
    // doc.LoadFile("xmlFile/trans1.xml");
    // XMLPrinter printer;
    // doc.Print(&printer);
    // string xmlString = printer.CStr();
    // cout << "send request to server :" << endl;
    // cout << xmlString << endl;
    // // send to server
    // int len = send(client_fd, xmlString.c_str(), xmlString.length(), 0);
    // if (len <= 0) {
    //     cerr << "Error: cannot send request to server." << endl;
    // }

    // // receive from server
    // vector<char> buffer(MAX_RECEIVE, 0);
    // int len2= recv(client_fd, &(buffer.data()[0]), MAX_RECEIVE, 0);
    // if (len2 <= 0) {
    //     cerr << "Error: cannot receive response from server." << endl;
    // }
    // string response(buffer.data(), len2);
    // cout << response << endl;
    // cout << "------------------------------------------ " << endl;

    string directoryPath = "xmlFile";
    for (const auto &entry : fs::directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".xml") {
            // convert xml to string
            XMLDocument doc;
            doc.LoadFile(entry.path().c_str());
            XMLPrinter printer;
            doc.Print(&printer);
            string xmlString = printer.CStr();

            size_t size = sizeof(char) * (xmlString.length() + 1);
            xmlString = to_string(size) + '\n' + xmlString;
            // cout << "send request to server :" << endl;
            // cout << xmlString << endl;
            // send to server
            int len = send(client_fd, xmlString.c_str(), xmlString.length(), 0);
            if (len <= 0) {
                cerr << "Error: cannot send request to server." << endl;
            }
            // receive from server
            vector<char> buffer(MAX_RECEIVE, 0);
            int len2 = recv(client_fd, &(buffer.data()[0]), MAX_RECEIVE, 0);
            if (len2 <= 0) {
                cerr << "Error: cannot receive response from server." << endl;
            }
            string response(buffer.data(), len2);
            cout << response << endl;
            cout << "------------------------------------------ " << endl;
        }
    }
    closefd(client_fd);
}