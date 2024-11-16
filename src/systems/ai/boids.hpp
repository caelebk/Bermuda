#include "ai.hpp"
#include "common.hpp"
#include "physics.hpp"

#define MIN_DIST 100.f
#define SEPERATION_WEIGHT 1.f
#define COHESION_WEIGHT 0.3f
#define ALIGNMENT_WEIGHT 0.3f

extern Entity player;
void do_boids(float elapsed_ms);
