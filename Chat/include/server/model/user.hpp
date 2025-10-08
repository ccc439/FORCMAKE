#pragma once

#include <string>
using namespace std;

//User表的ORM类（映射表的字段）
class User{
public:
    User(int id=-1,string name="",string password="",string state="offline")
        :id(id),name(name),password(password),state(state){}
    
    void SetId(int i){id=i;}
    void SetName(string i){name=i;}
    void SetPassword(string i){password=i;}
    void SetState(string i){state=i;}

    int GetId(){return id;}
    string GetName(){return name;}
    string GetPassword(){return password;}
    string GetState(){return state;}
private:
    int id;//用户id
    string name;//用户名
    string password;//用户密码
    string state;//当前登录状态
};