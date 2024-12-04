#pragma once

#include "collision_system.hpp"
#include "common.hpp"
#include "components.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

// Create the tutorial
Entity createTutorial(RenderSystem* renderer, vec2 position,
                      bool checkCollisions);

//////////////////////////////////////////////////////////////
// KRAB BOSS
//////////////////////////////////////////////////////////////
#define KRAB_BOSS_MS 50.0
#define KRAB_BOSS_DAMAGE -300.0
#define KRAB_BOSS_ATK_SPD 3000.0
#define KRAB_BOSS_SCALE vec2(0.1)
#define KRAB_BOSS_BOUNDING_BOX vec2(3433.f, 2494.f)
#define KRAB_BOSS_HEALTH 750.0
#define KRAB_BOSS_MIN_DIR_CD 1000
#define KRAB_BOSS_MAX_DIR_CD 8000
#define KRAB_BOSS_TRACKING_CD 1000.f
#define KRAB_BOSS_ENEMY_LIMIT 16  // 15 + itself
#define KRAB_BOSS_SPOT_RADIUS 400.f
#define KRAB_BOSS_LEASH_RADIUS 600.f
#define KRAB_BOSS_TRACKING_ACCELERATION 10.f
#define KRAB_BOSS_HEALTH_SCALE vec2(1.4f)
#define KRAB_BOSS_HEALTH_BAR_SCALE vec2(1.5f)
#define KRAB_BOSS_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define KRAB_BOSS_AI_CD 5000

Entity createCrabBossPos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions = true);
Entity createInitCrabBossPos(RenderSystem* renderer, vec2 position,
                             bool checkCollisions = true);
Entity respawnCrabBoss(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// SHARKMAN
//////////////////////////////////////////////////////////////
#define SHARKMAN_MS 125.0
#define SHARKMAN_MS_INC 25.0
#define SHARKMAN_DAMAGE -200.0
#define SHARKMAN_ATK_SPD 2000.0
#define SHARKMAN_SCALE vec2(0.35)
#define SHARKMAN_BOUNDING_BOX vec2(301.f, 246.f)
#define SHARKMAN_HEALTH 12000.0
#define SHARKMAN_SELF_DMG -2000.0
#define SHARKMAN_TRACKING_CD 9999999.f
#define SHARKMAN_SPOT_RADIUS 500.f
#define SHARKMAN_HEALTH_SCALE vec2(1.4f)
#define SHARKMAN_HEALTH_BAR_SCALE vec2(1.5f)
#define SHARKMAN_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define SHARKMAN_AI_CD 8000

Entity createSharkmanPos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions = true);
Entity createInitSharkmanPos(RenderSystem* renderer, vec2 position,
                             bool checkCollisions = true);
Entity respawnSharkman(RenderSystem* renderer, EntityState es);
void   addSharkmanWander();
void   addSharkmanTarget();

//////////////////////////////////////////////////////////////
// CTHULHU
//////////////////////////////////////////////////////////////
#define CTHULHU_DAMAGE -150.0
#define CTHULHU_ATK_SPD 2000.0
#define CTHULHU_SCALE vec2(0.75)
#define CTHULHU_BOUNDING_BOX vec2(330.f, 310.f)
#define CTHULHU_HEALTH 1500.0
#define CTHULHU_HEALTH_SCALE vec2(1.4f)
#define CTHULHU_HEALTH_BAR_SCALE vec2(1.41f)
#define CTHULHU_HEALTH_BOUNDING_BOX vec2(200.f, 10.f)
#define CTHULHU_REGEN_AMT 150.0
#define CTHULHU_REGEN_RATE 500.0
#define CTHULHU_AI_CD 5000

#define CTHULHU_SPAWN_TENTACLE_CD 1500.0
#define CTHULHU_RAGE_SPAWN_TENTACLE_CD 750.0
#define CTHULHU_ENEMY_LIMIT 9  // 8 + itself

#define CTHULHU_FIREBALL_FIRERATE 1200.0
#define CTHULHU_RAGE_FIREBALL_FIRERATE 600.0

#define CTHULHU_CANISTER_MS 100.0
#define CTHULHU_CANISTER_TIMER 2500.f
#define CTHULHU_CANISTER_FIRERATE 1500.0
#define CTHULHU_RAGE_CANISTER_MS 200.0
#define CTHULHU_RAGE_CANISTER_TIMER 1475.f
#define CTHULHU_RAGE_CANISTER_FIRERATE 1000.0

#define CTHULHU_WAVE_FIRERATE 2000.0
#define CTHULHU_FRENZY_FIRERATE 500.0

Entity createCthulhuPos(RenderSystem* renderer, vec2 position,
                        bool checkCollisions = true);
Entity respawnCthulhu(RenderSystem* renderer, EntityState es);
Entity respawnCthulhuPhase2(RenderSystem* renderer, EntityState es);
Entity respawnCthulhuTrans(RenderSystem* renderer, EntityState es);
Entity shootCanister(RenderSystem* renderer, vec2 position, vec2 direction,
                     bool is_angry);
void   addCthulhuRageAI();

//////////////////////////////////////////////////////////////
// TENTACLE
//////////////////////////////////////////////////////////////
#define TENTACLE_MS 35.0
#define TENTACLE_DAMAGE -50.0
#define TENTACLE_ATK_SPD 1500.0
#define TENTACLE_STUN_MS 1250.0
#define TENTACLE_SCALE vec2(0.6)
#define TENTACLE_BOUNDING_BOX vec2(93.f, 233.f)
#define TENTACLE_MIN_DIR_CD 5000
#define TENTACLE_MAX_DIR_CD 10000
#define TENTACLE_TRACKING_CD 500.f
#define TENTACLE_SPOT_RADIUS 800.f
#define TENTACLE_LEASH_RADIUS 900.f
#define TENTACLE_HEALTH 50.0
#define TENTACLE_HEALTH_SCALE vec2(1.4f)
#define TENTACLE_HEALTH_BAR_SCALE vec2(1.5f)
#define TENTACLE_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createTentaclePos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions = true);
Entity respawnTentacle(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// FIREBALL
//////////////////////////////////////////////////////////////
#define FIREBALL_MS 125.0
#define FIREBALL_SCALE vec2(0.7)
#define FIREBALL_BOUNDING_BOX vec2(160.f, 50.f)
#define FIREBALL_DAMAGE -150.0
#define FIREBALL_TIMER 8000.f

Entity shootFireball(RenderSystem* renderer, vec2 position, vec2 direction);

//////////////////////////////////////////////////////////////
// SHOCKWAVE
//////////////////////////////////////////////////////////////
#define SHOCKWAVE_GROW_RATE 500.0
#define SHOCKWAVE_BOUNDING_BOX vec2(799.f, 769.f)
#define SHOCKWAVE_DAMAGE -300.0
#define SHOCKWAVE_TIMER 1500.f

Entity shootShockwave(RenderSystem* renderer, vec2 position);

//////////////////////////////////////////////////////////////
// CTHULHU RAGE PROJECTILES
//////////////////////////////////////////////////////////////
#define CTHULHU_RAGE_PROJ_MS 90.0
#define CTHULHU_RAGE_PROJ_SCALE vec2(0.03)
#define CTHULHU_RAGE_PROJ_BOUNDING_BOX vec2(800.f, 800.f)
#define CTHULHU_RAGE_PROJ_DAMAGE -75.0
#define CTHULHU_RAGE_PROJ_TIMER 5000.f

Entity shootRageProjectile(RenderSystem* renderer, vec2 position, float angle);
