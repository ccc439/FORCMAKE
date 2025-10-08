#include "offlinemsgmodel.hpp"
#include "database.h"

//存储用户离线消息（userid：用户id，msg：消息）
bool OfflineMsgModel::insert(int userid,string msg){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into offline_messages value(%d,'%s')",userid,msg.c_str());
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}

//删除用户离线消息
bool OfflineMsgModel::remove(int userid){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"delete from offline_messages where userid=%d",userid);
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}

//查询用户离线消息
vector<string> OfflineMsgModel::query(int userid){
    //组装sql语句
    char sql[1024]={0};
    //只要message字段即可
    sprintf(sql,"select message from offline_messages where userid=%d",userid);
    
    vector<string> vec;//存储离线消息
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row;
            //逐行获取数据
            while((row=mysql_fetch_row(res))!=nullptr){      
                //若userid对应离线消息存在
                vec.push_back(row[0]);
            }
        }
        mysql_free_result(res);//释放结果集指针 
    }
    
    return vec;//若userid对应离线消息不存在或其他错误，vec为空
}