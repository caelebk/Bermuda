// Header
#include "world_system.hpp"
#include "common.hpp"
#include "player_factories.hpp"
#include "enemy_factories.hpp"
#include "tiny_ecs_registry.hpp"
#include "oxygen_system.hpp"
#include "physics_system.hpp"
#include "audio_system.hpp"

// stlib
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <sstream>

// Movement speed (To be balanced later)
#define SPEED_INC 2.0f

// Game configuration

// create the underwater world
WorldSystem::WorldSystem() : points(0), next_oxygen_deplete(PLAYER_OXYGEN_DEPLETE_TIME_MS) {
  // Seeding rng with random device
  rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{

  // Destroy all created components
  registry.clear_all_components();

  // Close the window
  glfwDestroyWindow(window);
}

// Debugging
namespace
{
  void glfw_err_cb(int error, const char *desc)
  {
    fprintf(stderr, "%d: %s", error, desc);
  }
} // namespace

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the
// renderer
GLFWwindow *WorldSystem::create_window()
{
  ///////////////////////////////////////
  // Initialize GLFW
  glfwSetErrorCallback(glfw_err_cb);
  if (!glfwInit())
  {
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
  window = glfwCreateWindow(window_width_px, window_height_px,
                            "Bermuda", nullptr, nullptr);
  if (window == nullptr)
  {
    fprintf(stderr, "Failed to glfwCreateWindow");
    return nullptr;
  }

  // Setting callbacks to member functions (that's why the redirect is needed)
  // Input is handled using GLFW, for more info see
  // http://www.glfw.org/docs/latest/input_guide.html
  glfwSetWindowUserPointer(window, this);
  auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
  {
    ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
  };
  auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
  {
    ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
  };
  auto mouse_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) {
    ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2);
  };
  glfwSetKeyCallback(window, key_redirect);
  glfwSetCursorPosCallback(window, cursor_pos_redirect);
  glfwSetMouseButtonCallback(window, mouse_redirect);

  return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
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
  // Updating window title with points
  std::stringstream title_ss;
  title_ss << "Points: " << points;
  glfwSetWindowTitle(window, title_ss.str().c_str());

  // Remove debug info from the last step
  while (registry.debugComponents.entities.size() > 0)
    registry.remove_all_components_of(registry.debugComponents.entities.back());

  // Processing the player state
  assert(registry.screenStates.components.size() <= 1);
  ScreenState &screen = registry.screenStates.components[0];

  // Deplete oxygen when it is time...
  next_oxygen_deplete -= elapsed_ms_since_last_update;
  if (next_oxygen_deplete < 0) {
    next_oxygen_deplete = PLAYER_OXYGEN_DEPLETE_TIME_MS;
    depleteOxygen(player);
  }

  // Update gun and harpoon angle
  updateWepProjPos(mouse_pos, player, player_weapon, player_projectile);
  

  float min_counter_ms = 3000.f;
  for (Entity entity : registry.deathTimers.entities)
  {
    // progress timer
    DeathTimer &counter = registry.deathTimers.get(entity);
    counter.counter_ms -= elapsed_ms_since_last_update;
    if (counter.counter_ms < min_counter_ms)
    {
      min_counter_ms = counter.counter_ms;
    }

    // restart the game once the death timer expired
    if (counter.counter_ms < 0)
    {
      registry.deathTimers.remove(entity);
      screen.darken_screen_factor = 0;
      restart_game();
      return true;
    }
  }

  for (Entity entity : registry.motions.entities)
  {
    Motion &motion = registry.motions.get(entity);
    Position &position = registry.positions.get(entity);
    position.position += motion.velocity;
  }

  // reduce window brightness if the player is dying
  screen.darken_screen_factor = 1 - min_counter_ms / 3000;
  return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
  // Debugging for memory/component leaks
  registry.list_all_components();
  printf("Restarting\n");

  // Reset the game speed
  current_speed = 1.f;

  // Remove all entities that we created
  // All that have a motion, we could also iterate over all fish, eels, ... but
  // that would be more cumbersome
  while (registry.motions.entities.size() > 0)
    registry.remove_all_components_of(registry.motions.entities.back());

  // Debugging for memory/component leaks
  registry.list_all_components();

  level_builder.room(ROOM_ONE).activate_room(); // TODO: change based on which room entered

  player = createPlayer(renderer, {window_width_px / 2 - 180, window_height_px - 100}, HARPOON_PROJECTILE); // TODO: get player spawn position
  player_weapon = getPlayerWeapon(player);
  player_projectile = getPlayerProjectile(player);
  oxygen_tank = createOxygenTank(renderer, {20, window_height_px - 20}); // TODO: figure out oxygen tank position

  Entity jelly = createJellyPos(renderer, {window_width_px - 200, window_height_px / 2 - 150}); // TODO: REMOVE once enemy spawning fully implemented
  createJellyHealthBar(renderer, jelly);                                                        // TODO: REMOVE once enemy spawning fully implemented
  Entity fish = createFishPos(renderer, {window_width_px / 2 + 190, window_height_px - 300});   // TODO: REMOVE once enemy spawning fully implemented
  createFishHealthBar(renderer, fish);                                                          // TODO: REMOVE once enemy spawning fully implemented
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
  auto &collisionsRegistry = registry.collisions;
  for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
    Entity entity = collisionsRegistry.entities[i];
    Entity entity_other = collisionsRegistry.components[i].other;

    if (registry.players.has(entity)) {
      // Player& player = registry.players.get(entity);

      // Checking Player - Deadly collisions
      if (registry.deadlys.has(entity_other)) {
        if (!registry.deathTimers.has(entity)) {
          registry.sounds.insert(entity, Sound(death_sound));
          registry.deathTimers.emplace(entity);
        }
      }
      // Checking Player - Eatable collisions
      else if (registry.consumables.has(entity_other))
      {
        if (!registry.deathTimers.has(entity))
        {
          // chew, count points, and set the LightUp timer
          registry.remove_all_components_of(entity_other);
          registry.sounds.insert(entity, Sound(eat_sound));
          ++points;
        }
      }
    }
  }

  registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
  return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
  // Player movement attributes
  Motion &player_motion = registry.motions.get(player);

  // Resetting game
  if (action == GLFW_RELEASE && key == GLFW_KEY_R)
  {
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    restart_game();
  }

  // Debugging
  if (key == GLFW_KEY_G)
  {
    if (action == GLFW_RELEASE)
      debugging.in_debug_mode = false;
    else
      debugging.in_debug_mode = true;
  }

  //Music swap
  if (key == GLFW_KEY_M) {
    registry.musics.insert(Entity(), Music(background_music2));
  }
  if (key == GLFW_KEY_N) {
    registry.musics.insert(Entity(), Music(background_music));
  }

  // TODO: REMOVE temporary key input to switch between rooms (facilitate collision testing)
  if (key == GLFW_KEY_1)
  {
    if (action == GLFW_PRESS)
      level_builder.room(ROOM_ONE).activate_room();
  }

  // TODO: REMOVE temporary key input to switch between rooms (facilitate collision testing)
  if (key == GLFW_KEY_2)
  {
    if (action == GLFW_PRESS)
      level_builder.room(ROOM_TWO).activate_room();
  }

  // WASD Movement Keys
  if (key == GLFW_KEY_W) {
      if (action == GLFW_RELEASE) {
        player_motion.velocity.y = 0;
      }
      else {
        player_motion.velocity.y = -SPEED_INC;
      }
  }
  if (key == GLFW_KEY_S) {
      if (action == GLFW_RELEASE) {
        player_motion.velocity.y = 0;
      }
      else {
        player_motion.velocity.y = SPEED_INC;
      }
  }
  if (key == GLFW_KEY_A) {
      if (action == GLFW_RELEASE) {
        player_motion.velocity.x = 0;
      }
      else {
        player_motion.velocity.x = -SPEED_INC;
      }
  }
  if (key == GLFW_KEY_D) {
      if (action == GLFW_RELEASE) {
        player_motion.velocity.x = 0;
      }
      else {
        player_motion.velocity.x = SPEED_INC;
      }
  }

  // Control the current speed with `<` `>`
  if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
      key == GLFW_KEY_COMMA)
  {
    current_speed -= 0.1f;
    printf("Current speed = %f\n", current_speed);
  }
  if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
      key == GLFW_KEY_PERIOD)
  {
    current_speed += 0.1f;
    printf("Current speed = %f\n", current_speed);
  }
  current_speed = fmax(0.f, current_speed);
}

// Mouse click callback
void WorldSystem::on_mouse_click(int button, int action, int mods) {
    // Shooting the projectile
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && registry.playerProjectiles.get(player_projectile).is_loaded) {
            setFiredProjVelo(player_projectile);
            consumeOxygen(player, player_weapon);
        }
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    mouse_pos = mouse_position;
}
