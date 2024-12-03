#include <cstdio>

#include "ai_system.hpp"
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "audio_system.hpp"
#include "collision_system.hpp"
#include "level_system.hpp"
#include "physics_system.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "saving_system.hpp"
#include "world_state.hpp"
#include "world_system.hpp"
// #include "game_start_system.hpp"

using Clock = std::chrono::high_resolution_clock;

bool   krab_boss_encountered = false;
bool   sharkman_encountered  = false;
bool   is_intro              = false;
bool   is_start              = false;
bool   is_paused             = false;
bool   is_krab_cutscene      = false;
bool   is_sharkman_cutscene  = false;
bool   is_cthulhu_cutscene   = false;
bool   is_death              = false;
bool   is_end                = false;
Entity overlay;
bool   room_transitioning = false;
Entity rt_entity;

Entity player;

// Consumable Entities
Entity      player_weapon;
Entity      player_projectile;
Entity      harpoon;
Entity      net;
Entity      concussive;
Entity      torpedo;
Entity      shrimp;
PROJECTILES wep_type;

Entity harpoon_gun;
Entity net_gun;
Entity concussive_gun;
Entity torpedo_gun;
Entity shrimp_gun;

// Entry point
int main(int argc, char* argv[]) {
  // Global systems
  WorldSystem     world;
  RenderSystem    renderer;
  AISystem        ai;
  PhysicsSystem   physics;
  AudioSystem     audios;
  CollisionSystem collisions;
  LevelSystem     level;

  // world seed
  unsigned int seed = 0;

  // Initializing window
  GLFWwindow* window = world.create_window();
  if (!window) {
    // Time to read the error message
    printf("Press any key to exit");
    getchar();
    return EXIT_FAILURE;
  }

  // optionally parse a seed input
  if (argc == 2) {
    // Convert argument to unsigned int
    char*         end;
    unsigned long value = std::strtoul(argv[1], &end, 10);

    // Validate the input
    if (!(*end != '\0' || value > std::numeric_limits<unsigned int>::max())) {
      seed = static_cast<unsigned int>(value);
    }
  }

  // Initialize the bare minimum to get the menu screen working
  renderer.init(window);
  audios.init();

  // Menu Screen
  is_start = true;
  registry.remove_all_components_of(overlay);
  overlay = createOverlay(&renderer);
  overlayState(TEXTURE_ASSET_ID::START_OVERLAY);
  while (!world.is_over() && !world.game_started) {
    glfwPollEvents();
    renderer.draw();
  }

  if (world.is_over()) {
    return EXIT_SUCCESS;
  }

  registry.remove_all_components_of(overlay);

  // we loaded from a save file, set the seed again
  if (world.load_from_save) {
    seed = get_seed_from_save_file();
  }

  // seed is 0 if we have a fresh start, or loading failed
  if (seed == 0) {
    setGlobalRandomSeed();
  } else {
    setGlobalSeed(seed);
  }

  // Generate a level.
  LevelBuilder level_builder = LevelBuilder();
  level_builder.generate_random_level();

  // initialize the main systems
  init_save_system(&level_builder, &level, &renderer);
  level.init(&renderer, &level_builder);
  collisions.init(&renderer, &level);
  world.init(&renderer, &level);
  ai.init(&renderer);

  // if we loaded from a save, load the save file
  if (seed != 0 && world.load_from_save) {
    load_game_from_file();
  }

  // variable timestep loop
  auto t = Clock::now();
  while (!world.is_over()) {
    // Processes system messages, if this wasn't present the window would become
    // unresponsive
    glfwPollEvents();
    // Calculating elapsed times in milliseconds from the previous iteration
    auto  now = Clock::now();
    float elapsed_ms =
        (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t))
            .count() /
        1000;
    t = now;

    world.step(elapsed_ms);
    bool is_frozen_state = is_intro || is_start || is_paused ||
                           is_krab_cutscene || is_sharkman_cutscene ||
                           is_cthulhu_cutscene || is_death || is_end ||
                           room_transitioning;
    if (!is_frozen_state) {
      // Note: WorldSystem::step runs simply to update FPS counter, but is
      // mostly disabled
      ai.step(elapsed_ms);
      physics.step(elapsed_ms);
      collisions.step(elapsed_ms);
    }
    audios.step(elapsed_ms);
    renderer.draw();
  }

  return EXIT_SUCCESS;
}
