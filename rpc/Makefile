# Makefile

ifndef RPC_DIR
RPC_DIR = .
endif

BIN_DIR = $(RPC_DIR)/bin

CXX = clang++
CFLAGS = -Wall -std=c++11 -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H
CPPFLAGS = -I$(RPC_DIR)/toy_rpc/
LIBS = -lthrift

AUTOSOURCE = $(RPC_DIR)/toy_rpc/divide_types.cpp $(RPC_DIR)/toy_rpc/divide.cpp $(RPC_DIR)/toy_rpc/divide_constants.cpp
SERVERSOURCE = $(RPC_DIR)/toy_rpc/toy_server.cpp $(AUTOSOURCE)
CLIENTSOURCE = $(RPC_DIR)/toy_rpc/toy_client.cpp $(AUTOSOURCE)

RM = rm -rf

all: prepare toy_server toy_client

# rules
prepare:
	thrift --gen cpp --out ${RPC_DIR}/toy_rpc ${RPC_DIR}/divide.thrift
	mkdir -p $(BIN_DIR)

toy_server: $(SERVERSOURCE)
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LIBS) -o $(BIN_DIR)/toy_server $^
	@echo 'Successfully Built in ./bin/toy_server'

toy_client: $(CLIENTSOURCE)
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LIBS) -o $(BIN_DIR)/toy_client $^
	@echo 'Successfully Built in ./bin/toy_client'

.PHONY: clean

clean:
	$(RM) $(BIN_DIR)
	${RM} ${RPC_DIR}/toy_rpc/divide*
