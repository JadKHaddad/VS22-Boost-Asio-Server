#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "helper.hpp"
#include "json_struct.h"

#define MAX_SIGNALS 3

using boost::asio::ip::tcp;

typedef std::deque<message> message_queue;

int signals_received = 0;

namespace
{
  std::function<void(int)> shutdown_handler;
  void signal_handler(int signal) { shutdown_handler(signal); }
}

class client
{
public:
  client(boost::asio::io_context &io_context,
         const tcp::resolver::results_type &endpoints)
      : io_context_(io_context),
        socket_(io_context)
  {
    do_connect(endpoints);
  }

  void write(const message &msg)
  {
    boost::asio::post(io_context_,
                      [this, msg]()
                      {
                        bool write_in_progress = !write_msgs_.empty();
                        write_msgs_.push_back(msg);
                        if (!write_in_progress)
                        {
                          do_write();
                        }
                      });
  }

  void close()
  {
    boost::asio::post(io_context_, [this]()
                      { socket_.close(); });
  }

private:
  void do_connect(const tcp::resolver::results_type &endpoints)
  {
    boost::asio::async_connect(socket_, endpoints,
                               [this](boost::system::error_code ec, tcp::endpoint)
                               {
                                 if (!ec)
                                 {
                                   do_read_header();
                                 }
                               });
  }

  void do_read_header()
  {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), message::header_length),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                              if (!ec && read_msg_.decode_header())
                              {
                                do_read_body();
                              }
                              else
                              {
                                socket_.close();
                              }
                            });
  }

  void do_read_body()
  {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                              if (!ec)
                              {
                                // parse the message body
                                std::string body_string(read_msg_.body());
                                JS::ParseContext context(body_string);
                                message_body body;
                                context.parseTo(body);
                                if (body.type == message_type::position_type)
                                {
                                  pos_ = body.pos;
                                  std::cout << "My Position: " << pos_.x << ", " << pos_.y << std::endl;

                                  // send a message back to the server
                                  message_body msg_body;
                                  msg_body.type = message_type::movement_type;
                                  msg_body.pos = pos_; // not used
                                  msg_body.dir = create_a_random_direction();
                                  msg_body.score = 0; // not used
                                  std::string pretty_json = JS::serializeStruct(msg_body);
                                  const void *body = pretty_json.c_str();

                                  message msg;
                                  msg.body_length(std::strlen((char *)body));
                                  std::memcpy(msg.body(), body, msg.body_length());
                                  msg.encode_header();
                                  write(msg);
                                }
                                else if (body.type == message_type::score_type)
                                {
                                  score_ = body.score;
                                  std::cout << "My Score: " << score_ << std::endl;
                                  exit(0);
                                }

                                do_read_header();
                              }
                              else
                              {
                                socket_.close();
                              }
                            });
  }

  void do_write()
  {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(write_msgs_.front().data(),
                                                 write_msgs_.front().length()),
                             [this](boost::system::error_code ec, std::size_t /*length*/)
                             {
                               if (!ec)
                               {
                                 write_msgs_.pop_front();
                                 if (!write_msgs_.empty())
                                 {
                                   do_write();
                                 }
                               }
                               else
                               {
                                 socket_.close();
                               }
                             });
  }

private:
  boost::asio::io_context &io_context_;
  tcp::socket socket_;
  message read_msg_;
  message_queue write_msgs_;
  position pos_;
  int score_;
};

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: client <host> <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    client client(io_context, endpoints);

    std::thread t([&io_context]()
                  { io_context.run(); });

    shutdown_handler = [&](int signum)
    {
      std::cout << std::endl
                << "Caught signal.. Terminating.." << signum << std::endl;
      signals_received++;
      if (signals_received >= MAX_SIGNALS)
      {
        std::cout << "MAX_SIGNALS received.. Forcing exit.." << std::endl;
        exit(1);
      }
      // create a message to send to the server
      message_body msg_body;
      msg_body.type = message_type::score_type;
      msg_body.pos = position{0, 0}; // not used
      msg_body.dir = direction::up;  // not used
      msg_body.score = 0;
      std::string pretty_json = JS::serializeStruct(msg_body);
      const void *body = pretty_json.c_str();

      message msg;
      msg.body_length(std::strlen((char *)body));
      std::memcpy(msg.body(), body, msg.body_length());
      msg.encode_header();
      client.write(msg);
    };

    // Register signal and signal handler
    signal(SIGINT, signal_handler);
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    client.close();
    t.join();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}