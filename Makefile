.PHONY: all  clean 

# Flags
CFLAGS =-g -Wall -fpic
LDFLAGS = -shared

CC?=gcc
CXX?=g++
LD?=gcc

# Directories
LUA_INC_DIR ?= /usr/include/lua5.1
PROTOBUF_INC_DIR ?=/usr/local/include 
PROTOBUF_LIB_DIR ?=/usr/local/lib
# Files
LIB = luapb.so

all:
	$(CXX) $(CFLAGS) -c -Iinclude -I$(LUA_INC_DIR) -I$(PROTOBUF_INC_DIR) src/*.cpp
	$(CXX) $(LDFLAGS) -o $(LIB) *.o -L$(PROTOBUF_LIB_DIR) -lprotobuf  -llua-5.1

clean:
	rm -f *.so
	rm -f *.o
	
