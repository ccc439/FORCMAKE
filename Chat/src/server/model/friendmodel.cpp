#include "friendmodel.hpp"
#include "database.h"
//添加好友关系
bool FriendModel::insert(int userid,int friendid){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into Friend value(%d,%d)",userid,friendid);
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}
//返回用户好友列表
vector<User> FriendModel::query(int userid){
    //组装sql语句
    char sql[1024]={0};
    //子查询（多表查询）
    sprintf(sql,"select id,name,state from User where id in (select friendid from Friend where userid=%d)",userid);
    
    vector<User> vec;//存储Friend信息
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row;
            //逐行获取数据
            while((row=mysql_fetch_row(res))!=nullptr){      
                //若有好友
                User user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);
                vec.push_back(user);
            }
        }
        mysql_free_result(res);//释放结果集指针 
    }
    
    return vec;//若好友不存在或其他错误，vec为空
}