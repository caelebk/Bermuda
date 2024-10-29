
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "audio_system.hpp"
#include "collision_system.hpp"
#include "level_build_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

using Clock = std::chrono::high_resolution_clock;

bool paused;

// Entry point
int main() {
  // Global systems
  WorldSystem     world;
  RenderSystem    renderer;
  PhysicsSystem   physics;
  AudioSystem     audios;
  CollisionSystem collisions;

  // TODO: Change this when pause menu is implemented
  paused = false;

  // Initializing window
  GLFWwindow* window = world.create_window();
  if (!window) {
    // Time to read the error message
    printf("Press any key to exit");
    getchar();
    return EXIT_FAILURE;
  }

  // initialize the main systems
  renderer.init(window);
  world.init(&renderer);
  audios.init();

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
    physics.step(elapsed_ms);
    collisions.step(elapsed_ms);
    audios.step(elapsed_ms);
    renderer.draw();
  }

  return EXIT_SUCCESS;
}
