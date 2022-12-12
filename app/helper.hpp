
#ifndef HELPER_HPP
#define HELPER_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "json_struct.h"

#define UNUSED(expr) \
  do                 \
  {                  \
    (void)(expr);    \
  } while (0)
#define MAX_CLIENTS 6
#define WIDTH 10
#define HEIGHT 10

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

//----------------------------------------------------------------------

JS_ENUM(direction, up, down, left, right)

//----------------------------------------------------------------------

JS_ENUM(message_type, position_type, movement_type, score_type)

//----------------------------------------------------------------------

typedef struct
{
  int x;
  int y;

  JS_OBJ(x, y);
} position;

typedef struct
{
  message_type type;
  position pos;
  direction dir;
  int score;

  JS_OBJ(type, pos, dir, score);
} message_body;

//----------------------------------------------------------------------

position create_a_random_position();

//----------------------------------------------------------------------

std::string direction_to_string(direction dir);

//----------------------------------------------------------------------

direction create_a_random_direction();

//----------------------------------------------------------------------

std::string message_type_to_string(message_type type);

//----------------------------------------------------------------------

message_body create_a_position_message_body(position pos);

//----------------------------------------------------------------------

message_body create_a_movement_message_body(direction dir);

//----------------------------------------------------------------------

message_body create_a_score_message_body(int score);

//----------------------------------------------------------------------

std::string encode_message_body(message_body msg_body);

//----------------------------------------------------------------------

message_body decode_message_body(const char *str);

//----------------------------------------------------------------------

message create_a_message_from_message_body(message_body msg_body);

//----------------------------------------------------------------------

JS_ENUM_DECLARE_STRING_PARSER(direction)

//----------------------------------------------------------------------

JS_ENUM_DECLARE_STRING_PARSER(message_type)

#endif // HELPER_HPP
