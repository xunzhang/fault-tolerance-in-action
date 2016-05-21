// stream_server.cpp

#include <cstdio>
#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

using boost::asio::local::stream_protocol;

class session : public boost::enable_shared_from_this<session> {
 public:
  session(boost::asio::io_service& io_service) : socket_(io_service) {
    data_.resize(1024);
  }

  stream_protocol::socket& socket() { return socket_; }

  void start() {
    socket_.async_read_some(boost::asio::buffer(&data_[0], 1024),
                            boost::bind(&session::handle_read,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code& error,
                   size_t bytes_transferred) {
    if (!error) {
      boost::asio::async_write(socket_,
                               boost::asio::buffer(&data_[0], bytes_transferred),
                               boost::bind(&session::handle_write,
                                           shared_from_this(),
                                           boost::asio::placeholders::error));
    }
  }

  void handle_write(const boost::system::error_code& error) {
    if (!error) {
      socket_.async_read_some(boost::asio::buffer(&data_[0], 1024),
                              boost::bind(&session::handle_read,
                                          shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
    }
  }

 private:
  // The socket used to communicate with the client.
  stream_protocol::socket socket_;

  // Buffer used to store data received from the client.
  std::string data_;
}; // class session

typedef boost::shared_ptr<session> session_ptr;

class server {
 public:
  server(boost::asio::io_service& io_service, const std::string& file)
      : io_service_(io_service),
      acceptor_(io_service, stream_protocol::endpoint(file)) {
    session_ptr new_session(new session(io_service_));
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(session_ptr new_session,
                     const boost::system::error_code& error) {
    if (!error) {
      new_session->start();
    }

    new_session.reset(new session(io_service_));
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
  }

 private:
  boost::asio::io_service& io_service_;
  stream_protocol::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try {
    if (argc != 2) {
      std::cerr << "Usage: stream_server <file>\n";
      std::cerr << "*** WARNING: existing file is removed ***\n";
      return 1;
    }

    boost::asio::io_service io_service;
    std::remove(argv[1]); // remove previous binding
    server s(io_service, argv[1]);
    io_service.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
