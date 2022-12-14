#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <ctime>
#include <mutex>
#include <thread>
#include <boost/asio.hpp>
#include "helper.hpp"
#include <ncurses.h>

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<message> message_queue;

//----------------------------------------------------------------------
class client
{
public:
  client() : id_(0),
             position_(position{0, 0}),
             last_positiopn_(position{0, 0}),
             score_(0),
             color_(1)
  {
  }
  virtual ~client() {}
  virtual void deliver(const message &msg) = 0;
  virtual void close() = 0;
  void set_id(int id) { id_ = id; }
  int get_id() { return id_; }
  void set_position(position position) { position_ = position; }
  position get_position() { return position_; }
  void set_last_position(position position) { last_positiopn_ = position; }
  position get_last_position() { return last_positiopn_; }
  void set_score(int score) { score_ = score; }
  int get_score() { return score_; }
  void set_color(int color) { color_ = color; }
  int get_color() { return color_; }

private:
  int id_;
  position position_;
  position last_positiopn_;
  int score_;
  int color_;
};

typedef std::shared_ptr<client> client_ptr;

//----------------------------------------------------------------------

namespace ncr
{
  void init()
  {
    initscr();

    // if console does not support colors, exit
    if (!has_colors())
    {
      endwin();
      std::cout << "Your terminal does not support color" << std::endl;
      exit(1);
    }

    start_color();
    use_default_colors();

    // init every color pair (7 colors + default), we use the default color for the background
    for (int i = 0; i < 8; i++)
    {
      init_pair(i + 1, i, -1);
    }

    // init default color pair
    init_pair(8, -1, -1);

    // hide cursor
    curs_set(0);
  }

  void end()
  {
    endwin();
  }

  void reset_color()
  {
    attron(COLOR_PAIR(8));
  }

  void print_top_bar(const char *fmt)
  {
    move(0, 0);
    clrtoeol();
    printw("%s", fmt);
    refresh();
  }

  void print_bottom_bar(const char *fmt)
  {
    move(HEIGHT + 3, 0);
    clrtoeol();
    printw("%s", fmt);
    refresh();
  }

  void print_debug(const char *fmt)
  {
    move(HEIGHT + 4, 0);
    clrtoeol();
    printw("%s", fmt);
    refresh();
  }

  void display_field_once()
  {
    move(2, 0);
    for (size_t i = 0; i < WIDTH; i++)
    {
      for (size_t j = 0; j < HEIGHT; j++)
      {
        printw("X ");
      }
      printw("\n");
    }
    refresh();
  }

  void refresh_field(std::vector<std::vector<std::set<client_ptr>>> &field)
  {
    for (size_t i = 0; i < WIDTH; i++)
    {
      for (size_t j = 0; j < HEIGHT; j++)
      {
        if (field[i][j].size() >= 1)
        {
          for (client_ptr client : field[i][j])
          {
            position old_position = client->get_last_position();

            // reset old position if empty
            if (field[old_position.x][old_position.y].size() == 0)
            {
              move(old_position.y + 2, old_position.x * 2);
              printw("X");
            }
          }

          move(j + 2, i * 2);
          attron(COLOR_PAIR(field[i][j].begin()->get()->get_color()));
          printw("%d", field[i][j].begin()->get()->get_id());

          reset_color();
        }
        else
        {
          move(j + 2, i * 2);
          printw("X");
        }
      }
    }
    refresh();
  }

  void display_scores_and_positions(std::set<client_ptr> &clients)
  {
    move(HEIGHT + 5, 0);
    for (client_ptr client : clients)
    {
      printw("Client %d (%d, %d): %d\n", client.get()->get_id(), client.get()->get_position().x, client.get()->get_position().y, client.get()->get_score());
    }
    clrtoeol();
    refresh();
  }

  int create_a_random_color()
  {
    return rand() % 7 + 1;
  }
}

class room
{
public:
  room()
  {
    ncr::print_top_bar("Waiting for clients to connect...");
    display_total_clients();

    // init the field
    field_ = std::vector<std::vector<std::set<client_ptr>>>(WIDTH, std::vector<std::set<client_ptr>>(HEIGHT, std::set<client_ptr>()));

    display_field_once();
  }

  void join(client_ptr client)
  {
    mtx_.lock();

    if (clients_.size() >= MAX_CLIENTS)
    {
      ncr::print_top_bar("Room is full");
      client->close();
      mtx_.unlock();
      return;
    }

    if (game_started_)
    {
      ncr::print_top_bar("Game has already started");
      client->close();
      mtx_.unlock();
      return;
    }

    configure_new_client(client);
    insert_client(client);

    std::string out = "Client " + std::to_string(client->get_id()) + " joined with position: " + std::to_string(client->get_position().x) + ", " + std::to_string(client->get_position().y);
    ncr::print_top_bar(out.c_str());

    display_total_clients();
    refresh_filed();

    // start the game if all clients are connected
    if (clients_.size() == MAX_CLIENTS)
    {
      ncr::print_top_bar("Starting game...");

      // run in a separate thread
      std::thread t(&room::run, this);
      t.detach();
    }
    mtx_.unlock();
  }

  void leave(client_ptr client)
  {
    mtx_.lock();

    clients_.erase(client);

    position pos = client->get_position();
    position old_pos = client->get_last_position();

    field_[pos.x][pos.y].erase(client);
    field_[old_pos.x][old_pos.y].erase(client);

    std::string out = "Client " + std::to_string(client->get_id()) + " left";
    ncr::print_top_bar(out.c_str());

    display_total_clients();

    if (clients_.size() == 0)
    {
      ncr::print_top_bar("No clients left. Exiting...");
      ncr::end();
      exit(0);
    }

    mtx_.unlock();
  }

  void on_new_message(const message &msg, client_ptr client)
  {
    message_body body = decode_message_body(msg.body());

    if (body.type == message_type::movement_type)
    {
      position pos = client->get_position();
      position old_pos = pos;

      uptade_position(body.dir, pos);

      mtx_.lock();

      // update position in the field
      field_[old_pos.x][old_pos.y].erase(client);
      field_[pos.x][pos.y].insert(client);

      mtx_.unlock();

      // update the position of the client
      client->set_position(pos);
      client->set_last_position(old_pos);
    }

    else if (body.type == message_type::score_type)
    {
      std::string out = "Client " + std::to_string(client->get_id()) + " wants to know his score";
      ncr::print_top_bar(out.c_str());

      // send score to the client
      message_body msg_body = create_a_score_message_body(client->get_score());
      message msg = create_a_message_from_message_body(msg_body);
      client->deliver(msg);
    }
  }

private:
  void display_field_once()
  {
    ncr::display_field_once();
  }

  void refresh_filed()
  {
    ncr::refresh_field(field_);
  }

  void display_total_clients()
  {
    std::string out = "Total clients = " + std::to_string(clients_.size()) + " / " + std::to_string(MAX_CLIENTS);
    ncr::print_bottom_bar(out.c_str());
  }

  void display_scores_and_positions()
  {
    ncr::display_scores_and_positions(clients_);
  }

  void configure_new_client(client_ptr &client)
  {
    position pos = create_a_random_position();
    client->set_position(pos);
    client->set_last_position(pos);
    client->set_id(clients_.size());
    client->set_color(ncr::create_a_random_color());
  }

  void insert_client(client_ptr client)
  {
    position pos = client->get_position();
    // add client to the field and to the clients set
    field_[pos.x][pos.y].insert(client);
    clients_.insert(client);
  }

  void calculate_score()
  {
    // create a vector to store the clients that will be updated
    std::vector<client_ptr> clients_to_remove;
    for (size_t i = 0; i < WIDTH; i++)
    {
      for (size_t j = 0; j < HEIGHT; j++)
      {
        // if there is more than one client in the cell decrease the score of all clients
        if (field_[i][j].size() > 1)
        {
          for (client_ptr client : field_[i][j])
          {
            clients_to_remove.push_back(client);
          }
        }
        // if there is only one client in the cell increase the score of the client
        else if (field_[i][j].size() == 1)
        {

          for (client_ptr client : field_[i][j])
          {
            client->set_score(client->get_score() + 1);
          }
        }
      }
    }

    for (client_ptr client : clients_to_remove)
    {
      // decrease the score of the client
      client->set_score(client->get_score() - 5);

      // create a random position for the client
      position current_pos = client->get_position();
      client->set_last_position(current_pos);
      position pos = create_a_random_position();
      client->set_position(pos);

      // update position in the field
      field_[current_pos.x][current_pos.y].erase(client);
      field_[pos.x][pos.y].insert(client);
    }
  }

  void send_positions_to_clients()
  {
    for (client_ptr client : clients_)
    {
      message_body msg_body = create_a_position_message_body(client->get_position());
      message msg = create_a_message_from_message_body(msg_body);
      client->deliver(msg);
    }
  }

  void run()
  {
    if (game_started_)
    {
      return;
    }
    game_started_ = true;
    ncr::print_top_bar("Running game...");
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      mtx_.lock();
      refresh_filed();
      calculate_score();
      display_scores_and_positions();
      mtx_.unlock();

      send_positions_to_clients();
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

  bool game_started_ = false;
  std::mutex mtx_;
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

  void close()
  {
    socket_.close();
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
  srand((unsigned int)time(NULL));
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: server <port> [<port> ...]\n";
      return 1;
    }

    ncr::init();
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
    ncr::end();
    std::cerr << "Exception: " << e.what() << "\n";
  }

  ncr::end();
  return 0;
}