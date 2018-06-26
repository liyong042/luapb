cmake_minimum_required(VERSION 2.8)
project(luapb)
#设置编译选项
add_definitions("-g -fPIC -shared -Wall -pipe -fno-ident -MMD -Werror -D_GNU_SOURCE -D_REENTRANT -O2")

#设定include目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(/usr/include/lua5.1)    #PROTOBUF_INC_DIR 
include_directories(/usr/local/include)     #PROTOBUF_INC_DIR

#获取目录下的c/cpp文件合集
aux_source_directory(${CMAKE_SOURCE_DIR}/src DIR_SRCS)

link_directories(/usr/local/lib)			  #liblua-5.1.so  libprotobuf.so  

#编译成动态库
add_library(luapb SHARED ${DIR_SRCS} ) 

#添加链接库
target_link_libraries(luapb lua-5.1 protobuf)
