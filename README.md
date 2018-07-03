# luapb
luapb is a google protocol buffers library for C++ without code generation.  like cjson. 
and lua-pbc is a very good library for lua, address: https://github.com/cloudwu/pbc

email: liyong042@163.com 

luapb 参考 cjson 的接口设计， 简单容易上手。200多行代码，实现也简洁。 

第一个版本(luapb_1 )使用的是 lua5.1.4 + protobuf2.5.1 版本 。  
对proto3的支持正在开发中(  lua5.1.4 + protobuf3.5.1 ) 
	
云风大神 写了一个luapbc不错，可惜不更新了，大家可以参考一下( https://github.com/cloudwu/pbc )

开源的LuaPb 有很多，这里主要针对环境是 OpenResty( ngnix+lua ). 设计目的就是为了简单易用。 


# Message API 简单Api 
	 
	pb.encode( "Person", lua_table )         
	pb.decode( "Person", pb_buf    )
	pb.register_file(  "test.proto" )
	pb.register_path(  "./pb_path/" )
  
# Debug Log 调试日志输出 
  经常会遇到 protobuf 异常和解析失败的问题，需要打印错误日志。这里增加一个简单日志等级接口，方便查看错误。 
  
	pb.set_log（ 3 ）
  
  日志等级设计
  
        enum LogLevel {
		LOGL_INFO  =0,    // Informational.  This is never actually used by libprotobuf.
		LOG_NORMAL =1,    // normal log 
		LOG_WARNING=2,    // Warns about issues that, although not technically a problem now.
		LOG_ERROR  =3,    // An error occurred which should never happen during normal use.
		LOG_FATAL  =4     // An error occurred from which the library cannot recover.  
	}
	
# 编译 
  ## linux 编译 
   Makefile 编译， 注意设置好 LUA_INC_DIR ， PROTOBUF_INC_DIR 库路径 ,  直接 make 就可以 ，
   如果提示找不到库， 可以更改 PROTOBUF_LIB_DIR  来查找库路径 
  
   Cmake的编译很简单， 直接 cmake , make ,  如果提示库找不到，请注意设置好 lua ,protobuf的库路径。 
  ##  windows 编译  
    windows的编译很简单, 直接使用 visual studio 将代码加入编译成库就好 。  
  
  
