// Header
#include "world_system.hpp"
#include "common.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

// stlib
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <sstream>

#include "physics_system.hpp"

// Movement speed (To be balanced later)
#define SPEED_INC 2.0f

// Game configuration

// create the underwater world
WorldSystem::WorldSystem()
    : points(0) {
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
  window = glfwCreateWindow(window_width_px, window_height_px,
                            "Salmon Game Assignment", nullptr, nullptr);
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
  glfwSetKeyCallback(window, key_redirect);
  glfwSetCursorPosCallback(window, cursor_pos_redirect);

  //////////////////////////////////////
  // Loading music and sounds with SDL
  //////////////////////////////////////
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Failed to initialize SDL Audio");
    return nullptr;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
    fprintf(stderr, "Failed to open audio device");
    return nullptr;
  }

  background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
  salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
  salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

  if (background_music == nullptr || salmon_dead_sound == nullptr ||
      salmon_eat_sound == nullptr) {
    fprintf(stderr,
            "Failed to load sounds\n %s\n %s\n %s\n make sure the data "
            "directory is present",
            audio_path("music.wav").c_str(),
            audio_path("death_sound.wav").c_str(),
            audio_path("eat_sound.wav").c_str());
    return nullptr;
  }

  return window;
}

void WorldSystem::init(RenderSystem *renderer_arg) {
  this->renderer = renderer_arg;
  // Playing background music indefinitely
  Mix_PlayMusic(background_music, -1);
  fprintf(stderr, "Loaded music\n");

  // Set all states to default
  restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
  // Updating window title with points
  std::stringstream title_ss;
  title_ss << "Points: " << points;
  glfwSetWindowTitle(window, title_ss.str().c_str());

  // Remove debug info from the last step
  while (registry.debugComponents.entities.size() > 0)
    registry.remove_all_components_of(registry.debugComponents.entities.back());

  // Processing the salmon state
  assert(registry.screenStates.components.size() <= 1);
  ScreenState &screen = registry.screenStates.components[0];

  float min_counter_ms = 3000.f;
  for (Entity entity : registry.deathTimers.entities) {
    // progress timer
    DeathTimer &counter = registry.deathTimers.get(entity);
    counter.counter_ms -= elapsed_ms_since_last_update;
    if (counter.counter_ms < min_counter_ms) {
      min_counter_ms = counter.counter_ms;
    }

    // restart the game once the death timer expired
    if (counter.counter_ms < 0) {
      registry.deathTimers.remove(entity);
      screen.darken_screen_factor = 0;
      restart_game();
      return true;
    }
  }

  for (Entity entity : registry.motions.entities) {
      Motion& motion = registry.motions.get(entity);
      motion.position += motion.velocity;
  }

  // reduce window brightness if the salmon is dying
  screen.darken_screen_factor = 1 - min_counter_ms / 3000;
  return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
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

  player = createPlayer(renderer, { window_width_px / 2, window_height_px - 200 });
  registry.colors.insert(player, { 1, 0.8f, 0.8f });
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
  // Loop over all collisions detected by the physics system
  auto &collisionsRegistry = registry.collisions;
  for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
    // The entity and its collider
    Entity entity = collisionsRegistry.entities[i];
    Entity entity_other = collisionsRegistry.components[i].other;

    // for now, we are only interested in collisions that involve the salmon
    if (registry.players.has(entity)) {
      // Player& player = registry.players.get(entity);

      // Checking Player - Deadly collisions
      if (registry.deadlys.has(entity_other)) {
        // initiate death unless already dying
        if (!registry.deathTimers.has(entity)) {
          // Scream, reset timer, and make the salmon sink
          registry.deathTimers.emplace(entity);
          Mix_PlayChannel(-1, salmon_dead_sound, 0);
        }
      }
      // Checking Player - Eatable collisions
      else if (registry.consumables.has(entity_other)) {
        if (!registry.deathTimers.has(entity)) {
          // chew, count points, and set the LightUp timer
          registry.remove_all_components_of(entity_other);
          Mix_PlayChannel(-1, salmon_eat_sound, 0);
          ++points;
        }
      }
    }
  }

  // Remove all collisions from this simulation step
  registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
  return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    // Player movement attributes
    Motion& motion = registry.motions.get(player);

  // Resetting game
  if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    restart_game();
  }

  // Debugging
  if (key == GLFW_KEY_G) {
    if (action == GLFW_RELEASE)
      debugging.in_debug_mode = false;
    else
      debugging.in_debug_mode = true;
  }

  // WASD Movement Keys
  if (key == GLFW_KEY_W) {
      if (action == GLFW_RELEASE)
          motion.velocity[1] = 0;
      else
          motion.velocity[1] = -SPEED_INC;
  }
  if (key == GLFW_KEY_S) {
      if (action == GLFW_RELEASE)
          motion.velocity[1] = 0;
      else
          motion.velocity[1] = SPEED_INC;
  }
  if (key == GLFW_KEY_A) {
      if (action == GLFW_RELEASE)
          motion.velocity[0] = 0;
      else
          motion.velocity[0] = -SPEED_INC;
  }
  if (key == GLFW_KEY_D) {
      if (action == GLFW_RELEASE)
          motion.velocity[0] = 0;
      else
          motion.velocity[0] = SPEED_INC;
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
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
  (vec2) mouse_position; // dummy to avoid compiler warning
}
