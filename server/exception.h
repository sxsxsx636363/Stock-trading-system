#ifndef _EXCEPTION_H
#define _EXCEPTION_H
#include <exception>
#include <string>
using namespace std;

class myException:public exception {
private:
    string message;
public:
    myException(const string& msg):message(msg){}
    virtual const char* what() const throw(){
        return message.c_str();
    }
};

#endif