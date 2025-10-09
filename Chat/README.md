用cmake管理，基于muduo网络库，json数据交换，nginx tcp负载均衡环境，松耦合高内聚的集群聊天服务器，客户端实现

--客户端头文件

    client.hpp              
--服务器头文件

    database.h              --数据库操作类MySQL 封装MYSQL C API
    user.hpp                --数据库User表的映射类
    group.hpp               --数据库AllGroup表的映射类
    groupuser.hpp           --继承User的群组用户，只多一个数据库GroupUser表的grouprole
    usermodel.hpp           --User表的数据库操作类
    friendmodel.hpp         --Friend表的数据库操作类
    groupmodel.hpp          --AllGroup和GroupUser表的数据库操作类
    offlinemsgmodel.hpp     --offline_messages表（离线消息表）的数据库操作类
    redis.hpp               --redis操作 封装redis C API
    chatserver.hpp          --网络模块（muduo库）
    chatservice.hpp         --业务模块
    public.hpp              --关于msgid的enum类型

网络模块和业务模块的松耦合：

    auto msgHandler=ChatService::getInstance().getHandler(js["msgid"].get<int>());
    msgHandler(conn,js,tim);
    在网络模块专门处理用户读写事件的回调函数 onMessage()中，ChatService::getInstance()返回一个对象实例，将js中"msgid"键对应值传入getHandler，运行返回的对应事件处理函数msgHandler

--一键cmake构建脚本

    autobuild.sh
