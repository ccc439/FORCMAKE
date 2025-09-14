cmake_minimum_required(VERSION 3.10)
project(FIRSTTRY)

add_subdirectory(add_sub)

add_executable(eee2 main.cpp)
target_include_directories(eee2 PUBLIC ./add_sub)
target_link_libraries(eee2 PUBLIC ADD SUB)