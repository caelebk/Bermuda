#include "saving_system.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>

#include "common.hpp"
#include "json.hpp"
#include "level_builder.hpp"
#include "level_factories.hpp"
#include "level_system.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "player_hud.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "respawn.hpp"
#include "room_builder.hpp"

using nlohmann::json;

static const std::string BERMUDA_SAVE_NAME =
    std::string(PROJECT_SOURCE_DIR) + "bermuda_save.json";
static LevelBuilder* level_builder;
static LevelSystem*  level_system;
static RenderSystem* renderer;

//////////////////////////////////////////////////////////////////////
// Game Saving
//////////////////////////////////////////////////////////////////////
/**
 * @brief initializes the local level builder. Do it this way so we don't have
 * to keep passing shit around
 *
 * @param lb
 */
void init_save_system(LevelBuilder* lb, LevelSystem* ls, RenderSystem* r) {
  level_builder = lb;
  level_system  = ls;
  renderer      = r;
}

/**
 * @brief Serializes a position struct to a json
 *
 * @param j
 * @param position
 */
static void position_to_json(json& save_file, Position& position) {
  save_file["position"] = {{"x", position.position.x},
                           {"y", position.position.y},
                           {"angle", position.angle},
                           {"scale.x", position.scale.x},
                           {"scale.y", position.scale.y},
                           {"originalScale.x", position.originalScale.x},
                           {"originalScale.y", position.originalScale.y}};
}

/**
 * @brief Serializes a entity state struct to json
 *
 * @param j
 * @param state
 */
static void entity_state_to_json(json& save_file, EntityState& state) {
  save_file["oxygen"] = state.oxygen;
  position_to_json(save_file, state.position);
  save_file["group"] = state.group;
  save_file["type"]  = state.type;
}

/**
 * @brief Saves meta data to the save data that is useful for debugging
 *
 * @param save_file
 * @return
 */
static bool save_meta_info(json& save_file) {
  // get seed
  save_file["meta"]["seed"] = getGlobalRandomSeed();
  // get time of save
  auto              now       = std::chrono::system_clock::now();
  auto              in_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm*          time_info = std::localtime(&in_time_t);
  std::stringstream ss;
  ss << std::put_time(time_info, "%Y-%m-%d %H:%M:%S");
  std::string formatted_time     = ss.str();
  save_file["meta"]["save_time"] = formatted_time;
  // git hash in lieu of version number
  save_file["meta"]["hash"] = GIT_HASH;
  return true;
}

/**
 * @brief Serializes and saves the player's relevant info
 *
 * @param save_file
 * @return
 */
static bool save_player_info(json& save_file) {
  Position&  position  = registry.positions.get(player);
  Inventory& inventory = registry.inventory.get(player);
  position_to_json(save_file["player"], position);
  save_file["player"]["inventory"] = {
      {"nets", inventory.nets},          {"concussors", inventory.concussors},
      {"torpedos", inventory.torpedos},  {"shrimp", inventory.shrimp},
      {"redKey", inventory.redKey},      {"blueKey", inventory.blueKey},
      {"yellowKey", inventory.yellowKey}};
  Oxygen& oxygen                = registry.oxygen.get(player);
  save_file["player"]["oxygen"] = oxygen.level;

  return true;
}

/**
 * @brief Serializes and saves the room's relevant info
 *
 * @param save_file
 * @param room
 * @return
 */
static bool save_room_info(json& save_file, RoomBuilder& room) {
  save_file["has_entered"]      = room.has_entered;
  save_file["is_tutorial_room"] = room.is_tutorial_room;
  save_file["is_boss_room"]     = room.is_boss_room;

  for (size_t i = 0; i < room.saved_entities.size(); i++) {
    entity_state_to_json(save_file["saved_enemies"][i],
                         room.saved_entities[i].es);
  }

  return true;
}

static bool save_current_room(json& save_file, std::string id,
                              RoomBuilder& room) {
  std::cout << "saving current room" << std::endl;
  save_file[id]["has_entered"]      = room.has_entered;
  save_file[id]["is_tutorial_room"] = room.is_tutorial_room;
  save_file[id]["is_boss_room"]     = room.is_boss_room;

  int idx = 0;

  for (Entity e : registry.deadlys.entities) {
    EntitySave es = EntitySave(e);
    entity_state_to_json(save_file[id]["saved_enemies"][idx++], es.es);
  }

  for (Entity e : registry.consumables.entities) {
    EntitySave es = EntitySave(e);
    entity_state_to_json(save_file[id]["saved_enemies"][idx++], es.es);
  }

  for (Entity e : registry.items.entities) {
    EntitySave es = EntitySave(e);
    entity_state_to_json(save_file[id]["saved_enemies"][idx++], es.es);
  }

  for (Entity e : registry.interactable.entities) {
    EntitySave es = EntitySave(e);
    entity_state_to_json(save_file[id]["saved_enemies"][idx++], es.es);
  }

  for (Entity e : registry.breakables.entities) {
    EntitySave es = EntitySave(e);
    entity_state_to_json(save_file[id]["saved_enemies"][idx++], es.es);
  }

  return true;
}

/**
 * @brief Goes through the entire game and serializes all level-related
 * information
 *
 * @param save_file
 * @return
 */
static bool save_level_info(json& save_file) {
  std::unordered_map<std::string, RoomBuilder> rooms = level_builder->rooms;
  std::string current_room  = level_system->current_room_editor_id;
  save_file["current_room"] = current_room;
  for (auto it = rooms.begin(); it != rooms.end(); it++) {
    std::string  id   = it->first;
    RoomBuilder& room = it->second;
    if (id == "") {
      // evil hack
      continue;
    }
    if (current_room == id) {
      save_current_room(save_file["rooms"], id, room);
    } else {
      save_room_info(save_file["rooms"][id], room);
    }
  }

  return true;
}

/**
 * @brief Saves all game state to a json file
 *
 * @return
 */
bool save_game_to_file() {
  json save_file;
  if (!save_meta_info(save_file)) {
    std::cout << "Error saving to file" << std::endl;
    return false;
  }

  if (!save_player_info(save_file)) {
    std::cout << "Error saving to file" << std::endl;
    return false;
  }

  if (!save_level_info(save_file)) {
    std::cout << "Error saving to file" << std::endl;
    return false;
  }

  std::ofstream file(BERMUDA_SAVE_NAME);
  if (file.is_open()) {
    std::cout << "Saving to file" << std::endl;
    file << save_file.dump(4);
    file.close();
  }

  return true;
}

//////////////////////////////////////////////////////////////////////
// Game loading
//////////////////////////////////////////////////////////////////////

/**
 * @brief validates with a position in a json is valid - including some sanity
 * bounds checks
 *
 * @param save_file
 * @return
 */
static bool is_valid_position(json& save_file) {
  if (!save_file.contains("angle")) {
    return false;
  }
  if (!save_file.contains("originalScale.x")) {
    return false;
  }
  if (!save_file.contains("originalScale.y")) {
    return false;
  }
  if (!save_file.contains("scale.x")) {
    return false;
  }
  if (!save_file.contains("scale.y")) {
    return false;
  }
  if (!save_file.contains("x") || save_file["x"] < 0.f ||
      save_file["x"] > window_width_px) {
    return false;
  }
  if (!save_file.contains("y") || save_file["y"] < 0.f ||
      save_file["y"] > window_height_px) {
    return false;
  }

  return true;
}

/**
 * @brief Validates enemies in the json
 *
 * @param save_file
 * @return
 */
static bool validate_enemies(json& save_file) {
  for (auto& enemy : save_file["saved_enemies"]) {
    if (!enemy.contains("group") || !enemy.contains("oxygen") ||
        !enemy.contains("position") || !enemy.contains("type")) {
      return false;
    }

    if (!is_valid_position(enemy["position"])) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Checks if a save file is valid
 *
 * Do this in case the file save was unsuccessul
 * Also Peyton will probably try to mess with the save file to see if it
 * breaks anything
 *
 * @param save_file
 * @return
 */
static bool valid_save_file(json& save_file) {
  if (!save_file.contains("meta") || !save_file["meta"].contains("hash") ||
      !save_file["meta"].contains("seed")) {
    std::cout << "Invalid meta-data" << std::endl;
    return false;
  }

  if (!(save_file["meta"]["hash"] != "\"" + std::string(GIT_HASH) + "\"")) {
    // we do not support save files across different versions
    // because that is annoying to debug
    std::cout << "Invalid version" << std::endl;
    std::cout << save_file["meta"]["hash"] << std::endl;
    std::cout << std::string(GIT_HASH) << std::endl;
    return false;
  }

  // player specific checks
  if (!save_file.contains("player") ||
      !save_file["player"].contains("inventory") ||
      !save_file["player"].contains("position")) {
    std::cout << "Invalid player" << std::endl;
    return false;
  }

  if ((int)save_file["player"]["inventory"].size() !=
      (int)INVENTORY::PROJ_COUNT - 1) {
    // since using defaults isn't game breaking, just do a sanity length check
    std::cout << "Invalid inventory" << std::endl;
    std::cout << (int)save_file["player"]["inventory"].size() << std::endl;
    std::cout << (int)INVENTORY::PROJ_COUNT - 1 << std::endl;
    return false;
  }

  if (!is_valid_position(save_file["player"]["position"])) {
    std::cout << "Invalid player position" << std::endl;
    return false;
  }

  // room specific checks
  if (!save_file.contains("rooms") || !save_file.contains("current_room")) {
    std::cout << "Invalid rooms" << std::endl;
    return false;
  }

  int num_rooms = 0;
  for (int num : ROOM_CLUSTERS) {
    num_rooms += num;
  }

  if (num_rooms != ((int)save_file["rooms"].size())) {
    std::cout << "Invalid number of rooms" << std::endl;
    return false;
  }

  for (const auto& room : save_file["rooms"].items()) {
    if (!room.value().contains("has_entered") ||
        !room.value().contains("is_boss_room") ||
        !room.value().contains("is_tutorial_room")) {
      std::cout << "Invalid room content" << std::endl;
      return false;
    }

    if (room.value().contains("saved_enemies") &&
        !validate_enemies(room.value())) {
      std::cout << "Invalid enemies" << std::endl;
      return false;
    }
  }

  return true;
}

static struct Position load_position_from_json(json& save_file) {
  struct Position p;
  p.angle           = save_file["position"]["angle"];
  p.position.x      = save_file["position"]["x"];
  p.position.y      = save_file["position"]["y"];
  p.scale.x         = save_file["position"]["scale.x"];
  p.scale.y         = save_file["position"]["scale.y"];
  p.originalScale.x = save_file["position"]["originalScale.x"];
  p.originalScale.y = save_file["position"]["originalScale.y"];

  return p;
}

static bool load_player_from_json(json& save_file) {
  // update the player position
  Position pos = load_position_from_json(save_file);
  registry.positions.remove(player);
  registry.inventory.remove(player);
  registry.positions.insert(player, pos);

  // update the weapon
  Position& weapon = registry.positions.get(player_weapon);
  weapon.scale.x = abs(weapon.scale.x);
  weapon.position.x = abs(weapon.position.x);
  if (pos.position.x < 0) {
    weapon.position.x *= -1;
  }

  if (pos.scale.x < 0) {
    weapon.scale.x *= -1;
  }

  // also update the collision mesh
  Player& p = registry.players.get(player);
  registry.positions.remove(p.collisionMesh);
  registry.positions.insert(p.collisionMesh, pos);

  // update inventory
  Inventory& inventory = registry.inventory.emplace(player);
  inventory.nets       = save_file["inventory"]["nets"];
  inventory.shrimp     = save_file["inventory"]["shrimp"];
  inventory.torpedos   = save_file["inventory"]["torpedos"];
  inventory.concussors = save_file["inventory"]["concussors"];
  inventory.redKey     = save_file["inventory"]["redKey"];
  inventory.blueKey    = save_file["inventory"]["blueKey"];
  inventory.yellowKey  = save_file["inventory"]["yellowKey"];

  updateInventoryCounter(renderer, INVENTORY::NET);
  updateInventoryCounter(renderer, INVENTORY::SHRIMP);
  updateInventoryCounter(renderer, INVENTORY::TORPEDO);
  updateInventoryCounter(renderer, INVENTORY::CONCUSSIVE);
  updateInventoryCounter(renderer, INVENTORY::RED_KEY);
  updateInventoryCounter(renderer, INVENTORY::BLUE_KEY);
  updateInventoryCounter(renderer, INVENTORY::YELLOW_KEY);

  // update oxygen
  Oxygen& oxygen = registry.oxygen.get(player);
  float   diff   = (float)save_file["oxygen"] - oxygen.level;

  // This will also update the health bar
  if (abs(diff) != 0) {
    modifyOxygenAmount(player, diff);
  }

  return true;
}

static bool load_room_info(json& save_file, std::string id, RoomBuilder& room) {
  // don't need to worry about the boss/tutorial rooms because it should be coo
  // from the seed
  // room.has_entered = save_file[id]["has_entered"];
  if (save_file[id]["has_entered"]) {
    room.has_entered = true;
    std::cout << "entered" << std::endl;
  } else {
    room.has_entered = false;
    std::cout << "not" << std::endl;
  }

  if (!save_file[id].contains("saved_enemies")) {
    return true;
  }

  for (auto& enemy : save_file[id]["saved_enemies"]) {
    struct EntityState es;
    es.oxygen   = enemy["oxygen"];
    es.group    = enemy["group"];
    es.type     = enemy["type"];
    es.position = load_position_from_json(enemy);
    room.saved_entities.push_back(EntitySave(es));
  }

  return true;
}

/**
 * @brief Loads all game info from a file
 *
 * FOR NOW ASSUMES THAT IT RUNS AFTER A FRESH SYSTEM RESTART
 *
 * @return
 */
bool load_game_from_file() {
  std::ifstream file(BERMUDA_SAVE_NAME);

  if (file.is_open()) {
    try {
      // Parse the JSON from the file
      json save_file;
      file >> save_file;

      // validate it
      if (!valid_save_file(save_file)) {
        std::cout << "Failed to load save file: Invalid" << std::endl;
        return false;
      }
      std::cout << "Json is validated" << std::endl;

      // assume it's good from here, just nuke everything
      level_system->clear_all_state();
      std::cout << "cleared all past level state" << std::endl;

      // load players stuff
      load_player_from_json(save_file["player"]);
      std::cout << "player loaded" << std::endl;

      // load rooms stuff
      std::unordered_map<std::string, RoomBuilder> rooms = level_builder->rooms;
      for (auto it = rooms.begin(); it != rooms.end(); it++) {
        std::string  id   = it->first;
        RoomBuilder& room = level_builder->get_room_by_editor_id(id);
        if (id == "") {
          // evil hack
          continue;
        }
        load_room_info(save_file["rooms"], id, room);
        std::cout << "loaded room " << id << std::endl;
      }

      std::cout << "activating from save" << std::endl;
      level_system->activate_from_save(save_file["current_room"]);

    } catch (const nlohmann::json::parse_error& e) {
      std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
  } else {
    std::cout << "Failed to load save file: Failed to open" << std::endl;
    return false;
  }

  std::cout << "Successfully loaded from file" << std::endl;
  return true;
}
