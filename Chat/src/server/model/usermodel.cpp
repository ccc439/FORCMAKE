#include "usermodel.hpp"
#include "database.h"
#include <iostream>
using namespace std;

bool UserModel::insert(User &user){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into User(name,password,state) value('%s','%s','%s')",
        user.GetName().c_str(),user.GetPassword().c_str(),user.GetState().c_str());
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            //mysql_insert_id获取插入成功的用户数据生成的主键id
            user.SetId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select * from User where id=%d",id);
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row=mysql_fetch_row(res);//mysql_fetch_row()此时通过主键查找，若找到一定只有一行，若id对应用户不存在，返回null
            if(row!=nullptr){      
                //若id对应用户存在
                User user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetPassword(row[2]);
                user.SetState(row[3]);
                mysql_free_result(res);//释放结果集指针
                return user;
            }
            
        }
        mysql_free_result(res);//释放结果集指针 
    }
    //若id对应用户不存在或其他错误
    return User();//返回一个默认对象（id=-1,name="",password="",state="offline"）
}
bool UserModel::updateState(User &user){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"update User set state='%s' where id=%d ",user.GetState().c_str(),user.GetId());
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}

//重置所有online1用户的状态为offline
bool UserModel::resetState(){
    //组装sql语句
    char sql[1024]="update User set state='offline' where state='online' ";

    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}