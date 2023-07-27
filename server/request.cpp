#include "request.h"
using namespace pqxx;

void Account::execute(XMLDocument& response, stock_database& db){
    //cout <<"Account execute start" <<endl;
    XMLElement* root = response.RootElement();
    try{       
        db.CreateAccounts(*this);
    }catch(const exception& e){
        XMLElement* error = response.NewElement("error");
        error->SetAttribute("id", accountId);
        XMLText* text = response.NewText(e.what());
        error->InsertEndChild(text);
        root->InsertEndChild(error);
        return;
    }
    XMLElement* created = response.NewElement("created");
    created->SetAttribute("id", accountId);
    root->InsertEndChild(created);
};
void Account::print(){
    cout <<"Account id: " << accountId << endl;
    cout <<"Account balance: " << balance << endl;
}

void Position::execute(XMLDocument& response, stock_database& db){
    XMLElement* root = response.RootElement();
    try{
        db.CreatePositions(*this);
    }catch(const exception& e){
        XMLElement* error = response.NewElement("error");
        error->SetAttribute("sym", symbol.c_str());
        error->SetAttribute("id", accountId);
        XMLText* text = response.NewText(e.what());
        error->InsertEndChild(text);
        root->InsertEndChild(error);
        return;
    }
    XMLElement* created = response.NewElement("created");
    created->SetAttribute("sym", symbol.c_str());
    created->SetAttribute("id", accountId);
    root->InsertEndChild(created);
}
void Position::print(){
    cout <<"Position accountid: " << accountId << endl;
    cout <<"Position symbol: " << symbol << endl;
    cout <<"Position amount: " << amount << endl;
}
void OpenOrder::execute(XMLDocument &response, stock_database &db){
    XMLElement* root = response.RootElement();
    try{       
       db.CreateOpenOrder(*this);
    }catch(const exception& e){
        XMLElement* error = response.NewElement("error");
        error->SetAttribute("sym", symbol.c_str());
        error->SetAttribute("amount", amount);
        error->SetAttribute("limit", limit);
        XMLText* text = response.NewText(e.what());
        error->InsertEndChild(text);
        root->InsertEndChild(error);
        return;
    }
    XMLElement* opened = response.NewElement("opened");
    opened->SetAttribute("sym", symbol.c_str());
    opened->SetAttribute("amount", amount);
    opened->SetAttribute("limit", limit);
    opened->SetAttribute("id", transId);
    root->InsertEndChild(opened);
}
void OpenOrder::print(){
    cout <<"OpenOrder symbol: " << symbol << endl;
    cout <<"OpenOrder amount: " << amount << endl;
    cout <<"OpenOrder limit: " << limit << endl;
}
void Query::execute(XMLDocument &response, stock_database &db){
    XMLElement* root = response.RootElement();
    //cout <<"Query execute start" << endl;
    try{
        db.getOrderQuery(*this, response);
    }catch(const exception& e){
        XMLElement* error = response.NewElement("error");
        error->SetAttribute("id", transId);
        XMLText* text = response.NewText(e.what());
        error->InsertEndChild(text);
        root->InsertEndChild(error);
        return;
    }
}
void Query::print(){
    cout <<"Query accountid: " << accountId << endl;
    cout <<"Query transid: " << transId << endl;
}
void Cancel::execute(XMLDocument &response, stock_database &db){
    XMLElement* root = response.RootElement();
    try{
        db.CancelOpenOrder(*this, response);
    }catch(const exception& e){
        XMLElement* error = response.NewElement("error");
        error->SetAttribute("id", transId);
        XMLText* text = response.NewText(e.what());
        error->InsertEndChild(text);
        root->InsertEndChild(error);
        return;
    }
}
void Cancel::print(){
    cout <<"Cancel accountid: " << accountId << endl;
    cout <<"Cancel transid: " << transId << endl;
}