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
#include "inventory_HUD.hpp"
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
#include <damage.hpp>
#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////
// Game configuration
/////////////////////////////////////////////////////////////

// create the underwater world
WorldSystem::WorldSystem()
    : oxygen_timer(PLAYER_OXYGEN_DEPLETE_TIME_MS) {
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

void WorldSystem::init(RenderSystem* renderer_arg, LevelBuilder* level_builder) {
  this->renderer = renderer_arg;
  this->level_builder = level_builder;

  restart_game();
  restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
  // Updating window title
  std::stringstream title_ss;
  title_ss << "Bermuda      FPS: "
           << int(1000.f / elapsed_ms_since_last_update);
  glfwSetWindowTitle(window, title_ss.str().c_str());

  // Remove debug info from the last step
  while (registry.debugComponents.entities.size() > 0)
    registry.remove_all_components_of(registry.debugComponents.entities.back());

  assert(registry.screenStates.components.size() <= 1);
  ScreenState& screen = registry.screenStates.components[0];

  if (!paused && !transitioning) {
    ////////////////////////////////////////////////////////
    // Processing the player state
    ////////////////////////////////////////////////////////
    if (registry.renderRequests.has(pause_menu)) {
      registry.renderRequests.remove(pause_menu);
    }

    for (Entity cursor : registry.cursors.entities) {
      if (registry.positions.has(cursor)) {
        Position& cursor_pos = registry.positions.get(cursor);
        cursor_pos.position  = vec2((float)mouse_pos.x, (float)mouse_pos.y);
      }
    }

    update_debuffs(elapsed_ms_since_last_update);
    update_attack(elapsed_ms_since_last_update);
    update_collision_timers(elapsed_ms_since_last_update);

    // Deplete oxygen when it is time...
    oxygen_timer = oxygen_drain(oxygen_timer, elapsed_ms_since_last_update);
    
    // Update the player's direction
    updatePlayerDirection(mouse_pos);

    // Update gun and projectile angle
    updateWepProjPos(mouse_pos);

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
        } else if (registry.drops.has(entity) &&
                   registry.positions.has(entity)) {
          Drop&     drop = registry.drops.get(entity);
          Position& pos  = registry.positions.get(entity);

          auto fn     = drop.dropFn;
          vec2 newPos = pos.position;
          registry.remove_all_components_of(entity);

          fn(renderer, newPos);
        } else {
          registry.remove_all_components_of(entity);
        }
      }
    }
    screen.darken_screen_factor = 1 - min_counter_ms / 3000;

    check_bounds();
  } else if (transitioning) {
    if (screen.darken_screen_factor < 1.f && registry.roomTransitions.has(rt_entity)) {
      screen.darken_screen_factor = min(1.f, screen.darken_screen_factor + 0.02f);
    } else if (screen.darken_screen_factor >= 1.f && registry.roomTransitions.has(rt_entity)) {
      RoomTransition& roomTransition = registry.roomTransitions.get(rt_entity);
      level_builder->enter_room(roomTransition.door_connection);
      registry.remove_all_components_of(rt_entity);
    } else if (screen.darken_screen_factor > 0.f && !registry.roomTransitions.has(rt_entity)) {
      screen.darken_screen_factor = max(0.f, screen.darken_screen_factor - 0.02f);
    } else if (screen.darken_screen_factor <= 0.f && !registry.roomTransitions.has(rt_entity)) {
      transitioning = false;
    }
  } else {
    if (!registry.renderRequests.has(pause_menu)) {
      registry.renderRequests.insert(
          pause_menu, {TEXTURE_ASSET_ID::PAUSE, EFFECT_ASSET_ID::TEXTURED,
                       GEOMETRY_BUFFER_ID::SPRITE});
    }
  }

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

  // Remove all entities that we created
  // All that have a motion, we could also iterate over all fish, eels, ...
  // but that would be more cumbersome
  for (Entity entity : registry.oxygen.entities) {
    registry.remove_all_components_of(registry.oxygen.get(entity).oxygenBar);
    registry.remove_all_components_of(
        registry.oxygen.get(entity).backgroundBar);
  }
  for (Entity pauseMenu : registry.pauseMenus.entities) {
    registry.remove_all_components_of(pauseMenu);
  }

  // Debugging for memory/component leaks
  remove_all_entities();
  registry.list_all_components();

  pause_menu = createPauseMenu(renderer);

  player = createPlayer(
      renderer,
      {150, window_height_px - 150});  // TODO: get player spawn position
  registry.inventory.emplace(player);
  createInvHUD(renderer, player);
  // init global variables
  player_weapon     = getPlayerWeapon();
  player_projectile = getPlayerProjectile();
  harpoon_gun       = player_weapon;
  harpoon           = player_projectile;
  wep_type          = PROJECTILES::HARPOON;
  net               = loadNet(renderer);
  concussive        = loadConcussive(renderer);
  torpedo           = loadTorpedo(renderer);
  shrimp            = loadShrimp(renderer);

  // Note: It's important that these are set after the projectile globals
  // becuase they're dependant on them being set
  net_gun =
      createConsumableGun(renderer, NET_GUN_OXYGEN_COST, PROJECTILES::NET);
  concussive_gun = createConsumableGun(renderer, CONCUSSIVE_GUN_OXYGEN_COST,
                                       PROJECTILES::CONCUSSIVE);
  torpedo_gun    = createConsumableGun(renderer, TORPEDO_GUN_OXYGEN_COST,
                                       PROJECTILES::TORPEDO);
  shrimp_gun     = createConsumableGun(renderer, SHRIMP_GUN_OXYGEN_COST,
                                       PROJECTILES::SHRIMP);

  createOxygenTank(
      renderer, player,
      {47.5, window_height_px / 2});

  /////////////////////////////////////////////
  // Enemy + Drops
  /////////////////////////////////////////////
  // spawn at random places in the room

  level_builder->activate_starting_room();

  paused = false;
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
  // Help / Pause
  /////////////////////////////////////
  // Toggling game pause
  if (action == GLFW_RELEASE && key == GLFW_KEY_P && !registry.deathTimers.has(player)) {
    paused = !paused;
    if (paused) {
      depleteOxygen(player);
    }
  }

  if (!paused) {
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

    // Handle weapon swapping
    handleWeaponSwapping(key);
  }

  // ESC to close game
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  /////////////////////////////////////
  // Player
  /////////////////////////////////////
  player_movement(key, action, mod);
}

/**
 * @brief Mouse click callback
 *
 * @param button
 * @param action
 * @param mods
 */
void WorldSystem::on_mouse_click(int button, int action, int mods) {
  if (!paused) {
    player_mouse(renderer, button, action, mods, harpoon, harpoon_gun);
  }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
  mouse_pos = mouse_position;
}
