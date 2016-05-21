// stream_client.cpp

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::local::stream_protocol;

int main(int argc, char* argv[])
{
  try {
    if (argc != 2) {
      std::cerr << "Usage: stream_client <file>\n";
      return 1;
    }

    boost::asio::io_service io_service;
    stream_protocol::socket s(io_service);
    s.connect(stream_protocol::endpoint(argv[1]));
    std::cout << "Domain socket connection established" << std::endl;

    std::cout << "Enter message: ";
    std::string request;
    std::getline(std::cin, request);
    boost::asio::write(s, boost::asio::buffer(request, request.size()));

    std::string reply;
    reply.resize(request.size());
    auto reply_length = boost::asio::read(s, boost::asio::buffer(&reply[0], request.size()));
    std::cout << reply << std::endl;

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
