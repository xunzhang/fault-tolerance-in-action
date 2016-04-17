#include <iostream>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "divide.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int main(int argc, char *argv[])
{
  boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  
  divideClient client(protocol);

  try {
    transport->open();

    Response res;
    client.div(res, 10., 3.1415926);
    if(res.error == Errors::SUCCESS) {
      std::cout << res.result << std::endl;
    } else if (res.error == Errors::INVALID_REQUEST){
      std::cout << "Cannot divide by 0!" << std::endl;
    } else {
      std::cout << "Unknown error!" << std::endl;
    }
    std::string what;
    client.sayhi(what);
    std::cout << what << std::endl;
    
    transport->close();
  } catch (TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
}
