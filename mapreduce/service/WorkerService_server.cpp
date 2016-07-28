#include <string>

#include "WorkerService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "util.h"

using std::string;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

template <class K, class V>
class WorkerServiceHandler : virtual public WorkerServiceIf {
 public:
  WorkerServiceHandler(string me,
                       mapreduce::Map<K, V> _mapper,
                       mapreduce::Reduce
                       int _nRPC) : name(me), nRPC(_nRPC) {
    mapper = _mapper;
    reducer = _reducer;
  }

  bool DoJob(const DoJobArgs& arg, const DoJobReply& res) {
    if(nRPC == 0) return false;
    nRPC -= 1;
    printf("DoJob\n");
    if (arg.Operation == "Map") {
      mapreduce::DoMap(arg.JobNumber, arg.File, arg.NumOtherPhase, mapper);
    } else if(arg.Operation == "Reduce") {
      mapreduce::DoReduce(arg.JobNumber, arg.File, arg.NumOtherPhase, reducer);
    } else {
      res.OK = false;
      return true;
    }
    res.OK = true;
    nJobs += 1;
    return true;
  }

  bool Shutdown(const ShutdownReply& res) {
    // TODO
    printf("Shutdown\n");
    return true;
  }

  inline int GetnRPC() { return nRPC; }

  inline int GetnJobs() { return nJobs; }

 private:
  string name;
  mapreduce::Map<K, V> mapper;
  mapreduce::Reduce<V> reducer;
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

