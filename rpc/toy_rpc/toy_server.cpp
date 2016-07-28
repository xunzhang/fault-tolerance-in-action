#include <stdio.h>

#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "divide.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using std::mutex;
using boost::shared_ptr;

class A {
 public:
  double div(double a, double b) { ++K; return a / b; }
  int getK() { return K; }
 private:
  int K = 0;
};

class divideHandler : virtual public divideIf {
 public:
  divideHandler(A _obj) : obj(_obj) {}
  void div(Response& _return, const double numerator, const double denominator) {
    if(denominator == 0.) {
      _return.error = Errors::INVALID_REQUEST;
      printf("Cannot divide by 0!\n");
      return;
    }
    _return.result = obj.div(numerator, denominator);
    std::cout << obj.getK() << std::endl;
    //_return.result = numerator / denominator;
    _return.error = Errors::SUCCESS;
    printf("div: %lf div %lf is %lf.\n", numerator, denominator, _return.result);
  }

  void sayhi(std::string& _return) {
    _return = "HELLO WORLD.";
    printf("sayh: .\n");
  }

  void too() { std::cout << "too" << std::endl; }

 private:
  A obj;
};

class ToyServer {
 
 public:
  ToyServer() {
    auto rpc_handler = [&] () {
      int port = 9090;
      handler_ptr = new divideHandler(obj);
      //shared_ptr<divideHandler> handler(new divideHandler(obj));
      shared_ptr<divideHandler> handler(handler_ptr);
      shared_ptr<TProcessor> processor(new divideProcessor(handler));
      shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
      shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
      shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
      server_ptr = new TSimpleServer(processor, serverTransport, transportFactory, protocolFactory);
      server_ptr->serve();
    };
    thrds.push_back(std::thread(rpc_handler));
    //thrds.push_back(std::thread(&TSimpleServer::serve, server));
  }

  ~ToyServer() {
    for(auto & thrd : thrds) {
      thrd.join();
    }
    delete handler_ptr;
    delete server_ptr;
  }

  void foo() {
    // do job in main thread...
    handler_ptr->too();
    std::cout << "main thread job" << std::endl;
  }

 private:
  mutex mtx;
  std::vector<std::thread> thrds;
  divideHandler *handler_ptr;
  TSimpleServer *server_ptr;
  A obj;
}; // class ToyServer

int main(int argc, char **argv) {
  ToyServer ts;
  ts.foo();
  return 0;
}

