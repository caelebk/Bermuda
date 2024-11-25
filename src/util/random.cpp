#include "random.hpp"

#include <cstdio>
#include <ctime>

#include "assert.h"

// I refuse to make this an object
// - MLK

// Global Mersenne Twister RNG
static std::mt19937 rng;
static unsigned int globalSeed = 0;

/**
 * @brief Sets the global random seed
 *
 * @param seed
 */
void setGlobalSeed(unsigned int seed) {
  printf("Setting seed to %d\n", seed);
  rng.seed(seed);
  globalSeed = seed;
}

/**
 * @brief Sets the global random seed
 *
 * @param seed 
 */
void setGlobalRandomSeed() {
  setGlobalSeed(static_cast<unsigned>(std::time(nullptr)));
}

/**
 * @brief Sets the global random seed
 *
 * @param seed
 */
unsigned int getGlobalRandomSeed() {
  return globalSeed;
}

/**
 * @brief Gets the global rng
 *
 * @return 
 */
std::mt19937 getGlobalRNG() {
  return rng;
}

/**
 * @brief Generates a random float between min and max
 *
 * @param min
 * @param max
 * @return
 */
float randomFloat(float min, float max) {
  std::uniform_real_distribution<float> distrib(min, max);
  return distrib(rng);
}

/**
 * @brief generates a random int between min and max
 *
 * @param min
 * @param max
 * @return
 */
int getRandInt(int min, int max) {
  std::uniform_int_distribution<int> distrib(min, max);
  return distrib(rng);
}

/**
 * @brief returns true with a chance % of the time, otherwise false
 *
 * @param chance float between 0-1
 * @return
 */
bool randomSuccess(float chance) {
  assert(chance >= 0.0 && chance <= 1.0);
  return (chance > randomFloat(0.f, 1.f));
}
