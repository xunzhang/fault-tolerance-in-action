#ifndef WORKER_H
#define WORKER_H

#include <string>
#include <iostream>

#include "util.h"

using std::string;

namespace mapreduce {

void RunWorker(const string & MasterAddr, const string & me,
               mapreduce::Map<K, V> & MapFunc, mapreduce::Reduce<V> & ReduceFunc,
               int nPRC_) {
  std::cout << "RunWorker " << me << std::endl;
  int masterPort = ;
  shared_ptr<TTransport> socket(new TSocket("localhost", masterPort));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  xxClient registerClient(protocol);
  try {
    transport->open();
    registerClient.Register(me);
  } catch (TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
  /*
     name = me;
     Map = MapFunc;
     Reduce = ReduceFunc;
     nRPC = nPRC_;
   */
}

template <class K, class V>
class WorkerRpc {
 public:
  void Shutdown() {
    std::cout << "Shutdown " << name << std::endl;
    nPRC = 1; // OK, because the same thread reads nPRC
    nJobs -= 1;
  }

 private:
  string name;
  mapreduce::Map<K, V> Map;
  mapreduce::Reduce<V> Reduce;
  int nRPC;
  int nJobs;
  // TODO: add listener data structure
};

} // namespace mapreduce

#endif
