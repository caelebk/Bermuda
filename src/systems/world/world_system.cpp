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
#include "player_hud.hpp"
#include "saving_system.hpp"
#include "spawning.hpp"
#include "text_factories.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_state.hpp"

// stlib
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdio>
#include <damage.hpp>
#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////
// Game configuration
/////////////////////////////////////////////////////////////

#define FPS_UPDATE_INTERVAL 500  // update every half-second
static std::stringstream title_ss;
static int               fps       = 0;
static float             fps_timer = 0;

// create the underwater world
WorldSystem::WorldSystem() : oxygen_timer(PLAYER_OXYGEN_DEPLETE_TIME_MS) {}

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
  auto scroll_redirect = [](GLFWwindow* wnd, double _0, double _1) {
    ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_scroll(_0, _1);
  };
  glfwSetKeyCallback(window, key_redirect);
  glfwSetCursorPosCallback(window, cursor_pos_redirect);
  glfwSetMouseButtonCallback(window, mouse_redirect);
  glfwSetScrollCallback(window, scroll_redirect);

  return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, LevelSystem* level) {
  this->renderer = renderer_arg;
  this->level    = level;

  restart_game();
}

void WorldSystem::update_fps(float elapsed_ms_since_last_update) {
  fps_timer -= elapsed_ms_since_last_update;
  if (fps_timer > 0.f) {
    return;
  }
  fps_timer    = FPS_UPDATE_INTERVAL;
  int curr_fps = int(1000.f / elapsed_ms_since_last_update);
  if (curr_fps != fps) {
    title_ss.str("");
    title_ss << "Bermuda      FPS: " << curr_fps;
    fps = curr_fps;
    glfwSetWindowTitle(window, title_ss.str().c_str());
  }
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
  // Updating window title
  update_fps(elapsed_ms_since_last_update);

  // Remove debug info from the last step
  while (registry.debugComponents.entities.size() > 0)
    registry.remove_all_components_of(registry.debugComponents.entities.back());

  assert(registry.screenStates.components.size() <= 1);
  ScreenState& screen = registry.screenStates.components[0];

  bool is_frozen_state = is_intro || is_start || is_paused ||
                         is_krab_cutscene || is_sharkman_cutscene || is_death ||
                         is_end || room_transitioning;

  if (!is_frozen_state) {
    ////////////////////////////////////////////////////////
    // Processing the player state
    ////////////////////////////////////////////////////////
    if (registry.renderRequests.has(overlay)) {
      registry.remove_all_components_of(overlay);
    }

    for (Entity cursor : registry.cursors.entities) {
      if (registry.positions.has(cursor)) {
        Position& cursor_pos = registry.positions.get(cursor);
        cursor_pos.position  = vec2((float)mouse_pos.x, (float)mouse_pos.y);
      }
    }

    // Geyser bubbles
    for (Entity entity : registry.geysers.entities) {
      Geyser& timer = registry.geysers.get(entity);
      timer.bubble_timer -= elapsed_ms_since_last_update;
      if (timer.bubble_timer <= 0.f) {
        timer.bubble_timer = BUBBLE_INTERVAL;
        Position& pos      = registry.positions.get(entity);
        createGeyserBubble(renderer, {pos.position.x + randomFloat(-10.f, 10.f),
                                      pos.position.y});
      }
    }

    // Torpedo Explosion VFX
    for (Entity entity : registry.explosions.entities) {
      Explosion& timer = registry.explosions.get(entity);
      timer.timer += elapsed_ms_since_last_update;
      if (timer.timer >= timer.expiry_time) {
        registry.remove_all_components_of(entity);
      } else {
        Position& pos = registry.positions.get(entity);
        pos.scale     = vec2(timer.timer / timer.expiry_time) * timer.full_scale;
      }
    }


    // Enemy Projectiles
    for (Entity entity : registry.enemyProjectiles.entities) {
      EnemyProjectile& timer = registry.enemyProjectiles.get(entity);
      if (timer.has_timer) {
        timer.timer -= elapsed_ms_since_last_update;
        if (timer.timer < 0.f) {
          registry.remove_all_components_of(entity);
        }
      }
    }

    update_debuffs(elapsed_ms_since_last_update);
    update_attack(elapsed_ms_since_last_update);
    update_collision_timers(elapsed_ms_since_last_update);
    update_notification_timers(elapsed_ms_since_last_update);

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

      if (counter.counter_ms < 0) {
        if (entity == player) {
          // show death overlay
          registry.deathTimers.remove(entity);
          overlay               = createOverlay(renderer);
          is_death              = true;
          overlay_transitioning = true;
          return true;
        } else if (registry.drops.has(entity) &&
                   registry.positions.has(entity)) {
          Drop&     drop = registry.drops.get(entity);
          Position& pos  = registry.positions.get(entity);

          auto fn     = drop.dropFn;
          vec2 newPos = pos.position;
          registry.remove_all_components_of(entity);

          fn(renderer, newPos, false);
        } else {
          registry.remove_all_components_of(entity);
        }
      }
    }
    screen.darken_screen_factor = 1 - min_counter_ms / 3000;

    check_bounds();
  } else if (is_start) {
    overlayState(TEXTURE_ASSET_ID::START_OVERLAY);
    // if intro triggered, darken the screen
    if (is_intro) {
      bool transition_complete =
          darkenTransitionState(screen, elapsed_ms_since_last_update);
      if (transition_complete) {
        // remove start overlay when transition is complete
        is_start = false;
        // transition intro screen in
        overlay_transitioning = true;
      }
    }
  } else if (is_intro) {
    if (overlay_transitioning) {
      bool brighten_complete =
          brightenTransitionState(screen, elapsed_ms_since_last_update);
      if (brighten_complete) {
        overlay_transitioning = false;
      }
    } else {
      // start timer after overlay transition complete
      overlay_timer = min(OVERLAY_TIMER_DURATION,
                          overlay_timer + elapsed_ms_since_last_update);
      if (overlay_timer >= OVERLAY_TIMER_DURATION) {
        bool transition_complete =
            darkenTransitionState(screen, elapsed_ms_since_last_update);
        if (transition_complete) {
          is_intro      = false;
          overlay_timer = 0.f;
          tutorialRoomDialogue(renderer);
        }
      }
    }

    if (registry.renderRequests.has(overlay)) {
      registry.renderRequests.remove(overlay);
    }
    overlayState(TEXTURE_ASSET_ID::INTRO_OVERLAY);
  } else if (is_krab_cutscene) {
    if (!registry.positions.has(overlay)) {
      overlay = createOverlay(renderer);
    }

    // play sound effect at start of transition
    if (!registry.renderRequests.has(overlay)) {
      registry.sounds.insert(overlay, Sound(SOUND_ASSET_ID::BOSS_INTRO, 10000));
    }
    if (room_transitioning) {
      roomTransitionState(renderer, screen, level,
                          elapsed_ms_since_last_update);
    } else {
      // start the timer when transition completes
      overlay_timer = min(OVERLAY_TIMER_DURATION,
                          overlay_timer + elapsed_ms_since_last_update);
      if (overlay_timer >= OVERLAY_TIMER_DURATION) {
        bool transition_complete =
            darkenTransitionState(screen, elapsed_ms_since_last_update);
        if (transition_complete) {
          is_krab_cutscene = false;
          overlay_timer    = 0.f;
          krabBossDialogue(renderer);
        }
      }
    }

    if (registry.renderRequests.has(overlay)) {
      registry.renderRequests.remove(overlay);
    }
    overlayState(TEXTURE_ASSET_ID::KRAB_OVERLAY);
  } else if (is_sharkman_cutscene) {
    if (!registry.positions.has(overlay)) {
      overlay = createOverlay(renderer);
    }

    // play sound effect at start of transition
    if (!registry.renderRequests.has(overlay)) {
      registry.sounds.insert(overlay, Sound(SOUND_ASSET_ID::BOSS_INTRO, 10000));
    }

    if (room_transitioning) {
      roomTransitionState(renderer, screen, level,
                          elapsed_ms_since_last_update);
    } else {
      // start the timer when transition completes
      overlay_timer = min(OVERLAY_TIMER_DURATION,
                          overlay_timer + elapsed_ms_since_last_update);
      if (overlay_timer >= OVERLAY_TIMER_DURATION) {
        bool transition_complete =
            darkenTransitionState(screen, elapsed_ms_since_last_update);
        if (transition_complete) {
          is_sharkman_cutscene = false;
          overlay_timer        = 0.f;
          sharkmanBossDialogue(renderer);
        }
      }
    }

    if (registry.renderRequests.has(overlay)) {
      registry.renderRequests.remove(overlay);
    }
    overlayState(TEXTURE_ASSET_ID::SHARKMAN_OVERLAY);
  } else if (is_death) {
    // play sound effect at start of transition
    if (!registry.renderRequests.has(overlay)) {
      registry.sounds.insert(overlay, Sound(SOUND_ASSET_ID::END_SCREEN, 10000));
    }
    if (overlay_transitioning) {
      bool brighten_complete =
          brightenTransitionState(screen, elapsed_ms_since_last_update);
      if (brighten_complete) {
        overlay_transitioning = false;
      }
    }
    if (registry.renderRequests.has(overlay)) {
      registry.renderRequests.remove(overlay);
    }
    overlayState(TEXTURE_ASSET_ID::DEATH_OVERLAY);
  } else if (is_end) {
    if (!registry.positions.has(overlay)) {
      overlay = createOverlay(renderer);
    }

    if (room_transitioning) {
      roomTransitionState(renderer, screen, level,
                          elapsed_ms_since_last_update);
    }
    // TODO: adjust when final boss implemented
    // if (overlay_transitioning) {
    //   bool brighten_complete =
    //       brightenTransitionState(screen, elapsed_ms_since_last_update);
    //   if (brighten_complete) {
    //     overlay_transitioning = false;
    //   }
    // }
    if (registry.renderRequests.has(overlay)) {
      registry.renderRequests.remove(overlay);
    }
    overlayState(TEXTURE_ASSET_ID::END_OVERLAY);
  } else if (room_transitioning) {
    roomTransitionState(renderer, screen, level, elapsed_ms_since_last_update);
  } else {
    overlayState(TEXTURE_ASSET_ID::PAUSE_OVERLAY);
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
  // registry.list_all_components();
  printf("Restarting\n");

  /////////////////////////////////////////////
  // World Reset
  /////////////////////////////////////////////

  // Debugging for memory/component leaks
  remove_all_entities();

  // registry.list_all_components();

  registry.remove_all_components_of(overlay);
  overlay               = createOverlay(renderer);
  is_death              = false;
  is_end                = false;
  is_paused             = false;
  krab_boss_encountered = false;
  sharkman_encountered  = false;

  player = createPlayer(
      renderer,
      {window_width_px / 2.f + 22.f,
       window_height_px / 2.f - 43.f});  // TODO: get player spawn position
  registry.inventory.emplace(player);
  createInventoryHud(renderer);
  createCommunicationHud(renderer);
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

  createOxygenTank(renderer, player, {47.5, window_height_px * 0.45f});

  // ugly number for the y position but the spacing looks fairly even
  createDashIndicator(renderer, player, {47.5, window_height_px * 0.78f});
  /////////////////////////////////////////////
  // Enemy + Drops
  /////////////////////////////////////////////
  // spawn at random places in the room

  level->activate_starting_room();
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
  bool is_frozen_state = is_intro || is_start || is_paused ||
                         is_krab_cutscene || is_sharkman_cutscene || is_death ||
                         is_end || room_transitioning;
  bool cannot_pause_state = is_intro || is_start || is_krab_cutscene ||
                            is_sharkman_cutscene || is_death || is_end ||
                            room_transitioning;
  bool cannot_reset_state = is_intro || is_start || is_krab_cutscene ||
                            is_sharkman_cutscene || room_transitioning;

  // ESC to close game (unconditional quit left commented for debugging)
  // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
  //   glfwSetWindowShouldClose(window, GL_TRUE);
  // }

  // Start Game
  if (is_start) {
    if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
      // start fresh
      is_intro       = true;
      game_started   = true;
      load_from_save = false;
    } else if (action == GLFW_RELEASE && key == GLFW_KEY_L) {
      // start from save
      is_start = false;  // remove start overlay
      is_intro =
          false;  // no cutscene because andy said "nah" and requested changes
      game_started   = true;
      load_from_save = true;
    }
    return;
  }

  // Start Game or skip cutscene.
  if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE &&
      !registry.deathTimers.has(player)) {
    if (is_start) {
      is_intro = true;
    } else if (is_intro) {
      overlay_timer = OVERLAY_TIMER_DURATION;
    } else if (is_krab_cutscene) {
      overlay_timer = OVERLAY_TIMER_DURATION;
    } else if (is_sharkman_cutscene) {
      overlay_timer = OVERLAY_TIMER_DURATION;
    }
  }

  if (is_death && action == GLFW_RELEASE && key == GLFW_KEY_L) {
    restart_game();
    load_game_from_file();
    return;
  }
  // Resetting game
  if (action == GLFW_RELEASE && key == GLFW_KEY_R && !cannot_reset_state) {
    // int w, h;
    // glfwGetWindowSize(window, &w, &h);
    restart_game();
    tutorialRoomDialogue(renderer);
    return;
  }

  // Toggling game pause
  if (action == GLFW_RELEASE && key == GLFW_KEY_P &&
      !registry.deathTimers.has(player) && !cannot_pause_state) {
    is_paused = !is_paused;
    depleteOxygen(player);
    if (is_paused) {
      overlay = createOverlay(renderer);
    } else {
      clearSaveStatus();
    }
  }

  if (action == GLFW_RELEASE && key == GLFW_KEY_S &&
      !registry.deathTimers.has(player) && is_paused && !cannot_pause_state) {
    depleteOxygen(player);
    displaySaveStatus(renderer, save_game_to_file());
  }

  if (action == GLFW_RELEASE && key == GLFW_KEY_L &&
      !registry.deathTimers.has(player) && is_paused && !cannot_pause_state) {
    is_death  = false;
    is_end    = false;
    is_paused = false;
    clearSaveStatus();
    load_game_from_file();
  }

  if (!is_frozen_state) {
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
    handleWeaponSwapping(renderer, key);
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
  bool is_frozen_state = is_intro || is_start || is_paused ||
                         is_krab_cutscene || is_sharkman_cutscene || is_death ||
                         is_end || room_transitioning;
  if (!is_frozen_state && !registry.stunned.has(player)) {
    player_mouse(renderer, button, action, mods, harpoon, harpoon_gun);
  }
}

void WorldSystem::on_mouse_scroll(double xOffset, double yOffset) {
  bool is_frozen_state = is_intro || is_start || is_paused ||
                         is_krab_cutscene || is_sharkman_cutscene || is_death ||
                         is_end || room_transitioning;
  if (!is_frozen_state) {
    player_scroll(xOffset, yOffset);
  }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
  mouse_pos = mouse_position;
}
