#ifndef UTIL_H
#define UTIL_H

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <cstdlib>
#include <ctime>
#include <cstdio>
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

#include <gflags/gflags.h>

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

template <typename... T>
std::string stringFormat(const std::string &fmt, T... vs) {
  char b;
  unsigned required = std::snprintf(&b, 0, fmt.c_str(), vs...) + 1;
  char bytes[required];
  std::snprintf(bytes, required, fmt.c_str(), vs...);
  return std::string(bytes);
}

int getFreePort(int a, int b) {
  auto isFreePort = [] (int port) {
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      return false;
    }
    close(sockfd);
    return true;
  };
  std::srand(std::time(0));
  while(1) {
    int port = (std::rand() % (b - a)) + a + 1;
    if(isFreePort(port)) {
      return port;
    }
  }
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
