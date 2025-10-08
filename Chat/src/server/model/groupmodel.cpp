#include "groupmodel.hpp"
#include "database.h"

//创建群组
bool GroupModel::createGroup(Group &group){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) value('%s','%s')",
        group.GetGroupname().c_str(),group.GetGroupdesc().c_str());
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            //mysql_insert_id获取插入成功的数据生成的主键id
            group.SetId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
//查询群组是否不存在
bool GroupModel::isGroupNotExist(int groupid){
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select id from AllGroup where id=%d",groupid);
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row;
            //逐行获取数据
            if((row=mysql_fetch_row(res))==nullptr){      
                //若群组不存在
                mysql_free_result(res);//释放结果集指针
                return true;
            }
        }
        mysql_free_result(res);//释放结果集指针 
    }
    return false;//若群组存在或有其他错误
}


//加入群组
bool GroupModel::addGroup(int userid,int groupid,string role){
    if(isGroupNotExist(groupid)){
        //若群组不存在，直接返回
        return false;
    }
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into GroupUser(userid,groupid,grouprole) value(%d,%d,'%s')",
        userid,groupid,role.c_str());
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}
//查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid){
    //组装sql语句
    char sql[1024]={0};
    //子查询（多表查询）
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from AllGroup a where id in (select groupid from GroupUser b where userid=%d)",userid);
    
    vector<Group> groupVec;//存储Group信息
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row;
            //逐行获取数据
            while((row=mysql_fetch_row(res))!=nullptr){      
                //若找到
                Group group;
                group.SetId(atoi(row[0]));
                group.SetGroupname(row[1]);
                group.SetGroupdesc(row[2]);
                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);//释放结果集指针 
    }
    //分别查询每个群组的用户信息
    for(Group &group:groupVec){
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from User a JOIN GroupUser b ON b.userid=a.id WHERE b.groupid=%d;",group.GetId());
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row;
            //逐行获取数据
            while((row=mysql_fetch_row(res))!=nullptr){      
                //若找到
                GroupUser groupuser;
                groupuser.SetId(atoi(row[0]));
                groupuser.SetName(row[1]);
                groupuser.SetState(row[2]);
                groupuser.SetRole(row[3]);
                group.getUsers().push_back(groupuser);
            }
        }
        mysql_free_result(res);//释放结果集指针 
    }
    return groupVec;//若群组不存在或其他错误，vec为空
}
//根据指定的groupid查询群组用户id列表，除userid自己，主要用于用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
    //组装sql语句
    char sql[1024]={0};
    //子查询（多表查询）
    sprintf(sql,"select userid from GroupUser where groupid=%d and userid!=%d",groupid,userid);
    
    vector<int> vec;//存储群组其他成员id
    
    MySQL mysql;
    if(mysql.connect()){
        //若连接成功
        MYSQL_RES* res=mysql.query(sql);//进行sql语句查询，获得MYSQL_RES结果集
        if(res!=nullptr){
            //查找操作成功（找到或没找到）
            MYSQL_ROW row;
            //逐行获取数据
            while((row=mysql_fetch_row(res))!=nullptr){      
                //若找到
                int id=atoi(row[0]);
                vec.push_back(id);
            }
        }
        mysql_free_result(res);//释放结果集指针 
    }
    
    return vec;//若群组不存在其他成员或其他错误，vec为空
}