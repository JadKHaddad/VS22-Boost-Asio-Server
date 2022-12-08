
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class message
{
public:
  enum
  {
    header_length = 4
  };
  enum
  {
    max_body_length = 512
  };

  message()
      : body_length_(0)
  {
  }

  const char *data() const
  {
    return data_;
  }

  char *data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + body_length_;
  }

  const char *body() const
  {
    return data_ + header_length;
  }

  char *body()
  {
    return data_ + header_length;
  }

  std::size_t body_length() const
  {
    return body_length_;
  }

  void body_length(std::size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  bool decode_header()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    body_length_ = std::atoi(header);
    if (body_length_ > max_body_length)
    {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  void encode_header()
  {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(body_length_));
    std::memcpy(data_, header, header_length);
  }

private:
  char data_[header_length + max_body_length];
  std::size_t body_length_;
};

enum direction
{
  up = 0,
  down = 1,
  left = 2,
  right = 3
};

class movement_message
{
public:
  movement_message(direction direction) : direction_(direction) {}
  direction direction_;
};

class position
{
public:
  position(int x, int y) : x_(x), y_(y) {}
  int x() { return x_; }
  int y() { return y_; }
  int x_;
  int y_;
};

class position_message
{
public:
  position_message(position position) : position_(position) {}
  position get_position() { return position_; }
  position position_;
};

class score_message
{
public:
  score_message(int score) : score_(score) {}
  int get_score() { return score_; }
  int score_;
};

enum message_type
{
  position_type,
  movement_type,
  score_type
};

class message_body
{
public:
  message_body() : type(position_type), position_msg(position_message(position(0, 0))), movement_msg(movement_message(up)), score_msg(score_message(0)) {}
  message_type type;
  position_message position_msg;
  movement_message movement_msg;
  score_message score_msg;
};
#endif // MESSAGE_HPP