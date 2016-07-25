#ifndef WORKER_H
#define WORKER_H

#include <unistd.h>
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
  inline void SetnRPC(int k) { nRPC = k; }
  inline int GetnJobs() { return nJobs; }
  inline void SetnJobs(int k) { nJobs = k; }

 private:
  string name;
  mapreduce::Map<K, V> mapper;
  mapreduce::Reduce<V> reducer;
  int nPRC;
  int nJobs = 0;
};

void RunWorker(const string & masterAddr, const string & me,
               mapreduce::Map<K, V> & mapFunc, mapreduce::Reduce<V> & reduceFunc,
               int nPRC) {
  std::cout << "RunWorker " << me << std::endl;
  Worker wk(me, mapFunc, reduceFunc, nRPC);

  // init registerClient
  int masterPort = std::stoi(mapreduce::strSplit(masterAddr, ':').back());
  shared_ptr<TTransport> socket(new TSocket("localhost", masterPort));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  MapReduceServiceClient registerClient(protocol);
  TSimpleServer *registerServerPtr;
  auto dojob_handler = [&] () {
    int port = std::stoi(mapreduce::strSplit(me, ':').back()); 
    shared_ptr<WorkerServiceHandler> handler(new WorkerServiceHandler(nRPC));
    shared_ptr<TProcessor> processor(new WorkerServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    registerServerPtr = new TSimpleServer(processor,
                                          serverTransport,
                                          transportFactory,
                                          protocolFactory);
    registerServerPtr->serve();
  };
  vector<thread> dojob_thrd;
  
  try {
    transport->open();
    registerClient.Register(me);
    
    dojob_thrd.push_back(thread(dojob_handler));

    while(wk.GetnRPC() != 0) {
      // continue accepting master rpc tasks
      wk.SetnRPC(doJobServerPtr->GetnRPC());
      wk.SetnJobs(doJobServePtr->GetnJobs());
      usleep(10000);
    }
    // TODO: close DoJobServer
    for(auto & thrd : dojob_thrd) {
      thrd.join();
    }
    transport->close();
  } catch (TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
}

} // namespace mapreduce

#endif
