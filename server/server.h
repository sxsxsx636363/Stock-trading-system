#ifndef SERVER_H
#define SERVER_H
#include "exception.h"
#include "vector"
#include "request.h"
#include "socket.h"
#include <mutex>
#include <thread>
#include <boost/thread.hpp>
#define MAX_RECEIVE 65535

class Server {
public:
    stock_database db;
    void setupServe();
    void handleRequest(void * info);
    void processXML(string& rawRequest, vector<Request*>& requests);
    string receiveReq(int socket_fd);
    void sendRes(int socket_fd, string response);
};

class Args {
 public:
  int socket_fd;
  string xml;
  Args(string s, int n): xml(s), socket_fd(n){};
};

#endif
