// Header
#include "world_system.hpp"
#include "common.hpp"
#include "death.hpp"
#include "debuff.hpp"
#include "enemy_factories.hpp"
#include "level_spawn.hpp"
#include "oxygen_system.hpp"
#include "physics_system.hpp"
#include "audio_system.hpp"
#include "player_controls.hpp"
#include "player_factories.hpp"
#include "spawning.hpp"
#include "tiny_ecs_registry.hpp"
#include "player_physics.hpp"

// stlib
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////
// Game configuration
/////////////////////////////////////////////////////////////

// create the underwater world
WorldSystem::WorldSystem()
    : points(0), oxygen_timer(PLAYER_OXYGEN_DEPLETE_TIME_MS) {
  // Seeding rng with random device
  rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

  // destroy music components
  if (background_music != nullptr)
    Mix_FreeMusic(background_music);
  if (salmon_dead_sound != nullptr)
    Mix_FreeChunk(salmon_dead_sound);
  if (salmon_eat_sound != nullptr)
    Mix_FreeChunk(salmon_eat_sound);

  Mix_CloseAudio();

  // Destroy all created components
  registry.clear_all_components();

  // Close the window
  glfwDestroyWindow(window);
}

// Debugging
namespace {
void glfw_err_cb(int error, const char *desc) {
  fprintf(stderr, "%d: %s", error, desc);
}
} // namespace

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the
// renderer
GLFWwindow *WorldSystem::create_window() {
  ///////////////////////////////////////
  // Initialize GLFW
  glfwSetErrorCallback(glfw_err_cb);
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW");
    return nullptr;
  }

  //-------------------------------------------------------------------------
  // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3
  // and enable the glDebugMessageCallback to have OpenGL catch your mistakes
  // for you. GLFW / OGL Initialization
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_RESIZABLE, 0);

  // Create the main window (for rendering, keyboard, and mouse input)
  window = glfwCreateWindow(window_width_px, window_height_px, "Bermuda",
                            nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to glfwCreateWindow");
    return nullptr;
  }

  // Setting callbacks to member functions (that's why the redirect is needed)
  // Input is handled using GLFW, for more info see
  // http://www.glfw.org/docs/latest/input_guide.html
  glfwSetWindowUserPointer(window, this);
  auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3) {
    ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
  };
  auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1) {
    ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
  };
  auto mouse_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2) {
    ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2);
  };
  glfwSetKeyCallback(window, key_redirect);
  glfwSetCursorPosCallback(window, cursor_pos_redirect);
  glfwSetMouseButtonCallback(window, mouse_redirect);

  return window;
}

void WorldSystem::init(RenderSystem *renderer_arg) {
  this->renderer = renderer_arg;

  // Build All Pre-Designed Rooms
  level_builder = LevelBuilder();
  level_builder.buildRoomOne();
  level_builder.buildRoomTwo();

  // Set all states to default
  restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
  // Calculate 't value': time loop / loop duration
  float lerp = elapsed_ms_since_last_update / LOOP_DURATION;

  // Updating window title with points
  std::stringstream title_ss;
  title_ss << "Bermuda - Points: " << points;
  glfwSetWindowTitle(window, title_ss.str().c_str());

  // Remove debug info from the last step
  while (registry.debugComponents.entities.size() > 0)
    registry.remove_all_components_of(registry.debugComponents.entities.back());

  ////////////////////////////////////////////////////////
  // Processing the player state
  ////////////////////////////////////////////////////////
  assert(registry.screenStates.components.size() <= 1);

  update_debuffs(elapsed_ms_since_last_update);

  // Deplete oxygen when it is time...
  oxygen_timer =
      oxygen_drain(player, oxygen_timer, elapsed_ms_since_last_update);

  // Update gun and harpoon angle
  updateWepProjPos(mouse_pos, player, player_weapon, player_projectile);

  ////////////////////////////////////////////////////////
  // Processing general entity state
  ////////////////////////////////////////////////////////

  // handle death of entities
  if (update_death(elapsed_ms_since_last_update)) {
    restart_game();
    return true;
  }

  // Set player acceleration (If player is alive)
  if (!registry.deathTimers.has(player)) {
      setPlayerAcceleration(player);
  }
  else {
      registry.motions.get(player).acceleration = { 0.f,0.f };
  }

  // Apply Water friction
  applyWaterFriction(player);

  // Update player velocity with lerp
  calculatePlayerVelocity(player, lerp);

  // Update Entity positions with lerp
  for (Entity entity : registry.motions.entities)
  {
  // TODO: move when collisions are done
  for (Entity entity : registry.motions.entities) {
    if (!debuff_entity_can_move(entity)) {
      continue;
    }
    Motion &motion = registry.motions.get(entity);
    Position &position = registry.positions.get(entity);
    position.position += motion.velocity * lerp;
    if (registry.oxygen.has(entity) && entity != player)
    {
      // make sure health bars follow moving enemies
      updateHealthBarAndEnemyPos(entity);
    }
  }

  return true;
}

/**
 * @brief Reset the world state to its initial state
 */
void WorldSystem::restart_game() {

  /////////////////////////////////////////////
  // Debugging
  /////////////////////////////////////////////
  registry.list_all_components();
  printf("Restarting\n");

  /////////////////////////////////////////////
  // World Reset
  /////////////////////////////////////////////
  // Reset the game speed
  current_speed = 1.f;

  // Remove all entities that we created
  // All that have a motion, we could also iterate over all fish, eels, ... but
  // that would be more cumbersome
  for (Entity entity : registry.oxygen.entities) {
    registry.remove_all_components_of(registry.oxygen.get(entity).oxygenBar);
    registry.remove_all_components_of(registry.oxygen.get(entity).backgroundBar);
  }
  while (registry.motions.entities.size() > 0)
    registry.remove_all_components_of(registry.motions.entities.back());

  // Debugging for memory/component leaks
  registry.list_all_components();

  remove_all_entities();

  /////////////////////////////////////////////
  // World Generation
  /////////////////////////////////////////////
  // TODO: Change based on which room entered
  auto curr_room = level_builder.room(ROOM_ONE);
  curr_room.activate_room();

  player = createPlayer(renderer, {window_width_px / 2 - 140, window_height_px - 140}); // TODO: get player spawn position
  player_weapon = getPlayerWeapon(player);
  player_projectile = getPlayerProjectile(player);
  createOxygenTank(renderer, player, {47.5, window_height_px / 2}); // TODO: figure out oxygen tank position

  Entity jelly = createJellyPos(renderer, {window_width_px - 200, window_height_px / 2 - 150}); // TODO: REMOVE once enemy spawning fully implemented
  createJellyHealthBar(renderer, jelly);                                                        // TODO: REMOVE once enemy spawning fully implemented
  Entity fish = createFishPos(renderer, {window_width_px / 2 + 190, window_height_px - 300});   // TODO: REMOVE once enemy spawning fully implemented
  createFishHealthBar(renderer, fish);                                                          // TODO: REMOVE once enemy spawning fully implemented                                                        // TODO: REMOVE once enemy spawning fully implemented
}

/**
 * @brief Returns whether the game be over ?
 *
 * @return True if over, false if not
 */
bool WorldSystem::is_over() const {
  return bool(glfwWindowShouldClose(window));
}

/**
 * @brief On key callback
 *
 * @param key 
 * @param action 
 * @param mod 
 */
void WorldSystem::on_key(int key, int, int action, int mod) {
  // Player movement attributes
  Player& keys = registry.players.get(player);

  // Player oxygen attributes
  Oxygen &player_oxygen = registry.oxygen.get(player);

  /////////////////////////////////////
  // Menu
  /////////////////////////////////////
  // Resetting game
  if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    restart_game();
  }

  /////////////////////////////////////
  // Debugging
  /////////////////////////////////////
  if (key == GLFW_KEY_G) {
    if (action == GLFW_RELEASE)
      debugging.in_debug_mode = false;
    else
      debugging.in_debug_mode = true;
  }

  // TODO: REMOVE temporary key input to switch between rooms (facilitate
  // collision testing)
  if (key == GLFW_KEY_1) {
    if (action == GLFW_PRESS)
      level_builder.room(ROOM_ONE).activate_room();
  }

  // TODO: REMOVE temporary key input to switch between rooms (facilitate
  // collision testing)
  if (key == GLFW_KEY_2) {
    if (action == GLFW_PRESS)
      level_builder.room(ROOM_TWO).activate_room();
  }

  // ESC to close game
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  //Music swap
  if (key == GLFW_KEY_M) {
    registry.musics.insert(Entity(), Music(background_music2));
  }
  if (key == GLFW_KEY_N) {
    registry.musics.insert(Entity(), Music(background_music));
  }

  // WASD Movement Keys
  if (!registry.deathTimers.has(player)) {
      if (key == GLFW_KEY_W) {
          if (action == GLFW_RELEASE) {
              keys.upHeld = false;
          }
          else {
              keys.upHeld = true;
          }
      }
      if (key == GLFW_KEY_S) {
          if (action == GLFW_RELEASE) {
              keys.downHeld = false;
          }
          else {
              keys.downHeld = true;
          }
      }
      if (key == GLFW_KEY_A) {
          if (action == GLFW_RELEASE) {
              keys.leftHeld = false;
          }
          else {
              keys.leftHeld = true;
          }
      }
      if (key == GLFW_KEY_D) {
          if (action == GLFW_RELEASE) {
              keys.rightHeld = false;
          }
          else {
              keys.rightHeld = true;
          }
      }
  }

  // Dashing (In case shift is held)
  if (key == GLFW_KEY_LEFT_SHIFT) {
      if (action == GLFW_PRESS) {
          registry.players.get(player).dashing = true;
          player_oxygen.rate = PLAYER_OXYGEN_RATE * 3;
      }
      else if (action == GLFW_RELEASE) {
          registry.players.get(player).dashing = false;
          player_oxygen.rate = PLAYER_OXYGEN_RATE;
      }
  }

  // Control the current speed with `<` `>`
  if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
      key == GLFW_KEY_COMMA) {
    current_speed -= 0.1f;
    printf("Current speed = %f\n", current_speed);
  }
  if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
      key == GLFW_KEY_PERIOD) {
    current_speed += 0.1f;
    printf("Current speed = %f\n", current_speed);
  }
  current_speed = fmax(0.f, current_speed);


  /////////////////////////////////////
  // Player
  /////////////////////////////////////
  player_movement(key, action, mod, player);
}

/**
 * @brief Mouse click callback
 *
 * @param button 
 * @param action 
 * @param mods 
 */
void WorldSystem::on_mouse_click(int button, int action, int mods) {
    // Shooting the projectile
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && registry.playerProjectiles.get(player_projectile).is_loaded) {
            if (!registry.deathTimers.has(player)) {
                setFiredProjVelo(player_projectile);
                modifyOxygen(player, player_weapon);
            }
        }
    }
  }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
  mouse_pos = mouse_position;
}
