#pragma once

#include <string>
#include <vector>

#include "level_util.hpp"
#include "tiny_ecs_registry.hpp"

#define ROOM_ORIGIN_POS                                                        \
  vec2(1.75f * (window_width_px) / 22.f,                                       \
       0.25f * (window_height_px) /                                            \
           12.f) // Position of Room Origin (Top-Left Corner)
#define WALL_THICKNESS 16.f

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

    void respawn(RenderSystem* renderer);

    vec2 rejection_sample();
  public:
      Entity entity; // the room under construction.

      std::vector<Entity> walls; // the walls inherent to this room.
      std::unordered_map<EditorID, Entity> doors; // the doors inherent to this room.
      std::unordered_map<EditorID, Direction> connections; // the connections inherent to this room.

      std::string name; // the human-readable name of this room

      // A bunch of boolean flags.
      bool has_entered = false;
      bool is_tutorial_room = false;
      bool is_boss_room = false;

      // possible objectives for the room.
      std::vector<Objective> objectives = { Objective::NONE };

      // Functions to be called when entering a room for the first time.
      std::vector<SpawnFunctionWrapper> spawn_wrappers;

      std::vector<EntitySave> saved_entities; // the entities inherent to this room.

      RoomBuilder();

      RoomBuilder &up(int magnitude = 0);
      RoomBuilder &down(int magnitude = 0);
      RoomBuilder &left(int magnitude = 0);
      RoomBuilder &right(int magnitude = 0);
      RoomBuilder &door(EditorID s_id, int magnitude = 0);

      void spawn(RenderSystem* renderer);

      bool is_in_room(vec2 &position);
      std::vector<EditorID> get_connections_with_direction(Direction direction);
      vec2 get_random_position();
  };
