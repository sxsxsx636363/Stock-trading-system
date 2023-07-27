#ifndef REQUEST_H
#define REQUEST_H
#include "tinyxml2.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>
#include <string>
#include <vector>
using namespace tinyxml2;
using namespace std;
using namespace pqxx;
class stock_database;

class Request {
  public:
    int accountId;
    Request(int id) : accountId(id) {}
    virtual void execute(XMLDocument &response, stock_database &db) = 0;
    virtual void print() = 0;
};

class Account : public Request {
  public:
    double balance;
    Account(int id, double b) : Request(id), balance(b) {}
    void execute(XMLDocument &response, stock_database &db) override;
    void print() override;
};

class Position : public Request {
  public:
    string symbol;
    double amount;
    Position(int id, string s, double amount)
        : Request(id), symbol(s), amount(amount) {}
    void execute(XMLDocument &response, stock_database &db) override;
    void print() override;
};

class OpenOrder : public Request {
  public:
    string symbol;
    double amount;
    double limit;
    int transId;
    OpenOrder(int id, string s, double amount, double limit)
        : Request(id), symbol(s), amount(amount), limit(limit) {}
    void execute(XMLDocument &response, stock_database &db) override;
    void print() override;
};

class Order : public Request {
  public:
    int transId;
    int orderId;
    string symbol;
    double amount;
    string status;
    long int time;
    double execute_price;
    Order(int _transId, int _orderId, string _symbol, double _amount,
          string _status, long int _time, int account_id, double _execute_price)
        : transId(_transId), orderId(_orderId), symbol(_symbol),
          amount(_amount), status(_status), time(_time), Request(account_id),
          execute_price(_execute_price) {}
    Order(int id, string s, double amount, double limit)
        : Request(id), symbol(s), amount(amount), execute_price(limit) {}
    void execute(XMLDocument &response, stock_database &db);
    void print() override;
};

class Query : public Request {
  public:
    int transId;
    Query(int id, int transid) : Request(id), transId(transid) {}
    void execute(XMLDocument &response, stock_database &db) override;
    void print() override;
};

class Cancel : public Request {
  public:
    int transId;
    Cancel(int id, int transid) : Request(id), transId(transid) {}
    void execute(XMLDocument &response, stock_database &db) override;
    void print() override;
};
class stock_database {
  private:
    connection *C;
    void connectDB();
    void setupTables(string filename);
    result MatchOrder(OpenOrder &inputOrder);
    void InsertOrders(OpenOrder & order_info, double & amount, string & status, long int & time, double & price);
    void InsertOrders(int & trans_id, string & symbol, double & amount, string & status, long int & time, int & account_id, double & price);
    void InsertOpenOrders(OpenOrder & order_info);
    void InsertOpenOrders(string & symbol, double & amount, double & limit, int & account_id);
    result getOpenOrder(int & trans_Id);
    result getPositionNo_lock(int & account_id, string & symbol);
    result getOrder(int & trans_id);
    result getAccount(int & account_id);
    result getOpenStatusOrder(int & trans_Id);
    result getOpenOrderno_lock(int & trans_Id);
    result getPosition(int & account_id, string & symbol);
    double getAccountBalance(int & account_id);
    void UpdateAccountBalance(int & account_id, double & balance);
    void UpdatePositionAmount(int & account_id, string & symbol, double & amount);
    void UpdateOpenOrderAmount(int & trans_id, double & amount);
    void UpdateOpenOrderStatus(int & trans_Id, string & status);
    void InsertPositions(int & account_id, string & symbol, double & amount);
    void processOrder(int & trans_id);
   // void TransHandler();

    //for test
    void InsertAccounts(int & account_id, double & balance);
  public:
    stock_database() {}
    ~stock_database() {
        // Close database connection
        if (C->is_open()) {
            C->disconnect();
            cout << "Successfully close database" << endl;
        }
    }
    /*initialise database*/
    void initDB(string filename);
    /*print out oder*/
    string PrintOrder(int & order_id, string & symbol);
    /*Cancel open oder*/
    void CancelOpenOrder(Cancel & cancel_order,XMLDocument &response);
    /*Create open oder*/
    void CreateOpenOrder(OpenOrder & input_order);
    /*Create position*/
    void CreatePositions(Position & position_info);
    /*Create account*/
    void CreateAccounts(Account & account_info);
    /*Query*/
    void getOrderQuery(Query & order_query, XMLDocument &response);
};
#endif

// void stock_database::TransHandler(OpenOrder & inputOrder, result::const_iterator & c, double & buyer_amount, double & seller_amount, double & updated_seller_balance){
//   long int current_time = time(NULL);
//   int trans_id = c[0].as<int>();//seller
//   int account_id = c[1].as<int>();
//   string symbol = c[2].as<string>();
//   double limit = c[4].as<double>();
//   string executed_str = "executed";
//   InsertOrders(inputOrder,buyer_amount,executed_str, current_time, limit); //buyer
//   InsertOrders(trans_id,symbol,seller_amount,executed_str, current_time, account_id, limit);//seller
//   UpdateAccountBalance(trans_id, updated_seller_balance);
// }