#把.指定路径下的所有源文件名字放入变量SRC_LIST中
aux_source_directory(. SRC_LIST)

#由SRC_LIST，DB_LIST，MD_LIST中的源文件，生成可执行文件ChatClient
add_executable(ChatClient ${SRC_LIST})

#ChatClient需要链接的库文件
target_link_libraries(ChatClient pthread)