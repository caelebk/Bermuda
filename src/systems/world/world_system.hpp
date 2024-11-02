#pragma once

// internal
#include "common.hpp"

// stlib
#include <random>
#include <vector>

#include "level.hpp"
#include "render_system.hpp"

// Loop Duration
#define LOOP_DURATION 500.f

// Container for all our entities and game logic. Individual rendering / update
// is deferred to the relative update() methods
class WorldSystem {
  public:
  WorldSystem();

  // Creates a window
  GLFWwindow* create_window();

  // starts the game
  void init(RenderSystem* renderer);

  // Releases all associated resources
  ~WorldSystem();

  // Steps the game ahead by ms milliseconds
  bool step(float elapsed_ms);

  // Should the game be over ?
  bool is_over() const;

  private:
  // Input callback functions
  void on_key(int key, int, int action, int mod);
  void on_mouse_click(int button, int action, int mods);
  void on_mouse_move(vec2 pos);
  void check_bounds();

  // restart level
  void restart_game();

  // OpenGL window handle
  GLFWwindow* window;

  // Number of fish eaten by the salmon, displayed in the window title
  unsigned int points;

  // Game state
  RenderSystem* renderer;
  float         oxygen_timer;
  vec2          mouse_pos;

  // C++ random number generator
  std::default_random_engine            rng;
  std::uniform_real_distribution<float> uniform_dist;  // number between 0..1
};

extern bool        paused;
extern Entity      pause_menu;
extern Entity      player;
extern Entity      player_weapon;
extern Entity      player_projectile;
extern Entity      harpoon;
extern Entity      net;
extern Entity      concussive;
extern Entity      torpedo;
extern Entity      shrimp;
extern PROJECTILES wep_type;

extern Entity harpoon_gun;
extern Entity net_gun;
extern Entity concussive_gun;
extern Entity torpedo_gun;
extern Entity shrimp_gun;

extern LevelBuilder level_builder;