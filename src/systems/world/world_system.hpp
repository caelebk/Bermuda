#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#include "render_system.hpp"
#include "level.hpp"

// Loop Duration 
#define LOOP_DURATION 500.f

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
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

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_click(int button, int action, int mods);
	void on_mouse_move(vec2 pos);

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	LevelBuilder level_builder;
	float current_speed;
	float next_oxygen_deplete;
	Entity player;
	Entity player_weapon;
	Entity player_projectile;
	Entity oxygen_tank;
	vec2 mouse_pos;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
