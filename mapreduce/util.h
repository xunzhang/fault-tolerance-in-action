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
using Map = std::function<std::vector<std::vector<std::pair<K, V>>>(std::string)>;

template <class K, class V>
using Reduce = std::function<std::string(std::string, std::vector<std::vector<std::pair<K, V>>>)>;

} // namespace mapreduce

#endif
