#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "MapReduceService.h"

#include "chan.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using cpp::channel;

class MapReduceServiceHandler : virtual public MapReduceServiceIf {
 public:
  MapReduceServiceHandler(channel chan) {
    registerChan = chan;
  }

  bool Register(const std::string& workerAddr) {
    std::cout << "Register: worker " << workerAddr << std::endl;
    registerChan.send(workerAddr);
    return true;
  }

 private:
  channel registerChan;
}; // class MapReduceServiceHandler
