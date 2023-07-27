#include "client.h"
#include <boost/bind/bind.hpp>

int main(int argc, char *argv[]) {
    /* input format: ./client hostname m n 
       m = 1 for testcase; 2 for test scalability 
       n = number of request 
       eg. ./client vcm-30747.vm.duke.edu 2 1000 */
    Client myClient;
    const char *hostname = argv[1];
    int testChoice = stoi(argv[2]);
    int numReq = stoi(argv[3]);
    if (testChoice == 1) {
        myClient.testXML(hostname);
    } else if (testChoice == 2) { // test scalability
        //int test_num = 1000;
        boost::asio::thread_pool pool(1);
        boost::asio::post(pool, [&myClient, hostname, numReq]() {
            myClient.runClient(hostname, numReq);
        });
        pool.join();
    }
    return EXIT_SUCCESS;
}