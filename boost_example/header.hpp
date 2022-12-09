
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "json_struct.h"

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

typedef struct
{
  int x;
  int y;

  JS_OBJ(x, y);
} position;

JS_ENUM(direction, up, down, left, right)

JS_ENUM(message_type, position_type, movement_type, score_type)

typedef struct
{
  message_type type;
  position pos;
  direction dir;
  int score;

  JS_OBJ(type, pos, dir, score);
} message_body;
JS_ENUM_DECLARE_STRING_PARSER(direction)
JS_ENUM_DECLARE_STRING_PARSER(message_type)


#endif // MESSAGE_HPP
