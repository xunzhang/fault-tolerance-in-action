# Makefile for mapreduce folder

ifndef MR_DIR
MR_DIR = .
endif

BIN_DIR = $(MR_DIR)/bin

CXX = g++
CXXFLAGS += -Wall -std=c++11

LIBS := $(LIBS) -lgflags -lboost_regex

TEST_SRC = $(MR_DIR)/wc_test.cpp

RM = rm -rf

all: prepare test_wc

prepare:
	mkdir -p $(BIN_DIR)

test_wc: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/wc_test $^ $(LIBS)

.PHONY: clean

clean:
	$(RM) $(BIN_DIR)
	$(RM) mrtmp.kjv12*
