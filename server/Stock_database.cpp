#include <iostream>
#include <pqxx/pqxx>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip> 
#include <fstream>      // std::ifstream
#include "request.h"
#include "exception.h"
using namespace std;
using namespace pqxx;
/*
    connect to database
*/
void stock_database::connectDB(){
    try{
        //Establish a connection to the database
        //Parameters: database name, user name, user password
        C = new connection("dbname=stockdb user=postgres password=passw0rd host=db port=5432");
        //C = new connection("dbname=stockdb user=postgres password=passw0rd");
        if (C->is_open()) {
            cout << "Opened database successfully: " << C->dbname() << endl;
        }else{
            cout << "Can't open database" << endl;
        }
    } catch (const std::exception &e){
        cerr << e.what() << std::endl;
    }
    return ;
}
/*
 Create table for account, positions, openorder, orders by import sql file
*/
void stock_database::setupTables(string filename){
    work W(*C);
    ifstream file(filename.c_str());
    string sqlstr,line;
    if(file.is_open()){
    while(getline(file,line)){
        sqlstr += line;
    }
    file.close();
    }
    else{
        cerr<< "cannot open file"<<endl;
    }
    try{       
        W.exec(sqlstr);
        W.commit();
        cout <<"create table successfully" <<endl;
    }catch(const exception &e){
        cerr<<e.what()<<endl;
        W.abort();
    } 
}
/*
    initialise database, including both connection and setup table
*/
void stock_database::initDB(string filename){
    connectDB();
    setupTables(filename);
}

/*
    Insert account into database with giving account_id and balance
*/
void stock_database::InsertAccounts(int & account_id, double & balance){
    
    /* Execute SQL query */   
    result R = getAccount(account_id);
    //cout<< "InsertAccounts()"<<endl;
    if(R.size()!=0){
        throw myException(" Error : account is already exist and can't be recreated");
        return;
    }else{
        string accounts = "INSERT INTO ACCOUNTS (ACCOUNT_ID,BALANCE) VALUES (" + to_string(account_id) + ", "  + to_string(balance) + ");";
    // cout << accounts <<endl;
        work W(*C);
        W.exec(accounts);
        W.commit();
}
        
}

void stock_database::CreateAccounts(Account & account_info){
    /* Execute SQL query */   
    result R = getAccount(account_info.accountId);
    if(R.size()!=0){
        //TODO: if the account is already exist, should we recreate it?
        throw myException(" Error: account is already exist and can't be recreated");
        return;
    }
    if(account_info.balance < 0){
        throw myException(" Error: balance cannot be negative.");
        return;
    }
    string accounts = "INSERT INTO ACCOUNTS (ACCOUNT_ID,BALANCE) VALUES (" + to_string(account_info.accountId) + ", "  + to_string(account_info.balance) + ");";
    // cout << accounts <<endl;
    work W(*C);  
    W.exec(accounts);
    W.commit();
}


/*Add symbol/position into account with corresponding account_id, symbol, amount: : If symbol is already exist in acount, update the amount, otherwise, insert a new position*/
void stock_database::InsertPositions(int & account_id, string & symbol, double & amount){
    //  result result_positions = getPosition(account_id, symbol);
    // //cout << player <<endl;
    // //Check if the symbol exist in this account or not 
    // if(result_positions.size() == 0){//Position didn't exist before
    //     cout << "Note InsertPositions(): the symbol didn't exist before"<<endl;
        work W(*C); 
        string positions = "INSERT INTO POSITIONS (ACCOUNT_ID,SYM,AMOUNT) VALUES (" + to_string(account_id) + ", " + W.quote(symbol) + ", " +  to_string(amount) + ") ON CONFLICT (ACCOUNT_ID,SYM) DO UPDATE SET AMOUNT = " + to_string(amount)+ "+ POSITIONS.AMOUNT;";
        //cout << positions <<endl;
        W.exec(positions);
        W.commit();
    // }else{
    //     int position_id = result_positions.begin()[0].as<int>();
    //     double update_amount = result_positions.begin()[0].as<int>()+ amount;
    //     UpdatePositionAmount(position_id, update_amount);
    // }
}

/*Insert open order into database*/
void stock_database::InsertOpenOrders(OpenOrder & order_info){
        work W(*C);     
        /* Execute SQL query */   

        string orders = "INSERT INTO OPENORDER (SYM,AMOUNT,LIMITS,ACCOUNT_ID,STATUS) VALUES (" + W.quote(order_info.symbol) + ", " +  to_string(order_info.amount) + ", " +  to_string(order_info.limit) + ", " + to_string(order_info.accountId) + ", 'open') RETURNING TRANS_ID";
        // cout << orders <<endl;
        try{
            result R(W.exec(orders));
            W.commit();          
            order_info.transId = R.begin()[0].as<int>();
            // cout << orders <<endl;
        }catch(const exception &e){
            cerr<<e.what()<<endl;
            W.abort();
        }
        
}
/*Insert open order into database with corresponding symbol, amount, limit, account_id*/
void stock_database::InsertOpenOrders(string & symbol, double & amount, double & limit, int & account_id){
    work W(*C);     
    /* Execute SQL query */   
    string orders = "INSERT INTO OPENORDER (SYM,AMOUNT,LIMITS,ACCOUNT_ID,STATUS) VALUES (" + W.quote(symbol) + ", " +  to_string(amount) + ", " +  to_string(limit) +  ", " + to_string(account_id) + ", 'open');";
    // cout << orders <<endl;
    try{
        W.exec(orders);
        W.commit();
    }catch(const exception &e){
        cerr<<e.what()<<endl;
        W.abort();
    }
}
/*Insert executed/canceled order into database with corresponding OpenOrder information with additional, amount, status, time, price*/
void stock_database::InsertOrders(OpenOrder & order_info, double & amount, string & status, long int & time, double & price){
    work W(*C);     
    /* Execute SQL query */   
    string orders = "INSERT INTO ORDERS (TRANS_ID,SYM,AMOUNT,STATUS,TIME,ACCOUNT_ID,PRICE) VALUES ("+ to_string(order_info.transId) +", "+ W.quote(order_info.symbol) + ", " +  to_string(amount) + ", " + W.quote(status) + ", " + to_string(time) + ", " + to_string(order_info.accountId) + ", " + to_string(price) + ");";
    // cout << orders <<endl;
    try{
        W.exec(orders);
        W.commit();
    }catch(const exception &e){
        cerr<<e.what()<<endl;
        W.abort();
    }
}

/*Insert executed/canceled order into database with with all attribute values*/
void stock_database::InsertOrders(int & trans_id, string & symbol, double & amount, string & status, long int & time, int & account_id, double & price){
    work W(*C);     
    /* Execute SQL query */   
    string orders = "INSERT INTO ORDERS (TRANS_ID,SYM,AMOUNT,STATUS,TIME,ACCOUNT_ID,PRICE) VALUES (" +  to_string(trans_id) + "," + W.quote(symbol) + ", " +  to_string(amount) + ", " + W.quote(status) + ", " + to_string(time)+ ", " + to_string(account_id) + ", " + to_string(price)+ ");";
    // cout << orders <<endl;
    try{
        W.exec(orders);
        W.commit();
    }catch(const exception &e){
        cerr<<e.what()<<endl;
        W.abort();
    }
}
/*Get order sets information according the trans_id*/
result stock_database::getOrder(int & trans_id){
    nontransaction N(*C);
    string order_info = "SELECT * FROM ORDERS WHERE TRANS_ID = " + to_string(trans_id)+ ";";
    result R(N.exec(order_info));
    return R;
}

/*Return corresponding response for giving order query information*/
void stock_database::getOrderQuery(Query & order_query, XMLDocument &response){
    nontransaction N(*C);
    string open_info = "SELECT * FROM OPENORDER WHERE TRANS_ID = " + to_string(order_query.transId)+ ";";
    result R(N.exec(open_info));
    string order_info = "SELECT * FROM ORDERS WHERE TRANS_ID = " + to_string(order_query.transId)+ ";";
    result order_set(N.exec(order_info));
    // result order_set = getOrder(order_query.transId);
    if (R.size()==0){
        throw myException("Error: Query transition id does not exist.");
        return;
    }
    XMLElement* status = response.NewElement("status");
    status->SetAttribute("id", to_string(order_query.transId).c_str());
    //check if it still has open part or not
    if(R.begin()[5].as<string>()=="open"){
        XMLElement* open = response.NewElement("open");
        open->SetAttribute("shares", R.begin()[3].as<string>().c_str());
        status->InsertEndChild(open);
    }

    //check if it still has execeted part or not
    for(result::const_iterator order_it = order_set.begin(); order_it!= order_set.end();order_it++){
        if(order_it[4].as<string>()=="executed"){
            XMLElement* executed = response.NewElement("executed");
            executed->SetAttribute("shares", order_it[3].as<string>().c_str());
            executed->SetAttribute("price", order_it[7].as<string>().c_str());
            executed->SetAttribute("time", order_it[5].as<string>().c_str());
            status->InsertEndChild(executed);
        }else if(order_it[4].as<string>()=="canceled"){
            XMLElement* canceled = response.NewElement("canceled");
            canceled->SetAttribute("shares", order_it[3].as<string>().c_str());
            canceled->SetAttribute("time", order_it[5].as<string>().c_str());
            status->InsertEndChild(canceled);
        }
    }
    XMLElement* root = response.RootElement();
    root->InsertEndChild(status);
}

/*Return result of OpenOrder of the corresponding trans_Id*/
result stock_database::getOpenOrder(int & trans_Id){
    string info =  "SELECT TRANS_ID, ACCOUNT_ID, SYM, AMOUNT, LIMITS FROM OPENORDER WHERE STATUS = 'open' AND TRANS_ID = " + to_string(trans_Id)+ " FOR UPDATE;";
    nontransaction N(*C);
    result R(N.exec(info));
    return R;
}

/*Return result of OpenOrder of the corresponding trans_Id*/
result stock_database::getOpenOrderno_lock(int & trans_Id){
    string info =  "SELECT TRANS_ID, ACCOUNT_ID, SYM, AMOUNT, LIMITS FROM OPENORDER WHERE STATUS = 'open' AND TRANS_ID = " + to_string(trans_Id)+ " ;";
    nontransaction N(*C);
    result R(N.exec(info));
    return R;
}
/*Print selected order*/
string stock_database::PrintOrder(int & order_id, string & symbol){
    nontransaction W(*C);
    string info =  "SELECT * FROM ORDERS WHERE ORDER_ID = " + to_string(order_id)+ "AND SYM = " +W.quote(symbol) + ";";
    cout << info <<endl;
    result R(W.exec(info));

    if(R.size()==0){
        cout <<"No order find" <<endl;
        return "";
    }
    //cout <<"trans_id: " << R.begin()[0].as<int>() <<" order_id: " <<R.begin()[1].as<int>()<<" sym: "<<R.begin()[2].as<string>()<<" amount: "<< R.begin()[3].as<double>()<<" status: "<< R.begin()[4].as<string>()<<" time: "<< R.begin()[5].as<long int>()<<" account_id: "<< R.begin()[6].as<int>()<<" price: "<< R.begin()[7].as<double>()<<endl;
    string order =  "trans_id id: " + R.begin()[0].as<string>() +" order_id: " +R.begin()[1].as<string>()+" sym: "+R.begin()[2].as<string>()+" amount: "+ R.begin()[3].as<string>() + " status: " + R.begin()[4].as<string>()+" time: " + R.begin()[5].as<string>();
    return order;
}

/*Get account balance*/
double stock_database::getAccountBalance(int & account_id){
    nontransaction W(*C);
    string info =  "SELECT BALANCE FROM ACCOUNTS WHERE ACCOUNT_ID = " + to_string(account_id)+ " FOR UPDATE;";
    result R(W.exec(info));
    //cout <<"get Order balance from account_id: " <<account_id << " balance: " <<R.begin()[0].as<double>() <<endl;
    return R.begin()[0].as<double>();
}


/*Change account balance*/
void stock_database::UpdateAccountBalance(int & account_id, double & balance){
    double origin_balance= getAccountBalance(account_id);
    double new_balance = origin_balance + balance;
    work W(*C);
    string info = "UPDATE ACCOUNTS SET BALANCE = " +to_string(new_balance)+ " WHERE ACCOUNT_ID = " + to_string(account_id) +";";
    try{
        //cout << info << endl;
        /* Execute SQL query */
        W.exec( info );
        W.commit();
        //cout << "Balance update successfully" << endl;
   } catch (const std::exception &e) {
      cerr << e.what() << std::endl;
   }
}

/*Get status of the selected open order inorder to */
result stock_database::getOpenStatusOrder(int & trans_Id){
    nontransaction W(*C);
    string info =  "SELECT STATUS FROM OPENORDER WHERE TRANS_ID = " + to_string(trans_Id)+ " AND STATUS = 'open' FOR UPDATE;";
    result R(W.exec(info));
    return R;
}

/*Change status of open order*/
void stock_database::UpdateOpenOrderStatus(int & trans_Id, string & status){
    result result_status = getOpenStatusOrder(trans_Id);
    if(result_status.size()==0){
        throw myException("Error: The open order correspoding to the trans_id didn't found.");
        return;
    }
    work W(*C);
    string info = "UPDATE OPENORDER SET STATUS = " + W.quote(status)+ " WHERE TRANS_ID = " + to_string(trans_Id) +";";
    try{
        //cout << info << endl; 
        /* Execute SQL query */
        W.exec( info );
        W.commit();
        //cout << "Records update successfully" << endl;
   } catch (const std::exception &e) {
      cerr << e.what() << std::endl;
   }
}

result stock_database::getPosition(int & account_id, string & symbol){
    nontransaction W(*C);
    string info =  "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " + to_string(account_id)+ " AND SYM = " + W.quote(symbol) + " FOR UPDATE;";
    
    result R(W.exec(info));
    //cout << "account: " << R.begin()[0].as<string>()<<" amount: " << R.begin()[2].as<string>()<<endl;
    return R;
}
/*Change position amount*/
void stock_database::UpdatePositionAmount(int & account_id, string & symbol, double & amount){
   
    result amount_result = getPosition(account_id, symbol);
    if(amount_result.size()==0){
        throw myException("Error: The position correspoding to the account_id&symbol didn't found.");
        return;
    }
    double origin_amount = amount_result.begin()[2].as<double>();
    double updated_amount = origin_amount+amount;
    if(updated_amount<0){
        throw myException("Error: The position amount can't update due to negative value.");
        return;
    }
    
    try{
        /* Execute SQL query */
        work W(*C);
        string info = "UPDATE POSITIONS SET AMOUNT = " + to_string(updated_amount)+ " WHERE ACCOUNT_ID = " + to_string(account_id)+ " AND SYM = " + W.quote(symbol) +";";
        //cout << info <<endl;
        W.exec( info );
        W.commit();
        // cout << "Position Amount update successfully" << endl;
   } catch (const std::exception &e) {
      cerr << e.what() << std::endl;
   }
}

/*Change amount for open order*/
void stock_database::UpdateOpenOrderAmount(int & trans_id, double & amount){
    result amount_result = getOpenOrder(trans_id);
    if(amount_result.size()==0){
        throw myException("Error: The openorder correspoding to the trans_id didn't found.");
        return;
    }
    work W(*C);
    string info = "UPDATE OPENORDER SET AMOUNT = " +to_string(amount) + " WHERE TRANS_ID = " + to_string(trans_id) +";";
    try{
        //cout << info << endl;
        /* Execute SQL query */
        W.exec( info );
        W.commit();
        //cout << "Records update successfully" << endl;
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
    }
}



result stock_database::getAccount(int & account_id){
    nontransaction W(*C);
    string info =  "SELECT * FROM ACCOUNTS WHERE ACCOUNT_ID = " + to_string(account_id)+ " ;";
    result R(W.exec(info));
    //cout <<"get account from account_id: " <<account_id << endl;
    return R;
}

/*Return select position results  according to account_id and symbol*/
result stock_database::getPositionNo_lock(int & account_id, string & symbol){
    nontransaction N(*C);
    string oldpositions = "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " + to_string(account_id) + " AND SYM = " + N.quote(symbol) + " ;";
    result R;
    try{
        R = N.exec(oldpositions);
    }catch(const exception &e){
        cerr<<e.what()<<endl;
        N.abort();
    }
    return R;
}

/*Return the vector of openorder.trans_id: 1.open 2.same symbol 3.limit prices are compatible*/
result stock_database::MatchOrder(OpenOrder & inputOrder){
    int trans_id = inputOrder.transId;
    int account = inputOrder.accountId;
    string symbol = inputOrder.symbol;
    double amount = inputOrder.amount;
    double limit = inputOrder.limit;
    OpenOrder temporder(account,symbol,amount,limit);
    temporder.transId = trans_id;
    inputOrder = temporder;
    nontransaction W(*C);
    string matchedinfo;

    //if it is from buyer:
    if(amount > 0){
        //cout << "buyer order : " << trans_id << "symbol: " <<symbol << " amount: "<<amount << " limit: "<< limit << endl;
        matchedinfo = "SELECT * FROM OPENORDER WHERE OPENORDER.SYM = "+ W.quote(symbol) + " AND OPENORDER.STATUS = 'open' AND OPENORDER.AMOUNT<0 AND OPENORDER.LIMITS <= " + to_string(limit) + " ORDER BY LIMITS ASC, TRANS_ID ASC FOR UPDATE;" ;
    }else{  //else it is from seller:
        //cout << "seller order : " << trans_id <<"symbol: " <<symbol << " amount: "<<amount << " limit: "<< to_string(limit) << endl;
        matchedinfo = "SELECT * FROM OPENORDER WHERE OPENORDER.SYM = "+ W.quote(symbol) + " AND OPENORDER.STATUS = 'open' AND OPENORDER.AMOUNT>0 AND OPENORDER.LIMITS >= " + to_string(limit) + " ORDER BY LIMITS DESC, TRANS_ID ASC FOR UPDATE;" ;
    }

    result R(W.exec(matchedinfo));
    // if(R.size()==0){//No seller order matches
    //     cout << "Didn't match order! " <<endl;  
    // }else{
    //     for(result::const_iterator c = R.begin(); c != R.end(); ++c){
    //         cout << "Matched order! " <<endl;
    //         cout << "order : " << " trans_id: "<<c[0].as<string>() << " account_id: "<< c[1].as<string>() << " symbol: "<< c[2].as<string>() << " amount: "<< c[3].as<string>()<< " limit: "<< c[4].as<string>()<< endl;       
    //     }
    // }
    return R;
}
/*Process created open order by matching with other open order*/
void stock_database::processOrder(int & trans_id){
    result resultOpenOrder = getOpenOrderno_lock(trans_id);
    if(resultOpenOrder.size()== 0){
        //cout << "processOrder(): didn't find corresponding open order" << endl;
        return;
    }
    else{
        OpenOrder inputOrder(resultOpenOrder.begin()[1].as<int>(),resultOpenOrder.begin()[2].as<string>(),resultOpenOrder.begin()[3].as<double>(),resultOpenOrder.begin()[4].as<double>());
        inputOrder.transId = resultOpenOrder.begin()[0].as<int>();
        result matchedOrder = MatchOrder(inputOrder);
        if(matchedOrder.size()==0){//No matched order, nothing to be changed
            //cout << "Didn't match order! " <<endl; 
            return; 
        }else{  
            //cout<<"matched"<<endl;
            string executed_str = "executed";
            if(inputOrder.amount>0){
                //cout<<"I'm buyer"<<endl;
                double buyer_amount = inputOrder.amount;
                result::const_iterator c = matchedOrder.begin();
                int trans_id = c[0].as<int>();//seller
                int account_id = c[1].as<int>();
                string symbol = c[2].as<string>();
                double amount = c[3].as<double>();
                double limit = c[4].as<double>();
                
                if((amount+buyer_amount)==0){
                    //Add executed order for both buyer and seller
                    //Change open order status for both buyer and seller
                    //cout<<"equal amount" <<endl;
                    long int current_time = time(NULL);
                    double add_seller_balance = limit * buyer_amount;
                    double insert_sell_amount = 0-buyer_amount;
                    //TransHandler(inputOrder, buyer_amount,insert_sell_amount);
                    InsertOrders(inputOrder,buyer_amount,executed_str, current_time, limit); //buyer       
                    InsertOrders(trans_id,symbol,insert_sell_amount,executed_str, current_time, account_id, limit);//seller
                    UpdateOpenOrderStatus(inputOrder.transId, executed_str);//buyer
                    UpdateOpenOrderStatus(trans_id, executed_str);//seller
                    UpdateAccountBalance(account_id, add_seller_balance);//add balance in seller
                    if(limit<inputOrder.limit){//refund balance in buyer
                        //cout<<"start refund"<<endl;
                        double refunded_buyer_balance = (inputOrder.limit-limit) * buyer_amount;
                        UpdateAccountBalance(inputOrder.accountId,refunded_buyer_balance);
                    }
                    
                    InsertPositions(inputOrder.accountId,inputOrder.symbol,buyer_amount);

                }else if((amount+buyer_amount)<0){//sell amount > buyer amount
                    //cout<<"amount >  buyer_amount: " <<amount+buyer_amount << endl;
                    double insert_sell_amount = 0-buyer_amount;
                    long int current_time = time(NULL);
                    double add_seller_balance = limit * buyer_amount;
                    double updated_seller_amount = amount+buyer_amount;
                    //TransHandler(inputOrder, buyer_amount,insert_sell_amount);
                    InsertOrders(inputOrder,buyer_amount,executed_str, current_time, limit); //buyer 
                    InsertOrders(trans_id,symbol,insert_sell_amount,executed_str, current_time, account_id, limit);//seller
                    UpdateOpenOrderAmount(trans_id, updated_seller_amount);//seller
                    UpdateOpenOrderStatus(inputOrder.transId, executed_str);//buyer
                    UpdateAccountBalance(account_id, add_seller_balance);//seller
                    if(limit<inputOrder.limit){//refund balance in buyer
                        //cout<<"start refund"<<endl;
                        double refunded_buyer_balance =(inputOrder.limit-limit) * buyer_amount;
                        UpdateAccountBalance(inputOrder.accountId, refunded_buyer_balance);
                    }
                    InsertPositions(inputOrder.accountId,inputOrder.symbol,buyer_amount);
                    
                }else{//sell amount < buyer amount
                    //cout<<"not equal amount" <<endl;
                     for(result::const_iterator a = matchedOrder.begin(); a != matchedOrder.end(); ++a){
                        trans_id = a[0].as<int>();
                        account_id = a[1].as<int>();
                        symbol = a[2].as<string>();
                        amount = a[3].as<double>();
                        limit = a[4].as<double>();            
                        if((amount+buyer_amount)==0){//amount is enough for thies transection
                            //Add buyer order
                            long int current_time = time(NULL);
                            double insert_sell_amount = 0-buyer_amount;
                            double  add_seller_balance = limit * buyer_amount;
                            //cout<<"sell amount == buyer amount" << amount+buyer_amount <<endl;        
                            //TransHandler(inputOrder, c, buyer_amount,insert_sell_amount,updated_seller_balance);
                            InsertOrders(inputOrder,buyer_amount,executed_str, current_time, limit);//buyer                           
                            InsertOrders(trans_id,symbol,insert_sell_amount,executed_str, current_time, account_id, limit);//seller
                            UpdateAccountBalance(account_id, add_seller_balance);//seller
                            UpdateOpenOrderStatus(trans_id, executed_str);
                            UpdateOpenOrderStatus(inputOrder.transId, executed_str);//buyer
                            
                            if(limit<inputOrder.limit){//refund balance in buyer
                                //cout<<"start refund"<<endl;
                                double refunded_buyer_balance = (inputOrder.limit-limit) * buyer_amount;
                                UpdateAccountBalance(inputOrder.accountId, refunded_buyer_balance);
                            }
                            InsertPositions(inputOrder.accountId,inputOrder.symbol,buyer_amount);
                            break;
                        }else if((amount+buyer_amount)<0){//amount is enough for thies transection
                            //Add buyer order
                            //cout<<"sell amount > buyer amount" << amount+buyer_amount <<endl;
                            long int current_time = time(NULL);
                            double insert_sell_amount = 0-buyer_amount;
                            double updated_buy_amount = amount+buyer_amount;
                            double add_seller_balance = limit * buyer_amount;
                            //TransHandler(inputOrder, c, buyer_amount,insert_sell_amount,updated_seller_balance);
                            InsertOrders(inputOrder,buyer_amount,executed_str, current_time, limit);//buyer            
                            InsertOrders(trans_id,symbol,insert_sell_amount,executed_str, current_time, account_id, limit);//seller
                            UpdateAccountBalance(account_id, add_seller_balance);
                            UpdateOpenOrderAmount(trans_id, updated_buy_amount);
                            UpdateOpenOrderStatus(inputOrder.transId, executed_str);
                            //cout << "buyer_amount: "<< buyer_amount<<"limit: "<<limit << endl;
                            
                            if(limit<inputOrder.limit){//refund balance in buyer
                                //cout<<"start refund"<<endl;
                                double refunded_buyer_balance = (inputOrder.limit-limit) * buyer_amount;
                                UpdateAccountBalance(inputOrder.accountId, refunded_buyer_balance);
                            }
                            InsertPositions(inputOrder.accountId,inputOrder.symbol,buyer_amount);
                            break;
                        }else{
                            //cout<<"sell amount < buyer amount" << endl;
                            long int current_time = time(NULL);
                            double insert_buy_amount = 0-amount;
                            double updated_buy_amount = amount+buyer_amount;
                            double add_seller_balance =0-limit * amount;
                            //TransHandler(inputOrder, c, buyer_amount,insert_sell_amount,updated_seller_balance);
                            InsertOrders(inputOrder,insert_buy_amount,executed_str, current_time, limit);//buyer
                            InsertOrders(trans_id,symbol,amount,executed_str, current_time, account_id, limit);//seller
                            UpdateAccountBalance(account_id, add_seller_balance);
                            UpdateOpenOrderAmount(inputOrder.transId, updated_buy_amount);//buyer
                            UpdateOpenOrderStatus(trans_id, executed_str);                                                
                            InsertPositions(inputOrder.accountId,inputOrder.symbol,insert_buy_amount);
                            if(limit<inputOrder.limit){//refund balance in buyer
                                //cout<<"start refund"<<endl;
                                double refunded_buyer_balance = 0-(inputOrder.limit-limit) * amount;
                                UpdateAccountBalance(inputOrder.accountId, refunded_buyer_balance);
                            }                 
                            buyer_amount+=amount;    
                            if(buyer_amount<=0){
                                break;
                            }  
                        }
                     }
                }
            }else{
                //cout<<"I'm seller"<<endl;
                //seller
                double seller_amount = inputOrder.amount;
                result::const_iterator c = matchedOrder.begin();
                //buyer
                int trans_id = c[0].as<int>();
                int account_id = c[1].as<int>();
                string symbol = c[2].as<string>();
                double amount = c[3].as<double>();
                double limit = c[4].as<double>();
                if((seller_amount+amount)==0){
                    //cout<<"equal amount" <<endl;
                    long int current_time = time(NULL);
                    double add_seller_balance = limit * amount;
                    InsertOrders(inputOrder,seller_amount,executed_str, current_time, limit);//serller
                    InsertOrders(trans_id,symbol,amount,executed_str, current_time, account_id,limit);//buyer
                    UpdateOpenOrderStatus(inputOrder.transId, executed_str);//serller
                    UpdateOpenOrderStatus(trans_id, executed_str);//buyer
                    UpdateAccountBalance(inputOrder.accountId, add_seller_balance);//serller balance
                    InsertPositions(account_id, symbol, amount);
                }else if((amount+seller_amount)>0){
                    //cout<<"amount < buyer_amount: " <<amount+seller_amount << endl;
                    double insert_buy_amount = 0-seller_amount;
                    long int current_time = time(NULL);
                    double updated_buyer_amount = amount+seller_amount;
                    double add_seller_balance = 0 -limit * seller_amount;
                    InsertOrders(inputOrder, seller_amount, executed_str, current_time,limit);//serller              
                    InsertOrders(trans_id,symbol,insert_buy_amount,executed_str, current_time, account_id,limit);//buyer
                    UpdateOpenOrderAmount(trans_id, updated_buyer_amount);//buyer
                    UpdateOpenOrderStatus(inputOrder.transId, executed_str);//seller
                    UpdateAccountBalance(inputOrder.accountId, add_seller_balance);//seller balance
                    InsertPositions(account_id,symbol,insert_buy_amount);
    
                }else{//sell amount > buyer amount
                    //cout<<"not equal amount" <<endl;
                    for(result::const_iterator a = matchedOrder.begin(); a != matchedOrder.end(); ++a){                  
                        
                        trans_id = a[0].as<int>();//buyer
                        account_id = a[1].as<int>();
                        symbol = a[2].as<string>();
                        amount = a[3].as<double>();
                        limit = a[4].as<double>();         
                        if((amount+seller_amount)==0){//amount is enough for thies transection
                            //cout<<"sell amount < buyer amount" << amount+seller_amount <<endl;
                            long int current_time = time(NULL);
                            double insert_buy_amount = 0-seller_amount;                           
                            double add_seller_balance = 0-limit * seller_amount;                           
                            InsertOrders(inputOrder,seller_amount,executed_str, current_time,limit);//seller
                            InsertOrders(trans_id,symbol,insert_buy_amount,executed_str, current_time, account_id,limit);//buyer
                            UpdateOpenOrderStatus(trans_id, executed_str);
                            UpdateOpenOrderStatus(inputOrder.transId, executed_str);
                            UpdateAccountBalance(inputOrder.accountId, add_seller_balance);
                            InsertPositions(account_id,symbol,insert_buy_amount);
                            break;
                        }else if((amount+seller_amount)>0){//amount is enough for thies transection
                            //cout<<"sell amount < buyer amount" << amount+seller_amount <<endl;
                            double insert_buy_amount = 0-seller_amount;
                            long int current_time = time(NULL);
                            double updated_buyer_amount = amount+seller_amount;
                            double add_seller_balance = 0-limit * seller_amount;    
                            InsertOrders(inputOrder,seller_amount,executed_str, current_time,limit);//seller
                            InsertOrders(trans_id,symbol,insert_buy_amount,executed_str, current_time, account_id,limit);//buyer
                            UpdateOpenOrderAmount(trans_id, updated_buyer_amount);//buyer
                            UpdateOpenOrderStatus(inputOrder.transId, executed_str);//seller
                            UpdateAccountBalance(inputOrder.accountId, add_seller_balance);//seller balance
                            InsertPositions(account_id,symbol,insert_buy_amount);
                            break;
                        }else{
                            //cout<<"sell amount > buyer amount" << endl;
                            double insert_sell_amount = 0-amount;
                            long int current_time = time(NULL);
                            double updated_buyer_amount = amount+seller_amount;
                            double add_seller_balance = limit * amount;
                            InsertOrders(inputOrder,insert_sell_amount,executed_str, current_time, limit);//seller
                            InsertOrders(trans_id,symbol,amount,executed_str, current_time, account_id,limit);//buyer
                            UpdateOpenOrderAmount(inputOrder.transId, updated_buyer_amount);//seller
                            UpdateOpenOrderStatus(trans_id, executed_str); //buyer                   
                            UpdateAccountBalance(inputOrder.accountId, add_seller_balance);//seller balance
                            InsertPositions(account_id, symbol, amount);
                            seller_amount+=amount;
                            if(seller_amount>=0){
                                break;
                            } 
                        }               
                    }
                }
            }
        }
    }    
}
/*Process canceled order include refund and return share*/
void stock_database::CancelOpenOrder(Cancel & cancel_order, XMLDocument &response){
    result resultOpenOrder = getOpenOrderno_lock(cancel_order.transId);
    if(resultOpenOrder.size()== 0){
        throw myException("Error: Can't cancel the order since transition id does not exist or has been executed.");
        return;
    }else{
        XMLElement* canceled = response.NewElement("canceled");
        canceled->SetAttribute("id", to_string(cancel_order.transId).c_str());
        string canceled_str = "canceled";
        OpenOrder inputOrder(resultOpenOrder.begin()[1].as<int>(),resultOpenOrder.begin()[2].as<string>(),resultOpenOrder.begin()[3].as<double>(),resultOpenOrder.begin()[4].as<double>());
        inputOrder.transId = resultOpenOrder.begin()[0].as<int>();
        //check if it is from the same account 
        if(inputOrder.accountId != cancel_order.accountId){
            throw myException("Error: transition id does not belong to this account.");
            return;
        }else{
            //Check if it is a buyer or seller
            if(inputOrder.amount > 0){
                //cout<<"I'm buyer"<<endl;
                //refund balance
                double add_buyer_balance = inputOrder.limit * inputOrder.amount;
                UpdateAccountBalance(inputOrder.accountId,add_buyer_balance);
            }else{
                //cout<<"I'm seller"<<endl;
                //return share
                double updated_sell_amount = 0-inputOrder.amount;
                InsertPositions(inputOrder.accountId, inputOrder.symbol, updated_sell_amount);
            }
            //Insert canceled order
            long int current_time  = time(NULL);
            InsertOrders(inputOrder,inputOrder.amount,canceled_str,current_time ,inputOrder.limit);       
            //Change open order status
            UpdateOpenOrderStatus(inputOrder.transId, canceled_str);
            // form xml response
            result order_set = getOrder(cancel_order.transId);
            for(result::const_iterator order_it = order_set.begin(); order_it!= order_set.end();order_it++){
                if(order_it[4].as<string>()=="executed"){
                    XMLElement* executed = response.NewElement("executed");
                    executed->SetAttribute("shares", order_it[3].as<string>().c_str());
                    executed->SetAttribute("price", order_it[7].as<string>().c_str());
                    executed->SetAttribute("time", order_it[5].as<string>().c_str());
                    canceled->InsertEndChild(executed);
                }else if(order_it[4].as<string>()=="canceled"){
                    XMLElement* cancel = response.NewElement("canceled");
                    cancel->SetAttribute("shares", order_it[3].as<string>().c_str());
                    cancel->SetAttribute("time", order_it[5].as<string>().c_str());
                    canceled->InsertEndChild(cancel);
                }
            }
        }
        XMLElement* root = response.RootElement();
        root->InsertEndChild(canceled);
    }    
}
/*Insert position into database: If symbol is already exist in acount, update the amount, otherwise, insert a new position*/
void stock_database::CreatePositions(Position & position_info){
    /* Execute SQL query */   
    //string oldpositions = "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " + to_string(position_info.accountId) + " AND SYM = " + W.quote(position_info.symbol) + "AND AMOUNT >= 0 FOR UPDATE;";
    result result_positions = getPositionNo_lock(position_info.accountId, position_info.symbol);
    result account = getAccount(position_info.accountId);
    //check if the symbol exist in this account or not
    if(account.size() == 0){  
        throw myException("Error: Can't create a position since account id does not exist.");
        return; 
    }
    if(position_info.amount < 0){  
        throw myException("Error: Symbol's amount cannot be negative.");
        return; 
    }
    if(result_positions.size() == 0){//Position didn't exist before
        work W(*C);     
        string positions = "INSERT INTO POSITIONS (ACCOUNT_ID,SYM,AMOUNT) VALUES (" + to_string(position_info.accountId) + ", " + W.quote(position_info.symbol) + ", " +  to_string(position_info.amount) + ");";
        W.exec(positions);
        W.commit();
    }else{
        string symbol = result_positions.begin()[1].as<string>();
        UpdatePositionAmount(position_info.accountId, symbol, position_info.amount);
    }
}
/*Create and process open order*/
void stock_database::CreateOpenOrder(OpenOrder & input_order){
    result account = getAccount(input_order.accountId);
    //Check if account exist or not
    if(account.size()==0){
        //TODO: return error: account is not exist
        throw myException(" Error : account does not exist.");
        return;
    }else{
        if(input_order.amount>0){
            //cout<<"I'm buyer"<<endl;
            //Check if balance is enough

            if(account.begin()[1].as<double>() < input_order.limit * input_order.amount){
                //TODO: return error: balance is not enough to buy the share
                throw myException(" Error : balance is not enough to buy the share.");
                return;
            }else{
                //Deduct from balance
                double add_buyer_balance =  0 - input_order.limit * input_order.amount;               
                UpdateAccountBalance(input_order.accountId, add_buyer_balance);//buyer               
                //insert open order
                InsertOpenOrders(input_order);
                processOrder(input_order.transId);
                
            }
        }else{
            //cout<<"I'm seller"<<endl;
            result amountResult = getPositionNo_lock(input_order.accountId, input_order.symbol);            
            if(amountResult.size()==0){
                //TODO: return error: share is not exist
                throw myException(" Error : Can't create open order since position does not exist.");
                return;
            }else{
                int account_id = amountResult.begin()[0].as<int>();
                string symbol = amountResult.begin()[1].as<string>();               
                double amount = amountResult.begin()[2].as<double>();
                //check if the share amount is enough or not
                if((amount+input_order.amount)<0){
                    //TODO: return error: share is not enough
                    //cout<<" error CreateOpenOrder(): share is not enough" <<endl;
                    throw myException(" Error : Can't create open order since do not have enough share.");
                    return;
                }else{
                    //Deduct share position
                    //cout<<"start Deduct share for seller"<<endl;
                    UpdatePositionAmount(account_id, symbol, input_order.amount);//seller position 
                    //cout <<"insert open order"<<endl;       
                    //insert open order
                    InsertOpenOrders(input_order);
                    processOrder(input_order.transId);
                }
            }
        } 
    }
}


// int main (int argc, char *argv[]) 
// {
//   //Allocate & initialize a Postgres connection object & create table
//     stock_database db;
//     db.initDB("Table_setup.sql");
//     cout <<"create state successfully" <<endl;
//     Account xin(1,100.5);
//     Account yan(2,1050.5);
//     Account sasa(1,200.5);//invalid
//     Account simple(4,-250.5);//invalid
//     db.CreateAccounts(xin);
//     db.CreateAccounts(yan);
//     //db.CreateAccounts(sasa);//Error: account is already exist and can't be recreated
//     // db.CreateAccounts(simple);// Error: balance cannot be negative.
//     Position xinpos1(1,"yes", 20);
//     Position yanpos1(2,"yes", 40);
//     Position sasapos1(3,"no", 50);//invalid
//     Position xinpos2(1,"yes", 30);
//     db.CreatePositions(xinpos1);
//     db.CreatePositions(yanpos1);
//     //db.CreatePositions(sasapos1);//Error: Account id does not exist.
//     db.CreatePositions(xinpos2);
    
//     OpenOrder errorOrder1(3, "yes", 10.0, 10.0);
//     // db.CreateOpenOrder(errorOrder1);//Error : account does not exist.
//     XMLDocument xml;
//     Query  Query1(3, 1);
//     // db.getOrderQuery(Query1, xml);//Error: transition id does not exist.
//     Query  Query2(2, 1);
//     // db.getOrderQuery(Query2, xml);//Error: Query transition id does not exist.
    
//     Cancel cancel1(1, 1);//Error: Query transition id does not exist.
//     //db.CancelOpenOrder(cancel1, xml);//Error: Can't cancel the order since transition id does not exist or has been executed.
//     // xml.Print();
//     OpenOrder Order1(2, "yes", 10.0, 10.0);
//     db.CreateOpenOrder(Order1);//trans_id:1 account_id: 2 balance: 1050.5
//     Query  Query3(2, 1);
//     db.getOrderQuery(Query3, xml);//BALANCE = 950.500000 WHERE ACCOUNT_ID = 2;
//     Cancel cancel2(2, 1);
//     db.CancelOpenOrder(cancel2, xml);//cancel open order trans_id:1 account_id: 2
//     //BALANCE = 1050.500000 WHERE ACCOUNT_ID = 2;
//     OpenOrder Order2(1, "yes", -40.0, 10.0);
//     db.CreateOpenOrder(Order2);//trans_id:2 account_id: 1 balance: 1050.5
//     Query  Query4(1, 2);
//     db.getOrderQuery(Query4, xml);//BALANCE = 100.500000 WHERE ACCOUNT_ID = 1;
    
//     OpenOrder Order3(2, "yes", 30.0, 20.0);
//     db.CreateOpenOrder(Order3);//trans_id:3 account_id: 1 balance: 1050.5
//     Query  Query5(2, 3);
//     db.getOrderQuery(Query5, xml);//BALANCE = 400.500000 WHERE ACCOUNT_ID = 1;
//     Query  Query6(1, 2);
//     db.getOrderQuery(Query6, xml);//BALANCE = 750.500000 WHERE ACCOUNT_ID = 2;
   
//     OpenOrder Order4(1, "yes", -15.0, 10.0);//Error : do not have enough share.
//     //db.CreateOpenOrder(Order4);//trans_id:4 account_id: 1 balance: 400.500000
//     Query  Query7(1, 4);
//     // db.getOrderQuery(Query7, xml);//Error: Query transition id does not exist.
    

// //trans 2 account 1 amount:-10 limit:10 open 
// //trans 4 account 1 amount:50 limit:8 open
// //trans 3 account 2 amount:30 limit:20 executed:10
// //trans 2 account 1  amount:-30 limit:10 executed:10
// //account 1 yes:10  balance:0.5 
// //account 2 yes:70 limit:2 balance:750.5
//     OpenOrder Order5(1, "yes", 50.0, 8.0);
//     db.CreateOpenOrder(Order5);//trans_id:4 account_id: 1 balance: 0.5
//     Query  Query8(1, 4);
//     db.getOrderQuery(Query8, xml);

//     OpenOrder Order6(1, "yes", 1.0, 10);
//     // db.CreateOpenOrder(Order6);//Error : balance is not enough to buy the share.
//     Query  Query9(1, 5);
//     // db.getOrderQuery(Query9, xml);//Error: Query transition id does not exist.

// // buy stock

// // trans 2 account 1 amount:-10 limit:10 open
// // trans 4 account 1 amount:50 limit:8 open
// // trans 5 account 1 amount:-10 limit:10 open
// // trans 3 account 2 amount:30 limit:20 executed:10
// // trans 2 account 1  amount:-30 limit:10 executed:10
// // account 1 yes:0  balance:0.5
// // account 2 yes:70 limit:2 balance:750.5
//     OpenOrder Order7(1, "yes", -10, 10);
//     db.CreateOpenOrder(Order7);//Error : balance is not enough to buy the share.
//     Query  Query10(1, 5);
//     // db.getOrderQuery(Query10, xml);//Error: Query transition id does not exist.

//     Account diao(3,2000.5);
//     db.CreateAccounts(diao);

//  /* 
// <!-- trans 6 account 3 amount:80 limit:20 open-->
// <!-- trans 4 account 1 amount:50 limit:8 open-->
// <!-- trans 3 account 2 amount:30 limit:20 executed:10
// <!-- trans 2 account 1  amount:-30 limit:10 executed:10
// <!-- trans 2 account 1 amount:-10 limit:10 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：10-->
// <!-- trans 5 account 1 amount:-10 limit:10 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：10-->
// <!-- account 1 yes:0  balance:200.5 -->
// <!-- account 2 yes:70 balance:750.5-->
// <!-- account 3 yes:20 balance:200.5-->
// <!-- buy 100 yes limit 10 -->
// */
//     OpenOrder Order8(3, "yes", 100, 20);
//     db.CreateOpenOrder(Order8);
//     Query  Query11(1, 6);//trans 6 amount:10 limit:20 executed：10 + trans 6 amount:10 limit:20 executed：10-->
//     db.getOrderQuery(Query11, xml);


//     OpenOrder Order9(1, "yes", -10, 20);
//     //db.CreateOpenOrder(Order9);//Error : Can't create open order since do not have enough share.

//     OpenOrder Order10(1, "no", -10, 20);
//     //db.CreateOpenOrder(Order10);//Error : Can't create open order since do not have enough share.


//     Position xinpos3(1,"yes", 150);
//     db.CreatePositions(xinpos3);
//  /* 
// <!-- trans 6 account 3 amount:70 limit:20 open-->
// <!-- trans 4 account 1 amount:50 limit:8 open-->
// <!-- trans 2 account 1 amount:-30 limit:20 executed:10-->
// <!-- trans 3 account 2 amount:30 limit:20 executed:10-->
// <!-- trans 2 account 1 amount:-10 limit:10 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：10-->
// <!-- trans 5 account 1 amount:-10 limit:10 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：20-->
// <!-- trans 7 account 1 amount:-10 limit:1 executed：20-->
// <!-- account 1 yes:140  balance:400.5 -->
// <!-- account 2 yes:70 limit:2 balance:750.5-->
// <!-- account 3 yes:30 limit:2 balance:200.5-->
// <!-- sell 10 yes limit 1 -->
// */
//     OpenOrder Order11(1, "yes", -10, 1);
//     db.CreateOpenOrder(Order11);
//     Query  Query12(1, 7);//trans 7 amount:-10 limit:1 executed：20
//     db.getOrderQuery(Query12, xml);
//  /* 
// <!-- trans 8 account 1 amount:-20 limit:1 open-->
// <!-- trans 2 account 1 amount:-30 limit:20 executed:10-->
// <!-- trans 3 account 2 amount:30 limit:20 executed:10-->
// <!-- trans 2 account 1 amount:-10 limit:10 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：10-->
// <!-- trans 5 account 1 amount:-10 limit:10 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：10-->
// <!-- trans 6 account 3 amount:10 limit:20 executed：20-->
// <!-- trans 7 account 1 amount:-10 limit:1 executed：20-->
// <!-- trans 6 account 3 amount:70 limit:20 executed：20-->
// <!-- trans 8 account 1 amount:-70 limit:1 executed：20-->
// <!-- trans 4 account 1 amount:50 limit:8 executed：8-->
// <!-- trans 8 account 1 amount:-50 limit:1 executed：8-->
// <!-- account 1 yes:50  balance:2200.5 -->
// <!-- account 2 yes:70 limit:2 balance:750.5-->
// <!-- account 3 yes:100 limit:2 balance:200.5-->
// <!-- sell 140 yes limit 1 -->
// */
//     OpenOrder Order12(1, "yes", -140, 1);
//     db.CreateOpenOrder(Order12);
//     Query  Query13(1, 8);//trans 8 amount:-70 limit:1 executed：20+ trans 8 amount:-50 limit:1 executed：8
//     db.getOrderQuery(Query13, xml);
    



//   return 0;
// }
