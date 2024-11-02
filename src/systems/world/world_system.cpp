// Header
#include "world_system.hpp"

#include "audio_system.hpp"
#include "common.hpp"
#include "consumable_factories.hpp"
#include "consumable_utils.hpp"
#include "death.hpp"
#include "debuff.hpp"
#include "enemy_factories.hpp"
#include "enemy_util.hpp"
#include "level_spawn.hpp"
#include "map_factories.hpp"
#include "map_util.hpp"
#include "oxygen_system.hpp"
#include "physics_system.hpp"
#include "player_controls.hpp"
#include "player_factories.hpp"
#include "spawning.hpp"
#include "tiny_ecs_registry.hpp"

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
  // Destroy all created components
  registry.clear_all_components();

  // Close the window
  glfwDestroyWindow(window);
}

// Debugging
namespace {
void glfw_err_cb(int error, const char* desc) {
  fprintf(stderr, "%d: %s", error, desc);
}
}  // namespace

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the
// renderer
GLFWwindow* WorldSystem::create_window() {
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
  auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
    ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
  };
  auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
    ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
  };
  auto mouse_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) {
    ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2);
  };
  glfwSetKeyCallback(window, key_redirect);
  glfwSetCursorPosCallback(window, cursor_pos_redirect);
  glfwSetMouseButtonCallback(window, mouse_redirect);

  return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
  this->renderer = renderer_arg;

  level_builder = LevelBuilder();

  // Test the randomized room generation
  level_builder.generate_random_level({5, 5, 5}, {80, 50, 0});

  // Set all states to default
  curr_room =
      level_builder.room("0");  // TODO: change based on which room entered
  curr_room.activate_room();
  restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
  // Calculate 't value': time loop / loop duration
  float lerp = elapsed_ms_since_last_update / LOOP_DURATION;

  // Updating window title
  std::stringstream title_ss;
  title_ss << "Bermuda";
  glfwSetWindowTitle(window, title_ss.str().c_str());

  // Remove debug info from the last step
  while (registry.debugComponents.entities.size() > 0)
    registry.remove_all_components_of(registry.debugComponents.entities.back());

  ////////////////////////////////////////////////////////
  // Processing the player state
  ////////////////////////////////////////////////////////
  assert(registry.screenStates.components.size() <= 1);
  ScreenState& screen = registry.screenStates.components[0];

  update_debuffs(elapsed_ms_since_last_update);
  update_attack(elapsed_ms_since_last_update);

  // Deplete oxygen when it is time...
  oxygen_timer =
      oxygen_drain(player, oxygen_timer, elapsed_ms_since_last_update);

  // Update gun and harpoon angle
  updateWepProjPos(mouse_pos, player, player_weapon, player_projectile);

  float min_counter_ms = 4000.f;
  for (Entity entity : registry.deathTimers.entities) {
    // progress timer
    DeathTimer& counter = registry.deathTimers.get(entity);
    counter.counter_ms -= elapsed_ms_since_last_update;
    if (counter.counter_ms < min_counter_ms && entity == player) {
      min_counter_ms = counter.counter_ms;
    }

    // restart the game once the death timer expired
    if (counter.counter_ms < 0) {
      if (entity == player) {
        registry.deathTimers.remove(entity);
        screen.darken_screen_factor = 0;
        restart_game();
        return true;
      } else {
        registry.remove_all_components_of(entity);
      }
    }
  }

  // Set player acceleration (If player is alive)
  if (!registry.deathTimers.has(player)) {
    setPlayerAcceleration(player);
  } else {
    registry.motions.get(player).acceleration = {0.f, 0.f};
  }

  // Apply Water friction
  applyWaterFriction(player);

  // Update player velocity with lerp
  calculatePlayerVelocity(player, lerp);

  check_bounds();

  // Update Entity positions with lerp
  for (Entity entity : registry.motions.entities) {
    if (!debuff_entity_can_move(entity)) {
      continue;
    }
    Motion&   motion   = registry.motions.get(entity);
    Position& position = registry.positions.get(entity);
    position.position += motion.velocity * lerp;
    if (registry.oxygen.has(entity) && entity != player) {
      // make sure health bars follow moving enemies
      updateHealthBarAndEnemyPos(entity);
    }
  }
  screen.darken_screen_factor = 1 - min_counter_ms / 3000;
  return true;
}

// Temporary
void WorldSystem::check_bounds() {
  Position& player_position      = registry.positions.get(player);
  Position& player_proj_position = registry.positions.get(player_projectile);
  PlayerProjectile& player_proj =
      registry.playerProjectiles.get(player_projectile);
  float vertical   = player_position.scale.y / 2.0f;
  float horizontal = player_position.scale.x / 2.0f;

  if (player_position.position.x + horizontal > window_width_px) {
    player_position.position.x = window_width_px - horizontal;
  }
  if (player_position.position.x - horizontal < 0) {
    player_position.position.x = +horizontal;
  }
  if (player_position.position.y + vertical > window_height_px) {
    player_position.position.y = window_height_px - vertical;
  }
  if (player_position.position.y - vertical < 0) {
    player_position.position.y = vertical;
  }
  if (player_proj_position.position.x > window_width_px ||
      player_proj_position.position.x < 0 ||
      player_proj_position.position.y > window_height_px ||
      player_proj_position.position.y < 0) {
    player_proj.is_loaded = true;
  }
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
    registry.remove_all_components_of(
        registry.oxygen.get(entity).backgroundBar);
  }

  // Debugging for memory/component leaks
  remove_all_entities();
  registry.list_all_components();

  player = createPlayer(
      renderer,
      {130, window_height_px - 140});  // TODO: get player spawn position
  player_weapon     = getPlayerWeapon(player);
  player_projectile = getPlayerProjectile(player);
  createOxygenTank(
      renderer, player,
      {47.5, window_height_px / 2});  // TODO: figure out oxygen tank position

  /////////////////////////////////////////////
  // Enemy + Drops
  /////////////////////////////////////////////
  // spawn at random places in the room
  execute_config_rand(LVL_1_RAND_POS, curr_room, renderer);
  execute_config_rand_chance(LVL_1_RAND_POS, curr_room, renderer, 0.5);

  // // spawn at fixed positions in the room
  execute_config_fixed(LVL_1_FIXED);
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
    if (action == GLFW_PRESS && !(action == GLFW_REPEAT)) {
      curr_room.deactivate_room();
      curr_room = level_builder.room(
          "0");  // TODO: change based on which room entered
      curr_room.activate_room();
      restart_game();
    }
  }
  // TODO: REMOVE temporary key input to switch between rooms (facilitate
  // collision testing)
  if (key == GLFW_KEY_2) {
    if (action == GLFW_PRESS && !(action == GLFW_REPEAT)) {
      curr_room.deactivate_room();
      curr_room = level_builder.room(
          "1");  // TODO: change based on which room entered
      curr_room.activate_room();
      restart_game();
    }
  }

  // ESC to close game
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
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
  player_mouse(button, action, mods, player, player_weapon, player_projectile);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
  mouse_pos = mouse_position;
}
