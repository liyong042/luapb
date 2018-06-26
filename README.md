# luapb
luapb is a google protocol buffers library for C++ without code generation.  like cjson. 
and lua-pbc is a very good library for lua, address: https://github.com/cloudwu/pbc

luapb  参考 cjson 的接口设计， 简单容易上手。
这里的 第一个版本使用的是 lua5.1.4 + protobuf2.5.1 版本 。  
对proto3的支持将在后续版本加入。 


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
  	   LOG_WARNING=1,    // Warns about issues that, although not technically a problem now, could cause problems in the future.
           LOG_ERROR  =2,    // An error occurred which should never happen during normal use.
  	   LOG_FATAL  =3     // An error occurred from which the library cannot recover.  
	}
	
