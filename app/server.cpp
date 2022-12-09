#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "helper.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;

//----------------------------------------------------------------------
class client
{
public:
  client() : id_(0), position_(position{0, 0}) {}
  virtual ~client() {}
  virtual void deliver(const message &msg) = 0;
  void set_id(int id) { id_ = id; }
  int get_id() { return id_; }
  void set_position(position position) { position_ = position; }
  position get_position() { return position_; }
  void set_score(int score) { score_ = score; }
  int get_score() { return score_; }

private:
  int id_;
  position position_;
  int score_;
};

typedef std::shared_ptr<client> client_ptr;

//----------------------------------------------------------------------

class room
{
public:
  room()
  {
    std::cout << "Room created" << std::endl;
    // init field
    field_ = std::vector<std::vector<std::set<client_ptr>>>(WIDTH, std::vector<std::set<client_ptr>>(HEIGHT, std::set<client_ptr>()));
  }

  void display_field()
  {
    for (size_t i = 0; i < WIDTH; i++)
    {
      for (size_t j = 0; j < HEIGHT; j++)
      {
        if (field_[i][j].size() == 0)
        {
          std::cout << "X ";
        }
        else
        {
          // get the client id
          std::cout << field_[i][j].begin()->get()->get_id() << " ";
        }
      }
      std::cout << std::endl;
    }
  }

  void join(client_ptr client)
  {
    if (clients_.size() >= MAX_CLIENTS)
    {
      std::cout << "Room is full" << std::endl;
      return;
    }
    // set client id
    client->set_id(clients_.size());
    // create a random posision for the new client
    position pos = create_a_random_position();
    client->set_position(pos);
    // add client to the field
    field_[pos.x][pos.y].insert(client);
    // add the new client to the list of clients
    clients_.insert(client);
    std::cout << "Client " << client->get_id() << " joined with position: " << client->get_position().x << ", " << client->get_position().y << std::endl;
    std::cout << "Total clients = " << clients_.size() << std::endl;
    // start the game if all clients are connected
    if (clients_.size() == MAX_CLIENTS)
    {
      std::cout << "Starting the game" << std::endl;
      // run in a separate thread
      std::thread t(&room::run, this);
      t.detach();
    }
  }

  void run()
  {
    while (true)
    {
      std::cout << "Running" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      for (size_t i = 0; i < WIDTH; i++)
      {
        for (size_t j = 0; j < HEIGHT; j++)
        {
          // if there is more than one client in the cell
          if (field_[i][j].size() > 1)
          {
            // iterate through the clients in the cell
            for (auto client : field_[i][j])
            {
              // decrease the score of the client
              client->set_score(client->get_score() - 5);
              // create a random position for the client
              position pos = create_a_random_position();
              client->set_position(pos);
              // remove client from the field
              field_[i][j].erase(client);
              // add client to the field
              field_[pos.x][pos.y].insert(client);
            }
          }
          else
          {
            // if there is only one client in the cell
            for (auto client : field_[i][j])
            {
              // increase the score of the client
              client->set_score(client->get_score() + 1);
            }
          }
        }
      }

      display_field();
      // send position to all clients
      for (auto client : clients_)
      {
        message_body msg_body = create_a_position_message_body(client->get_position());
        message msg = create_a_message_from_message_body(msg_body);
        client->deliver(msg);
      }
    }
  }

  void leave(client_ptr client)
  {
    clients_.erase(client);
    // remove client from the field
    position pos = client->get_position();
    field_[pos.x][pos.y].erase(client);

    std::cout << "Client left" << std::endl;
    std::cout << "Total clients = " << clients_.size() << std::endl;

    if (clients_.size() == 0)
    {
      std::cout << "Room is empty exiting.." << std::endl;
      exit(0);
    }
  }

  void on_new_message(const message &msg, client_ptr client)
  {
    message_body body = decode_message_body(msg.body());
    if (body.type == message_type::movement_type)
    {
      // update the position of the client
      position pos = client->get_position();
      position old_pos = pos;

      uptade_position(body.dir, pos);

      // remove client from the field
      field_[old_pos.x][old_pos.y].erase(client);
      // add client to the field
      field_[pos.x][pos.y].insert(client);
      // update the position of the client
      client->set_position(pos);
    }

    else if (body.type == message_type::score_type)
    {
      std::cout << "Client wants to know his score and leave" << std::endl;
      // send score to the client
      message_body msg_body = create_a_score_message_body(client->get_score());
      message msg = create_a_message_from_message_body(msg_body);
      client->deliver(msg);
    }
  }

  void uptade_position(direction &dir, position &pos)
  {
    switch (dir)
    {
    case direction::up:
      pos.y -= 1;
      if (pos.y < 0)
        pos.y = HEIGHT - 1;
      break;
    case direction::down:
      pos.y += 1;
      if (pos.y > HEIGHT - 1)
        pos.y = 0;
      break;
    case direction::left:
      pos.x -= 1;
      if (pos.x < 0)
        pos.x = WIDTH - 1;
      break;
    case direction::right:
      pos.x += 1;
      if (pos.x > WIDTH - 1)
        pos.x = 0;
      break;
    default:
      break;
    }
  }

private:
  std::set<client_ptr> clients_;
  std::vector<std::vector<std::set<client_ptr>>> field_;
};

//----------------------------------------------------------------------

class session
    : public client,
      public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket, room &room)
      : socket_(std::move(socket)),
        room_(room)
  {
  }

  void start()
  {
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const message &msg)
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
  room &room_;
  message read_msg_;
  message_queue write_msgs_;
};

//----------------------------------------------------------------------

class server
{
public:
  server(boost::asio::io_context &io_context,
         const tcp::endpoint &endpoint)
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
int main(int argc, char *argv[])
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
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}