#pragma once
#include "enemy_factories.hpp"

// currently used as "always spawn"
#define LVL_1_FIXED                                                      \
  {                                                                      \
      [this]() {                                                         \
        createJellyPos(renderer,                                         \
                       {window_width_px - 550, window_height_px - 250}); \
      },                                                                 \
      [this]() {                                                         \
        createJellyPos(renderer,                                         \
                       {window_width_px - 650, window_height_px - 250}); \
      },                                                                 \
  }

// currently used as "sometimes spawn"
#define LVL_1_RAND                                                       \
  {                                                                      \
      [this]() {                                                         \
        createJellyPos(renderer,                                         \
                       {window_width_px - 650, window_height_px - 350}); \
      },                                                                 \
      [this]() {                                                         \
        createJellyPos(renderer,                                         \
                       {window_width_px - 650, window_height_px - 450}); \
      },                                                                 \
      [this]() {                                                         \
        createJellyPos(renderer,                                         \
                       {window_width_px - 650, window_height_px - 300}); \
      },                                                                 \
      [this]() {                                                         \
        createJellyPos(renderer,                                         \
                       {window_width_px - 650, window_height_px - 200}); \
      },                                                                 \
  }

#define LVL_1_RAND_POS                              \
  {createCratePos, createCratePos, createCratePos,  \
   createCratePos, createCratePos, createGeyserPos, \
   createSharkPos, createKrabPos,  createFishPos}
// createJellyPos,          createJellyPos,  createJellyPos,          \
      // createJellyPos,          createJellyPos,  createOxygenCanisterPos, \
      // createOxygenCanisterPos, createGeyserPos, createGeyserPos,         \
      // createCratePos,          createCratePos,  createCratePos,          \
      // createCratePos,          createCratePos,  createCratePos,          \
      // createFishPos,           createFishPos,   createFishPos,           \
      // createFishPos,           createFishPos,   createFishPos,           \
      //
