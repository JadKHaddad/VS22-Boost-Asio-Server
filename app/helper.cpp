#include "helper.hpp"

position create_a_random_position()
{
  position pos;
  pos.x = rand() % WIDTH;
  pos.y = rand() % HEIGHT;
  return pos;
}

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

direction create_a_random_direction()
{
  return static_cast<direction>(rand() % 4);
}

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