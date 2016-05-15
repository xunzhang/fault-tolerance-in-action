# Makefile for rpc folder

ifndef RPC_DIR
RPC_DIR = .
endif

BIN_DIR = $(RPC_DIR)/bin

CXX = g++

CXXFLAGS += -Wall -std=c++11 -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H
CPPFLAGS += -I$(RPC_DIR)/toy_rpc/ -I/usr/local/include

LDFLAGS += -L/usr/local/lib
LIBS += -lpthread -lthrift

AUTOSOURCE = $(wildcard ${RPC_DIR}/toy_rpc/*.cpp)
TOY_DIR = $(RPC_DIR)/toy_rpc
SERVERSOURCE = $(TOY_DIR)/toy_server.cpp
CLIENTSOURCE = $(TOY_DIR)/toy_client.cpp

OBJS = divide_types.o divide_constants.o divide.o

RM = rm -rf

all: prepare ${OBJS} toy_server toy_client
	@echo 'Successfully Built toy_rpc...'

prepare:
	thrift --gen cpp --out ${TOY_DIR} ${RPC_DIR}/divide.thrift
	mkdir -p $(BIN_DIR)

divide_types.o: ${TOY_DIR}/divide_types.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TOY_DIR)/divide_types.cpp 

divide_constants.o: ${TOY_DIR}/divide_constants.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TOY_DIR)/divide_constants.cpp

divide.o: ${TOY_DIR}/divide.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TOY_DIR)/divide.cpp

toy_server: $(SERVERSOURCE)
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) ${OBJS} -o $(BIN_DIR)/toy_server $^ $(LIBS)

toy_client: $(CLIENTSOURCE)
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) ${OBJS} -o $(BIN_DIR)/toy_client $^ $(LIBS)

.PHONY: clean

clean:
	$(RM) $(BIN_DIR)
	$(RM) $(TOY_DIR)/divide*
	$(RM) $(RPC_DIR)/*.o
