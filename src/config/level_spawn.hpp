#pragma once

#include "map_factories.hpp"
#include "boss_factories.hpp"
#include "enemy_factories.hpp"
#include "consumable_factories.hpp"

const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> EMPTY = {};

const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> LVL_1_RAND_POS =                                                       
  {createOxygenCanisterPos, createCratePos, createCratePos,  createCratePos, 
   createCratePos,          createCratePos, createGeyserPos, createSharkPos, 
   createKrabPos,           createFishPos,  createJellyPos};

// TODO:
// We don't have any new enemies yet, so just spawn a billion sharks for LVL 2.
const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> LVL_2_RAND_POS =                                                       
{createOxygenCanisterPos, createCratePos, createCratePos,  createCratePos, 
  createCratePos,          createCratePos, createGeyserPos, createFishPos, 
  createSharkPos,           createSharkPos,  createSharkPos};

// TODO:
// Ditto.
const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> LVL_3_RAND_POS =                                                       
  {createSharkPos, createSharkPos, createSharkPos,  createSharkPos, 
   createSharkPos,          createSharkPos, createSharkPos, createSharkPos, 
   createSharkPos,           createSharkPos,  createSharkPos};

const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> TUTORIAL_JELLYFISH_MINIBOSS =                                 
{                                                                        
    createJellyBossPos,                                                      
};

const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> CRAB_MINIBOSS = 
{                                                                              
    createCrabBossPos, createKrabPos, createKrabPos, createKrabPos, createKrabPos, createKrabPos, createKrabPos,
    createOxygenCanisterPos, createOxygenCanisterPos, createOxygenCanisterPos
};

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