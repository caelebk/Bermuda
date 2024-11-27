#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>

#include "world_state.hpp"
#include "level_system.hpp"
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
  void init(RenderSystem* renderer, LevelSystem* level);

  // Releases all associated resources
  ~WorldSystem();

  // Steps the game ahead by ms milliseconds
  bool step(float elapsed_ms);

  // Should the game be over ?
  bool is_over() const;

  bool          game_started = false;
  bool          load_from_save = false;
  private:
  // Input callback functions
  void on_key(int key, int, int action, int mod);
  void on_mouse_click(int button, int action, int mods);
  void on_mouse_move(vec2 pos);
  void check_bounds();
  void update_fps(float elapsed_ms_since_last_update);

  // restart level
  void restart_game();

  // OpenGL window handle
  GLFWwindow* window;

  // Number of fish eaten by the salmon, displayed in the window title
  unsigned int points;

  // Game state
  LevelSystem*  level;
  RenderSystem* renderer;
  bool          overlay_transitioning = false;
  float         overlay_timer = 0.f;
  float         oxygen_timer;
  vec2          mouse_pos;
};

extern bool        is_intro;
extern bool        is_start;
extern bool        is_paused;
extern bool        is_krab_cutscene;
extern bool        is_sharkman_cutscene;
extern bool        is_death;
extern bool        is_end;
extern Entity      overlay;
extern bool        room_transitioning;
extern Entity      rt_entity;
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
