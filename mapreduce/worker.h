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

void RunWorker(const string & masterAddr, const string & me,
               mapreduce::Map<K, V> & mapFunc, mapreduce::Reduce<V> & reduceFunc,
               int nPRC) {
  std::cout << "RunWorker " << me << std::endl;

  // init registerClient
  int masterPort = std::stoi(mapreduce::strSplit(masterAddr, ':').back());
  shared_ptr<TTransport> socket(new TSocket("localhost", masterPort));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  MapReduceServiceClient registerClient(protocol);

  WorkerServiceHandler *handler_ptr = new WorkerServiceHandler(me, mapFunc, reduceFunc, nRPC);
  TSimpleServer *doJobServerPtr;
  auto dojob_handler = [&] () {
    int port = std::stoi(mapreduce::strSplit(me, ':').back()); 
    shared_ptr<WorkerServiceHandler> handler(handler_ptr),
    shared_ptr<TProcessor> processor(new WorkerServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    doJobServerPtr = new TSimpleServer(processor,
                                       serverTransport,
                                       transportFactory,
                                       protocolFactory);
    doJobServerPtr->serve();
  };
  vector<thread> dojob_thrd;
  
  try {
    transport->open();
    registerClient.Register(me);
    
    // start registerServer
    dojob_thrd.push_back(thread(dojob_handler));

    // maybe unnecessary, auto shutdown in DoJob 
    while(handler_ptr->GetnRPC() != 0) {
      // wait in main thread, continue accepting master rpc tasks
      usleep(1000000);
    }
    // TODO: close DoJobServer
    for(auto & thrd : dojob_thrd) {
      thrd.join();
    }
    transport->close();
  } catch (TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
  // double free?
  delete handler_ptr;
}

} // namespace mapreduce

#endif
