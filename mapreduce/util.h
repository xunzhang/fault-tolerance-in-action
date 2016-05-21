#ifndef UTIL_H
#define UTIL_H

#include <sys/stat.h>

#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include <utility>
#include <unordered_map>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <google/gflags.h>

namespace mapreduce {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using std::mutex;
using boost::shared_ptr;

template <class K, class V>
using Map = std::function<std::vector<std::pair<K, V>>(const std::string &)>;

template <class V>
using Reduce = std::function<V(const std::vector<V> &)>;

std::vector<std::string> strSplit(const std::string & str,  const char sep) {
  std::vector<std::string> result;
  size_t st = 0, en = 0;
  while(1) {
    en = str.find(sep, st);
    auto s = str.substr(st, en - st);
    if(s.size()) result.push_back(std::move(s));
    if(en == std::string::npos) break;
    st = en + 1;
  }
  return result;
}

using JobType = std::string;

struct DoJobArgs {
  std::string File;
  JobType Operation;
  int JobNumber; // this job's number
  int NumOtherPhase; // total number of jobs in other phase (map or reduce)
};

struct DoJobReply {
};

void callWorkerShutdown() {}

void callWorkerDoJob() {}

void callMapReduceRegister() {}

void callMapReduceShutdown() {}

} // namespace mapreduce

#endif
