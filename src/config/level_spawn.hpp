#pragma once

#include "boss_factories.hpp"
#include "consumable_factories.hpp"
#include "enemy_factories.hpp"
#include "map_factories.hpp"
#include "level_util.hpp"

// SpawnFunctionWrapper(<spawn function>, <probability to spawn>, <pack size>)

const std::vector<SpawnFunctionWrapper> EMPTY = {};

const std::vector<SpawnFunctionWrapper>
  LVL_1 = {
    // Packs
    SpawnFunctionWrapper(createFishPos, 1.0f, 10),

    // Oxygen Canisters
    SpawnFunctionWrapper(createOxygenCanisterPos, 0.8f, 0),

    // Crates
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 0.5f, 0),
    SpawnFunctionWrapper(createCratePos, 0.5f, 0),

    // Geysers
    SpawnFunctionWrapper(createGeyserPos, 1.0f, 0),

    // Krabs
    SpawnFunctionWrapper(createKrabPos, 1.0f, 0),
    SpawnFunctionWrapper(createKrabPos, 0.5f, 0),

    // Jellies
    SpawnFunctionWrapper(createJellyPos, 1.0f, 0),
    SpawnFunctionWrapper(createJellyPos, 1.0f, 0),
    SpawnFunctionWrapper(createJellyPos, 0.5f, 0),

    // Sharks
    SpawnFunctionWrapper(createSharkPos, 1.0f, 0),
    SpawnFunctionWrapper(createSharkPos, 0.5f, 0),

    // Ambient
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createJunkPos, 1.0f, 0),
    SpawnFunctionWrapper(createJunkPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createCoralPos, 1.0f, 0),
    SpawnFunctionWrapper(createCoralPos, 1.0f, 0),
    SpawnFunctionWrapper(createCoralPos, 1.0f, 0),
  };

const std::vector<SpawnFunctionWrapper>
  LVL_2 = {
    // Sharks
    SpawnFunctionWrapper(createSharkPos, 1.0f, 5),
    
    // Oxygen Canisters
    SpawnFunctionWrapper(createOxygenCanisterPos, 1.0f, 0),

    // Crates
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),

    // Geysers
    SpawnFunctionWrapper(createGeyserPos, 0.33f, 0),
    
    // Seahorse
    SpawnFunctionWrapper(createSeahorsePos, 1.0f, 0),
    SpawnFunctionWrapper(createSeahorsePos, 0.25f, 0),

    // Siren
    SpawnFunctionWrapper(createSirenPos, 1.0f, 0),
    SpawnFunctionWrapper(createSirenPos, 0.5f, 0),

    // Fish
    SpawnFunctionWrapper(createFishPos, 1.0f, 0),

    // Ambient
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createBonesPos, 1.0f, 0),
    SpawnFunctionWrapper(createJunkPos, 1.0f, 0),
    SpawnFunctionWrapper(createJunkPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createBonesPos, 1.0f, 0),
    SpawnFunctionWrapper(createCoralPos, 1.0f, 0),
    SpawnFunctionWrapper(createCoralPos, 1.0f, 0),
  };

const std::vector<SpawnFunctionWrapper>
  LVL_3 = {
    // Oxygen Canisters
    SpawnFunctionWrapper(createOxygenCanisterPos, 1.0f, 0),

    // Crates
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),
    SpawnFunctionWrapper(createCratePos, 1.0f, 0),

    // Seahorses
    SpawnFunctionWrapper(createSeahorsePos, 1.0f, 0),

    // Sharks
    SpawnFunctionWrapper(createSharkPos, 1.0f, 0),
    SpawnFunctionWrapper(createSharkPos, 1.0f, 0),

    // Urchins
    SpawnFunctionWrapper(createUrchinPos, 1.0f, 0),

    // Lobsters
    SpawnFunctionWrapper(createLobsterPos, 1.0f, 0),
    SpawnFunctionWrapper(createLobsterPos, 0.25f, 0),

    // Siren
    SpawnFunctionWrapper(createSirenPos, 1.0f, 0),

    // Succ
    SpawnFunctionWrapper(createTurtlePos, 1.0f, 0),
    SpawnFunctionWrapper(createTurtlePos, 1.0f, 0),

    // Ambient
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createKelpPos, 1.0f, 0),
    SpawnFunctionWrapper(createBonesPos, 1.0f, 0),
    SpawnFunctionWrapper(createBonesPos, 1.0f, 0),
    SpawnFunctionWrapper(createBonesPos, 1.0f, 0),
    SpawnFunctionWrapper(createJunkPos, 1.0f, 0),
    SpawnFunctionWrapper(createJunkPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createShellPos, 1.0f, 0),
    SpawnFunctionWrapper(createBonesPos, 1.0f, 0),
    SpawnFunctionWrapper(createCoralPos, 1.0f, 0),
  };

const std::vector<SpawnFunctionWrapper> FINAL_BOSS = {
  SpawnFunctionWrapper(createCthulhuPos, 1.0f, 0),
  SpawnFunctionWrapper(createCthulhuRocksPos, 1.0f, 0),
};

const std::vector<SpawnFunctionWrapper> TUTORIAL_JELLYFISH_MINIBOSS = { 
    SpawnFunctionWrapper(createTutorial, 1.0f, 0)
};

const std::vector<SpawnFunctionWrapper>
    CRAB_MINIBOSS = {
        // Oxygen Canisters
        SpawnFunctionWrapper(createOxygenCanisterPos, 1.0f, 0),
        SpawnFunctionWrapper(createOxygenCanisterPos, 1.0f, 0),
        SpawnFunctionWrapper(createOxygenCanisterPos, 1.0f, 0),
        SpawnFunctionWrapper(createOxygenCanisterPos, 1.0f, 0),

        // Krabs
        SpawnFunctionWrapper(createKrabPos, 1.0f, 0),
        SpawnFunctionWrapper(createKrabPos, 1.0f, 0),
        SpawnFunctionWrapper(createKrabPos, 1.0f, 0),
        SpawnFunctionWrapper(createKrabPos, 1.0f, 0),
        SpawnFunctionWrapper(createKrabPos, 1.0f, 0),

        // Krab Boss
        SpawnFunctionWrapper(createCrabBossPos, 1.0f, 0),
    };

const std::vector<SpawnFunctionWrapper>
  SHARKMAN_MINIBOSS = {
      // Crates
      SpawnFunctionWrapper(createSharkmanCratesPos, 1.0f, 0),

      // Sharks
      SpawnFunctionWrapper(createSharkPos, 1.0f, 0),
      SpawnFunctionWrapper(createSharkPos, 1.0f, 0),
      SpawnFunctionWrapper(createSharkPos, 1.0f, 0),
      SpawnFunctionWrapper(createSharkPos, 1.0f, 0),
      SpawnFunctionWrapper(createSharkPos, 1.0f, 0),

      // Sharkman
      SpawnFunctionWrapper(createInitSharkmanPos, 1.0f, 0),
      SpawnFunctionWrapper(createCrabBossPos, 0.001f, 0),
  };

  

const SpawnFunctionWrapper RED_KEY_SPAWN = {
    SpawnFunctionWrapper(createRedKeyPos, 1.0f, 0)
};

const SpawnFunctionWrapper BLUE_KEY_SPAWN = {
    SpawnFunctionWrapper(createBlueKeyPos, 1.0f, 0)
};

const SpawnFunctionWrapper YELLOW_KEY_SPAWN = {
    SpawnFunctionWrapper(createYellowKeyPos, 1.0f, 0)
};

const SpawnFunctionWrapper ROCK_SPAWN = {
    SpawnFunctionWrapper(createRockPos, 1.0f, 0)
};

const SpawnFunctionWrapper PRESSURE_PLATE_SPAWN = {
    SpawnFunctionWrapper(createPressurePlatePos, 1.0f, 0)
};