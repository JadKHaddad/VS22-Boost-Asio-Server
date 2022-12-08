#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "header.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;

//----------------------------------------------------------------------
class participant
{
public:
  participant() : id_(0) , position_(0,0) {}
  virtual ~participant() {}
  virtual void deliver(const message& msg) = 0;
  void set_id(int id) { id_ = id; }
  int get_id() { return id_; }
  void set_position(position position) { position_ = position; }
  position get_position() { return position_; }

private:
  int id_;
  position position_;
};

typedef std::shared_ptr<participant> participant_ptr;

//----------------------------------------------------------------------

class room
{
public:
  room() {
    std::cout << "room created" << std::endl;
    //run in a separate thread
    std::thread t(&room::run, this);
    t.detach();
  }

  void join(participant_ptr participant)
  {
    //set participant id
    participant->set_id(participants_.size());
    //create a random posision for the new client
    position pos = position(rand() % 100, rand() % 100);
    participant->set_position(pos);
    //add the new client to the list of clients
    participants_.insert(participant);
    std::cout << "client " << participant->get_id() << " joined with position: " << participant->get_position().x() << ", " << participant->get_position().y() << std::endl;
    std::cout << "total clients = " << participants_.size() << std::endl;


    //create a position message for the new client
    position_message position_msg = position_message(pos);

    message_body msg_body = message_body();
    msg_body.type = position_type;
    msg_body.position_msg = position_msg;

    //convet the message body to a message
    // message msg = message();
    // msg.body_length(std::strlen((char*)&msg_body));
    // std::memcpy(msg.body(), (char*)&msg_body, msg.body_length());
    // msg.encode_header();


    //say hello to the new client
    message msg;
    msg.body_length(std::strlen("hello"));
    std::memcpy(msg.body(), "hello", msg.body_length());
    msg.encode_header();
    participant->deliver(msg);
  }

  void run()
  {
    while (true)
    {
      std::cout << "room running" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  void leave(participant_ptr participant)
  {
    participants_.erase(participant);
    std::cout << "client left" << std::endl;
    std::cout << "total clients = " << participants_.size() << std::endl;
  }

  void on_new_message(const message& msg, participant_ptr participant)
  {
    std::cout << "new message from client: " << participant->get_id() << std::endl;
  }

  void deliver(const message& msg)
  {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();

    for (auto participant: participants_)
      participant->deliver(msg);
  }

private:
  std::set<participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class session
  : public participant,
    public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket, room& room)
    : socket_(std::move(socket)),
      room_(room)
  {
  }

  void start()
  {
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
      do_write();
    }
  }

private:
  void do_read_header()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), message::header_length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            room_.on_new_message(read_msg_, shared_from_this());
            do_read_header();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_write()
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
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
            room_.leave(shared_from_this());
          }
        });
  }

  tcp::socket socket_;
  room& room_;
  message read_msg_;
  message_queue write_msgs_;
};

//----------------------------------------------------------------------

class server
{
public:
  server(boost::asio::io_context& io_context,
      const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket), room_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{

  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_context io_context;

    std::list<server> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}