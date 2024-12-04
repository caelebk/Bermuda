#pragma once

#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <set>

#include "respawn.hpp"
#include "level_builder.hpp"
#include "player_hud.hpp"

Direction get_opposite_direction(Direction direction);

/**
 * A high-level OOP to ECS wrapper API that facilitates level interactions during gameplay.
 */
class LevelSystem {
private:
    RenderSystem* renderer;

    void set_current_room_editor_id(std::string room_editor_id);

    void spawn();

    void activate_walls();
    void recalculate_current_room_locks(Entity& door, DoorConnection& door_connection);
    void activate_doors();
    void activate_floor();
    void activate_current_room();
    void deactivate_current_room(bool save=true);
        
    void deactivate_boundary(Entity& boundary);

    void move_player_to_door(Direction direction, Entity& door);
public:
    LevelSystem();
    
    LevelBuilder* level;

    std::function<void()> dialogue_function = [this](){tutorialRoomDialogue(renderer);}; 

    std::string current_room_editor_id;

    void init(RenderSystem* renderer, LevelBuilder* level);

    void assign_door_sprite(Entity& door, DoorConnection& door_connection);
    
    void set_dialogue(DoorConnection& door_connection);

    // Switches to the room pointed at the given DoorConnection.
    void enter_room(DoorConnection& door_connection);

    // Activates the starting room.
    void activate_starting_room();

    void activate_from_save(std::string id);
    void clear_all_state();

    void collect_key(Objective color);
};
