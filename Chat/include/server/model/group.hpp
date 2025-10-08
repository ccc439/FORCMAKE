#pragma once

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

//AllGroup表的ORM类（映射表的字段）
class Group{
public:
    Group(int i=-1,string name="",string desc="")
        :id(i),groupname(name),groupdesc(desc){}
    
    void SetId(int i){id=i;}
    void SetGroupname(string i){groupname=i;}
    void SetGroupdesc(string i){groupdesc=i;}

    int GetId(){return id;}
    string GetGroupname(){return groupname;}
    string GetGroupdesc(){return groupdesc;}
    vector<GroupUser>& getUsers(){return users;}
private:
    int id;//群组id
    string groupname;//群组名
    string groupdesc;//群组功能描述
    vector<GroupUser> users;//群组用户信息
};