#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "../server/socket.h"
#include "../server/thread_pool.hpp"
#include "../server/tinyxml2.h"
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <time.h>
#define SERVERPORT "12345"
#define MAX_RECEIVE 65535
namespace fs = boost::filesystem;
using namespace tinyxml2;
using namespace std;

class Client {
  public:
    void runClient(const char *hostname, int test_num);
    void testXML(const char *hostname);
    ostringstream createAccount(int account_id, double balance, string symbol_name, double amount);
    ostringstream createTransaction(int account_id, string symbol_name, double amount, double limit);
};

#endif