#include "random.hpp"

#include <ctime>

#include "assert.h"

/**
 * @brief Generates a random float between min and max
 *
 * @param min
 * @param max
 * @return
 */
float randomFloat(float min, float max) {
  std::random_device                    rd;         // seed generator
  std::mt19937                          gen(rd());  // mersenne twister rng
  std::uniform_real_distribution<float> distrib(min, max);

  return distrib(gen);
}

/**
 * @brief generates a random int between min and max
 *
 * @param min
 * @param max
 * @return
 */
int getRandInt(int min, int max) {
  return min + std::rand() % (max - min + 1);
}

/**
 * @brief returns true with a chance % of the time, otherwise false
 *
 * @param chance float between 0-1
 * @return
 */
bool randomSuccess(float chance) {
  assert(chance >= 0.0 && chance <= 1.0);
  static std::default_random_engine rng(
      static_cast<unsigned>(std::time(nullptr)));  // Seed with current time
  // std::default_random_engine            rng;
  std::uniform_real_distribution<float> uniform_dist;  // number between 0..1
  return (chance > uniform_dist(rng));
}
