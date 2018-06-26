

#ifndef _LUA_LOG_H
#define _LUA_LOG_H

#include <iostream>
#include <string.h>

#define LOG_FATAL    4 
#define LOG_ERROR    3 
#define LOG_WARNING  2 
#define LOG_NORMAL   1
#define LOG_INFO     0  

class  LogFile
{
public:
	int m_log_level;
	LogFile() :m_log_level(LOG_NORMAL){}
public:
	static LogFile& instance(){ static LogFile g_log; return g_log; }
};
 
#define DF_LOG(lvl, ...)\
do {                                                          		\
	if (  lvl >=  LogFile::instance().m_log_level ) {				\
		const char* basefile = strrchr( __FILE__, '/' );			\
		basefile = ( basefile )? ( basefile+1 ): __FILE__ ;			\
		std::cout << " "<< lvl <<" " << basefile <<" [" << __FUNCTION__<<" " <<__LINE__ << " ] "<< __VA_ARGS__ << std::endl;\
	}                                                              \
} while (0)	


#endif

