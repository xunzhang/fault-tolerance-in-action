#ifndef WORKER_H
#define WORKER_H

#include <vector>
#include <string>
#include <iostream>
#include <thread>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "WorkerService.h"
#include "util.h"

using std::string;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using boost::shared_ptr;

namespace mapreduce {

template <class K, class V>
class Worker {
 public:
  Worker(string me,
         mapreduce::Map<K, V> _mapper,
         mapreduce::Reduce<V> _reducer,
         int _nRPC) : name(me), nRPC(_nRPC) {
    mapper = _mapper;
    reducer = _reducer;
  }
  inline int GetnRPC() { return nRPC; }
  inline void DecrnRPC() { nRPC--; }
  inline int GetnJobs() { return nJobs; }
  inline void IncrnJobs() { nJobs++; }

 private:
  string name;
  mapreduce::Map<K, V> mapper;
  mapreduce::Reduce<V> reducer;
  int nPRC;
  int nJobs = 0;
};

void RunWorker(const string & masterAddr, const string & me,
               mapreduce::Map<K, V> & mapFunc, mapreduce::Reduce<V> & reduceFunc,
               int nPRC_) {
  std::cout << "RunWorker " << me << std::endl;
  Worker wk(me, mapFunc, reduceFunc, nRPC);

  // init registerClient
  int masterPort = std::stoi(mapreduce::strSplit(masterAddr, ':').back());
  shared_ptr<TTransport> socket(new TSocket("localhost", masterPort));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  MapReduceServiceClient registerClient(protocol);
  
  vector<thread> service_thrds;
  auto _lambda_StartDoJobServer = [&] () {
    int port = mapreduce::getFreePort();
    // TODO
  };

  try {
    transport->open();
    registerClient.Register(me);
    while(wk.GetnRPC() != 0) {
      // continue accepting master rpc tasks
      wk.DecrnRPC();
      _lambda_StartDoJobServer();
      wk.IncrJobs();
    }
    for(auto & thrd : service_thrds) {
      thrd.join();
    }
    transport->close();
  } catch (TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
}

} // namespace mapreduce

#endif
