#include "helper.hpp"
#include "json_struct.h"

position create_a_random_position()
{
  position pos;
  pos.x = rand() % WIDTH;
  pos.y = rand() % HEIGHT;
  return pos;
}

//----------------------------------------------------------------------

std::string direction_to_string(direction dir)
{
  switch (dir)
  {
  case direction::up:
    return "up";
  case direction::down:
    return "down";
  case direction::left:
    return "left";
  case direction::right:
    return "right";
  default:
    return "unknown";
  }
}

//----------------------------------------------------------------------

direction create_a_random_direction()
{
  return static_cast<direction>(rand() % 4);
}

//----------------------------------------------------------------------

std::string message_type_to_string(message_type type)
{
  switch (type)
  {
  case message_type::position_type:
    return "position_type";
  case message_type::movement_type:
    return "movement_type";
  case message_type::score_type:
    return "score_type";
  default:
    return "unknown";
  }
}

//----------------------------------------------------------------------

message_body create_a_position_message_body(position pos)
{
  message_body msg_body;
  msg_body.type = message_type::position_type;
  msg_body.pos = pos;
  msg_body.dir = direction::up; // not used
  msg_body.score = 0;           // not used
  return msg_body;
}

//----------------------------------------------------------------------

message_body create_a_movement_message_body(direction dir)
{
  message_body msg_body;
  msg_body.type = message_type::movement_type;
  msg_body.pos = position{0, 0}; // not used
  msg_body.dir = dir;
  msg_body.score = 0; // not used
  return msg_body;
}

//----------------------------------------------------------------------

message_body create_a_score_message_body(int score)
{
  message_body msg_body;
  msg_body.type = message_type::score_type;
  msg_body.pos = position{0, 0}; // not used
  msg_body.dir = direction::up;  // not used
  msg_body.score = score;
  return msg_body;
}

//----------------------------------------------------------------------

std::string encode_message_body(message_body msg_body)
{
  std::string pretty_json = JS::serializeStruct(msg_body);
  return pretty_json;
}

//----------------------------------------------------------------------

message_body decode_message_body(const char *str)
{
  std::string body_string(str);
  JS::ParseContext context(body_string);
  message_body msg_body;
  context.parseTo(msg_body);
  return msg_body;
}

//----------------------------------------------------------------------

message create_a_message_from_message_body(message_body msg_body)
{
  std::string pretty_json = encode_message_body(msg_body);
  const void *body = pretty_json.c_str();

  message msg;
  msg.body_length(std::strlen(static_cast<char const *>(body)));
  std::memcpy(msg.body(), body, msg.body_length());
  msg.encode_header();
  return msg;
}
