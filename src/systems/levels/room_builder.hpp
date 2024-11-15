#pragma once

#include <string>
#include <vector>

#include "tiny_ecs_registry.hpp"

#define ROOM_ORIGIN_POS                                                        \
  vec2(1.75f * (window_width_px) / 22.f,                                       \
       0.25f * (window_height_px) /                                            \
           12.f) // Position of Room Origin (Top-Left Corner)
#define WALL_THICKNESS 16.f

#define PASS [](){};

enum class Objective {
  KEY,
  DARK,
  PRESSURE_PLATE,
};

class RoomBuilder {
  protected:
    std::function<vec2(int)> direction; // the last direction of a RoomBuilder boundary construction call.
    vec2 pointer; // the absolute position of where the 'cursor' is, i.e it's position is that of the last wall added to this RoomBuilder.

    void update_bounding_box(Vector &vector);
    Entity make_boundary(int magnitude);
    RoomBuilder &add_wall(int magnitude);

    vec2 get_updated_up_position(int magnitude);
    vec2 get_updated_down_position(int magnitude);
    vec2 get_updated_right_position(int magnitude);
    vec2 get_updated_left_position(int magnitude);

    vec2 rejection_sample();
  public:
      Entity entity; // the room under construction.

      std::vector<Entity> walls; // the walls inherent to this room.
      std::unordered_map<std::string, Entity> doors; // the doors inherent to this room.

      // A bunch of boolean flags.
      bool has_entered;
      bool is_tutorial_room = false;
      bool is_boss_room = false;

      // room_spawn_function and fixed_room_spawn_function are always ran when you enter a room for the first time.
      std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> room_spawn_function;
      // boss_spawn_function is only ran if is_boss_room is true.
      std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> boss_spawn_function;
      
      std::vector<EntitySave> saved_entities; // the entities inherent to this room.

      RoomBuilder();

      RoomBuilder &up(int magnitude = 0);
      RoomBuilder &down(int magnitude = 0);
      RoomBuilder &left(int magnitude = 0);
      RoomBuilder &right(int magnitude = 0);
      RoomBuilder &door(std::string s_id, int magnitude = 0);

      // void add_objective(Objective objective);

      std::vector<Entity> get_doors();

      bool is_in_room(vec2 &position);
      vec2 get_random_position();
  };
