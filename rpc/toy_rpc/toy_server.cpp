#include <stdio.h>
#include <string>
#include "divide.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class divideHandler : virtual public divideIf {
 public:
  void div(Response& _return, const double numerator, const double denominator) {
    if(denominator == 0.) {
      _return.error = Errors::INVALID_REQUEST;
      printf("Cannot divide by 0!\n");
      return;
    }
    _return.result = numerator / denominator;
    _return.error = Errors::SUCCESS;
    printf("div: %lf div %lf is %lf.\n", numerator, denominator, _return.result);
  }
  
  void sayhi(std::string& _return) {
    _return = "HELLO WORLD.";
    printf("sayh: .\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<divideHandler> handler(new divideHandler());
  shared_ptr<TProcessor> processor(new divideProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

