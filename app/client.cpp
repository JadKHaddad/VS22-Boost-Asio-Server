#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "helper.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;

//----------------------------------------------------------------------

int signals_received = 0;

//----------------------------------------------------------------------

namespace
{
  std::function<void(int)> shutdown_handler;
  void signal_handler(int signal) { shutdown_handler(signal); }
}

//----------------------------------------------------------------------

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
                                message_body msg_body = decode_message_body(read_msg_.body());

                                if (msg_body.type == message_type::position_type)
                                {
                                  pos_ = msg_body.pos;
                                  std::cout << "My Position: " << pos_.x << ", " << pos_.y << std::endl;

                                  // send a message back to the server
                                  message_body msg_body = create_a_movement_message_body(create_a_random_direction());
                                  message msg = create_a_message_from_message_body(msg_body);
                                  write(msg);
                                }
                                else if (msg_body.type == message_type::score_type)
                                {
                                  score_ = msg_body.score;
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

//----------------------------------------------------------------------

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
      UNUSED(signum);

      if (signals_received < 1)
      {
        std::cout << std::endl
                  << "Caught signal.. Terminating.." << std::endl;
        std::cout << "Waiting for server to respond.." << std::endl;
        std::cout << "Press Ctrl+C again to force exit.." << std::endl;
      }

      signals_received++;

      if (signals_received >= 2)
      {
        std::cout << std::endl
                  << "Forcing exit.." << std::endl;
        exit(1);
      }

      // create a message to send to the server
      message_body msg_body = create_a_score_message_body(0);
      message msg = create_a_message_from_message_body(msg_body);
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