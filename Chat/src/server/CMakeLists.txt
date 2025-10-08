#把.指定路径下的所有源文件名字放入变量SRC_LIST中
aux_source_directory(. SRC_LIST)
#./database
aux_source_directory(./database DB_LIST)
#./model
aux_source_directory(./model MD_LIST)
#./redis
aux_source_directory(./redis RD_LIST)

#由SRC_LIST，DB_LIST，MD_LIST，RD_LIST中的源文件，生成可执行文件ChatServer
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MD_LIST} ${RD_LIST})

#ChatServer需要链接的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)