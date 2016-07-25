#include "WorkerService.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class WorkerServiceHandler : virtual public WorkerServiceIf {
 public:
  WorkerServiceHandler(int _nRPC) : nRPC(_nRPC) {}

  bool DoJob(const DoJobArgs& arg, const DoJobReply& res) {
    nRPC -= 1;
    printf("DoJob\n");
    // TODO
    nJobs += 1;
    return true;
  }

  bool Shutdown(const ShutdownReply& res) {
    printf("Shutdown\n");
    return true;
  }

  int GetnRPC() { return nRPC; }
  inline int GetnJobs() { return nJobs; }

 private:
  int nRPC;
  int nJobs = 0;
};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<WorkerServiceHandler> handler(new WorkerServiceHandler());
  shared_ptr<TProcessor> processor(new WorkerServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

