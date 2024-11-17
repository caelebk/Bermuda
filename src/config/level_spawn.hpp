#pragma once

#include "boss_factories.hpp"
#include "consumable_factories.hpp"
#include "enemy_factories.hpp"
#include "map_factories.hpp"

const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> EMPTY = {};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    LVL_1_RAND_POS = {createOxygenCanisterPos, createCratePos, createCratePos,
                      createCratePos,          createCratePos, createCratePos,
                      createGeyserPos,         createSharkPos, createKrabPos,
                      createFishPos,           createJellyPos, createUrchinPos};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    LVL_1_PACKS = {createFishPos};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    LVL_2_RAND_POS = {createOxygenCanisterPos, createCratePos, createCratePos,
                      createCratePos,          createCratePos, createCratePos,
                      createGeyserPos,         createFishPos};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    LVL_2_PACKS = {createSharkPos};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    LVL_3_RAND_POS = {createSharkPos, createSharkPos, createSharkPos,
                      createUrchinPos, createFishPos, createCratePos,
                      createCratePos, createCratePos, createOxygenCanisterPos,
                      createLobsterPos};

const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> TUTORIAL_JELLYFISH_MINIBOSS =                                 
{                                                                        
    createTutorial,                                                      
};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    CRAB_MINIBOSS = {createCrabBossPos,       createKrabPos,
                     createKrabPos,           createKrabPos,
                     createKrabPos,           createKrabPos,
                     createKrabPos,           createOxygenCanisterPos,
                     createOxygenCanisterPos, createOxygenCanisterPos};

const std::vector<
    std::function<Entity(RenderSystem* r, vec2 p, bool b)>>
    SHARKMAN_MINIBOSS = {createInitSharkmanPos, createSharkmanCratesPos,
                         createSharkPos,        createSharkPos,
                         createSharkPos,        createSharkPos,
                         createSharkPos};

const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> RED_KEY_SPAWN = {
  createRedKeyPos,
};

const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> BLUE_KEY_SPAWN = {
  createBlueKeyPos,
};

const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>> YELLOW_KEY_SPAWN = {
  createYellowKeyPos,
};