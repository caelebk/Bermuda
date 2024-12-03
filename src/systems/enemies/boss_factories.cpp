#include "boss_factories.hpp"

#include <functional>
#include <vector>

#include "consumable_factories.hpp"
#include "enemy_factories.hpp"
#include "entity_type.hpp"
#include "map_factories.hpp"
#include "oxygen_system.hpp"

/////////////////////////////////////////////////////////////////
// Util and behaviours
/////////////////////////////////////////////////////////////////
/**
 * @brief Checks whether or not the spawn is valid or invalid based on spawn
 * collisons The entity should already have the position attached
 *
 * @param entity - enemy to check
 * @return true if valid, false otherwise
 */
static bool checkSpawnCollisions(Entity entity) {
  if (!registry.positions.has(entity)) {
    return false;
  }
  const Position& enemyPos = registry.positions.get(entity);

  // Tentacles can't spawn over other tentacles
  if (registry.deadlys.has(entity) &&
      registry.deadlys.get(entity).type == ENTITY_TYPE::TENTACLE) {
    for (Entity tentacle : registry.deadlys.entities) {
      if (!registry.positions.has(tentacle) ||
          registry.deadlys.get(tentacle).type != ENTITY_TYPE::TENTACLE) {
        continue;
      }
      const Position tentaclePos = registry.positions.get(tentacle);
      if (box_collides(enemyPos, tentaclePos)) {
        return false;
      }
    }
  }

  // Entities can't spawn in the player
  for (Entity player : registry.players.entities) {
    if (!registry.positions.has(player)) {
      continue;
    }
    const Position player_pos = registry.positions.get(player);
    if (box_collides(enemyPos, player_pos)) {
      return false;
    }
  }

  // Entities can't spawn in walls
  for (Entity wall : registry.activeWalls.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(enemyPos, wallPos)) {
      return false;
    }
  }

  // Entities can't spawn in doors
  for (Entity door : registry.activeDoors.entities) {
    if (!registry.positions.has(door)) {
      continue;
    }
    const Position doorPos = registry.positions.get(door);
    if (box_collides(enemyPos, doorPos)) {
      return false;
    }
  }
  return true;
}

static void addCrabBossWander() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b = registry.bosses.entities[0];

  auto& wander         = registry.wanders.emplace(b);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(KRAB_BOSS_MIN_DIR_CD, KRAB_BOSS_MAX_DIR_CD);

  printf("Crab boss is wandering!\n");
}

static void addCrabMelee() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b = registry.bosses.entities[0];

  auto& tracking        = registry.trackPlayer.emplace(b);
  tracking.tracking_cd  = KRAB_BOSS_TRACKING_CD;
  tracking.spot_radius  = KRAB_BOSS_SPOT_RADIUS;
  tracking.leash_radius = KRAB_BOSS_LEASH_RADIUS;
  tracking.acceleration = KRAB_BOSS_TRACKING_ACCELERATION * 4;
  printf("Crab boss is trying to melee!\n");

  addCrabBossWander();
}

static void addCrabRanged() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }

  if (registry.deadlys.entities.size() > KRAB_BOSS_ENEMY_LIMIT) {
    // If we're at limit melee instead
    addCrabMelee();
    return;
  }

  Entity& b = registry.bosses.entities[0];

  auto& tracking        = registry.trackPlayerRanged.emplace(b);
  tracking.tracking_cd  = KRAB_BOSS_TRACKING_CD;
  tracking.spot_radius  = KRAB_BOSS_SPOT_RADIUS * 3;
  tracking.min_distance = KRAB_BOSS_SPOT_RADIUS;
  tracking.leash_radius = KRAB_BOSS_LEASH_RADIUS * 2;
  tracking.acceleration = KRAB_BOSS_TRACKING_ACCELERATION;
  printf("Crab boss is trying to shoot player!\n");

  addCrabBossWander();
}

void addSharkmanWander() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b = registry.bosses.entities[0];

  auto& wander = registry.wanders.emplace(b);
  // *almost* never pick a new direction
  wander.active_dir_cd = SHARKMAN_TRACKING_CD;
  wander.change_dir_cd = SHARKMAN_TRACKING_CD;

  registry.bosses.get(b).is_angry = false;

  printf("Sharkman is cooking...\n");
}

void addSharkmanTarget() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b = registry.bosses.entities[0];

  auto& tracking = registry.trackPlayer.emplace(b);
  // *almost* never chase player, only target on "bounce"
  tracking.curr_cd      = SHARKMAN_TRACKING_CD;
  tracking.tracking_cd  = SHARKMAN_TRACKING_CD;
  tracking.spot_radius  = SHARKMAN_SPOT_RADIUS;
  tracking.leash_radius = SHARKMAN_SPOT_RADIUS;
  printf("WATCH OUT WATCH OUT WATCH O-\n");
}

static void addCthulhuTentacles() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b    = registry.bosses.entities[0];
  Boss&   boss = registry.bosses.get(b);

  // it'd be funny if it basically shoots tentacles
  auto& shooter      = registry.shooters.emplace(b);
  shooter.type       = RangedEnemies::CTHULHU_TENTACLE;
  shooter.cooldown   = boss.is_angry ? CTHULHU_RAGE_SPAWN_TENTACLE_CD
                                     : CTHULHU_SPAWN_TENTACLE_CD;
  shooter.default_cd = boss.is_angry ? CTHULHU_RAGE_SPAWN_TENTACLE_CD
                                     : CTHULHU_SPAWN_TENTACLE_CD;

  printf("beware the tentacles!\n");
}

static void addCthulhuFireballs() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b    = registry.bosses.entities[0];
  Boss&   boss = registry.bosses.get(b);

  auto& shooter      = registry.shooters.emplace(b);
  shooter.type       = RangedEnemies::CTHULHU_FIREBALL;
  shooter.cooldown   = boss.is_angry ? CTHULHU_RAGE_FIREBALL_FIRERATE
                                     : CTHULHU_FIREBALL_FIRERATE;
  shooter.default_cd = boss.is_angry ? CTHULHU_RAGE_FIREBALL_FIRERATE
                                     : CTHULHU_FIREBALL_FIRERATE;

  printf("FIREBALLFIREBALLFIREBALL\n");
}

static void addCthulhuCanisters() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b    = registry.bosses.entities[0];
  Boss&   boss = registry.bosses.get(b);

  auto& shooter      = registry.shooters.emplace(b);
  shooter.type       = RangedEnemies::CTHULHU_CANISTER;
  shooter.cooldown   = CTHULHU_CANISTER_FIRERATE;
  shooter.default_cd = CTHULHU_CANISTER_FIRERATE;

  printf("TAKE THIS!\n");
}

static void addCthulhuShockwaves() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b = registry.bosses.entities[0];

  auto& shooter      = registry.shooters.emplace(b);
  shooter.type       = RangedEnemies::CTHULHU_SHOCKWAVE;
  shooter.default_cd = CTHULHU_WAVE_FIRERATE;
  shooter.cooldown   = CTHULHU_WAVE_FIRERATE;

  // sound effect alert
  if (!registry.sounds.has(b)) {
    registry.sounds.insert(b, Sound(SOUND_ASSET_ID::CTHULHU_SHOCKWAVE, 3000));
  }

  printf("TIME TO HIDE\n");
}

static void addCthulhuRageProjectiles() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b = registry.bosses.entities[0];

  auto& shooter      = registry.shooters.emplace(b);
  shooter.type       = RangedEnemies::CTHULHU_RAGE_PROJ;
  shooter.default_cd = CTHULHU_FRENZY_FIRERATE;
  shooter.cooldown   = CTHULHU_FRENZY_FIRERATE;

  printf("FRENZY\n");
}

void addCthulhuRageAI() {
  if (registry.bosses.entities.size() != 1) {
    return;
  }
  Entity& b    = registry.bosses.entities[0];
  Boss&   boss = registry.bosses.get(b);
  // heal cthulhu to full, then change ai
  Oxygen& oxygen = registry.oxygen.get(b);
  if (oxygen.level < CTHULHU_HEALTH) {
    boss.in_transition = true;
    if (!boss.is_angry) {
      if (registry.renderRequests.has(b)) {
        registry.renderRequests.get(b).used_texture =
            TEXTURE_ASSET_ID::CTHULHU_RAGE;
      }
      if (!registry.sounds.has(b)) {
        registry.sounds.insert(b, Sound(SOUND_ASSET_ID::CTHULHU_ANGRY, 4000));
      }
      boss.is_angry = true;
    }
    modifyOxygenAmount(b, CTHULHU_REGEN_AMT);
  } else {
    oxygen.level = CTHULHU_HEALTH;
    boss.in_transition = false;
    boss.curr_cd = 0;  // reset cd
    boss.ai_cd   = CTHULHU_AI_CD;
    boss.ai      = std::vector<std::function<void()>>(
        {addCthulhuShockwaves, addCthulhuRageProjectiles, addCthulhuTentacles,
              addCthulhuFireballs, addCthulhuCanisters, addCthulhuCanisters});

    printf("RAGE STRONGER RAHHH\n");
  }
}

/////////////////////////////////////////////////////////////////
// Create Tutorial and Bosses
/////////////////////////////////////////////////////////////////
Entity createTutorial(RenderSystem* renderer, vec2 position,
                      bool checkCollisions) {
  vec2 canisterPos = {window_width_px - 332.f, window_height_px - 250.f};
  createOxygenCanisterPos(renderer, canisterPos, checkCollisions);

  vec2 geyserPos = {window_width_px - 220.f, window_height_px - 265.f};
  createGeyserPos(renderer, geyserPos, checkCollisions);

  vec2 cratePos = {386.f, window_height_px - 260.f};
  createCratePos(renderer, cratePos, checkCollisions);

  vec2 jellyPos = {window_width_px - 100.f, 100.f};

  // Also, make it drop a boss key, which unlocks its room.
  Entity entity = createJellyPos(renderer, jellyPos, checkCollisions);

  // If it spawned with a drop, get rid of it, otherwise it'll dupe the entity
  // in the ECS.
  if (registry.drops.has(entity)) {
    registry.drops.remove(entity);
  }

  // Replace it with the boss key drop.
  Drop& drop  = registry.drops.emplace(entity);
  drop.dropFn = unlockTutorial;
  return entity;
};

Entity createCrabBossPos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions) {
  // Reserve an entity
  auto  entity = Entity();
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = KRAB_BOSS_SCALE * KRAB_BOSS_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d = registry.deadlys.emplace(entity);
  d.type    = ENTITY_TYPE::KRAB_BOSS;

  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = KRAB_BOSS_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = KRAB_BOSS_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion = registry.motions.emplace(entity);
  // krab boss starts off moving opposite direction of player
  vec2 direction = pos.position - registry.positions.get(player).position;
  if (direction.x < 0) {
    pos.scale.x = abs(pos.scale.x) * -1;
  } else {
    pos.scale.x = abs(pos.scale.x);
  }
  direction           = normalize(direction);
  motion.velocity     = direction * (float)KRAB_BOSS_MS;
  motion.acceleration = {0, 0};

  Boss& boss = registry.bosses.emplace(entity);
  boss.type  = ENTITY_TYPE::KRAB_BOSS;

  addCrabBossWander();

  boss.ai_cd = KRAB_BOSS_AI_CD;
  boss.ai    = std::vector<std::function<void()>>(
      {addCrabMelee, addCrabMelee, addCrabMelee, addCrabMelee, addCrabMelee,
          addCrabMelee, addCrabMelee, addCrabMelee, addCrabRanged, addCrabRanged,
          addCrabBossWander});

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::KRAB_BOSS, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, KRAB_BOSS_HEALTH,
                         KRAB_BOSS_HEALTH_SCALE, KRAB_BOSS_HEALTH_BAR_SCALE,
                         KRAB_BOSS_HEALTH_BOUNDING_BOX);

  // Replace it with the boss key drop.
  Drop& drop  = registry.drops.emplace(entity);
  drop.dropFn = unlockBossDoors;

  return entity;
}

Entity createInitCrabBossPos(RenderSystem* renderer, vec2 position,
                             bool checkCollisions) {
  return createCrabBossPos(renderer, room_center, false);
}

/**
 * @brief Respawns krab boss based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnCrabBoss(RenderSystem* renderer, EntityState es) {
  Entity entity = createCrabBossPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

Entity createSharkmanPos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions) {
  // Reserve an entity
  auto  entity = Entity();
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = SHARKMAN_SCALE * SHARKMAN_BOUNDING_BOX;

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d = registry.deadlys.emplace(entity);
  d.type    = ENTITY_TYPE::SHARKMAN;

  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = SHARKMAN_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = SHARKMAN_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion = registry.motions.emplace(entity);
  // sharkman starts off running opposite direction of player
  vec2 direction = pos.position - registry.positions.get(player).position;
  if (direction.x < 0) {
    pos.scale.x = abs(pos.scale.x) * -1;
  } else {
    pos.scale.x = abs(pos.scale.x);
  }
  direction           = normalize(direction);
  motion.velocity     = direction * (float)SHARKMAN_MS;
  motion.acceleration = {0, 0};

  Boss& boss = registry.bosses.emplace(entity);
  boss.type  = ENTITY_TYPE::SHARKMAN;

  addSharkmanWander();

  boss.ai_cd   = SHARKMAN_AI_CD;
  boss.curr_cd = SHARKMAN_AI_CD;
  boss.ai      = std::vector<std::function<void()>>(
      {addSharkmanTarget, addSharkmanWander});

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SHARKMAN0, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, SHARKMAN_HEALTH,
                         SHARKMAN_HEALTH_SCALE, SHARKMAN_HEALTH_BAR_SCALE,
                         SHARKMAN_HEALTH_BOUNDING_BOX);

  // Replace it with the boss key drop.
  Drop& drop  = registry.drops.emplace(entity);
  drop.dropFn = unlockBossDoors;

  return entity;
}

Entity createInitSharkmanPos(RenderSystem* renderer, vec2 position,
                             bool checkCollisions) {
  return createSharkmanPos(renderer, room_center, false);
}

/**
 * @brief Respawns a boss based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnSharkman(RenderSystem* renderer, EntityState es) {
  Entity entity = createSharkmanPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

Entity createCthulhuPos(RenderSystem* renderer, vec2 position,
                        bool checkCollisions) {
  // Reserve an entity
  auto  entity = Entity();
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = room_center;
  pos.scale    = CTHULHU_SCALE * CTHULHU_BOUNDING_BOX;

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d = registry.deadlys.emplace(entity);
  d.type    = ENTITY_TYPE::CTHULHU;

  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = CTHULHU_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = CTHULHU_ATK_SPD;

  Boss& boss = registry.bosses.emplace(entity);
  boss.type  = ENTITY_TYPE::CTHULHU;

  addCthulhuTentacles();

  boss.ai_cd   = CTHULHU_AI_CD;
  boss.curr_cd = CTHULHU_AI_CD;
  boss.ai      = std::vector<std::function<void()>>(
      {addCthulhuCanisters, addCthulhuTentacles, addCthulhuFireballs});

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::CTHULHU_NORMAL, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, CTHULHU_HEALTH, CTHULHU_HEALTH_SCALE,
                         CTHULHU_HEALTH_BAR_SCALE, CTHULHU_HEALTH_BOUNDING_BOX);

  return entity;
}

Entity respawnCthulhu(RenderSystem* renderer, EntityState es) {
  Entity entity = createCthulhuPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

Entity createTentaclePos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions) {
  // Reserve an entity
  auto  entity = Entity();
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = TENTACLE_SCALE * TENTACLE_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d = registry.deadlys.emplace(entity);
  d.type    = ENTITY_TYPE::TENTACLE;

  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = TENTACLE_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = TENTACLE_ATK_SPD;

  auto& stun    = registry.stuns.emplace(entity);
  stun.duration = TENTACLE_STUN_MS;

  // Initialize the position, scale, and physics components
  auto& motion    = registry.motions.emplace(entity);
  motion.velocity = {TENTACLE_MS, 0};

  // ai
  auto& wander         = registry.wanders.emplace(entity);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(TENTACLE_MIN_DIR_CD, TENTACLE_MAX_DIR_CD);

  auto& tracking        = registry.trackPlayer.emplace(entity);
  tracking.tracking_cd  = TENTACLE_TRACKING_CD;
  tracking.spot_radius  = TENTACLE_SPOT_RADIUS;
  tracking.leash_radius = TENTACLE_LEASH_RADIUS;

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::TENTACLE, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, TENTACLE_HEALTH,
                         TENTACLE_HEALTH_SCALE, TENTACLE_HEALTH_BAR_SCALE,
                         TENTACLE_HEALTH_BOUNDING_BOX);

  return entity;
}

Entity respawnTentacle(RenderSystem* renderer, EntityState es) {
  Entity entity = createTentaclePos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

Entity shootShockwave(RenderSystem* renderer, vec2 pos) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.scale     = 0.01f * SHOCKWAVE_BOUNDING_BOX;

  // this is here just so physics_system can see it in step
  Motion& motion = registry.motions.emplace(entity);

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = SHOCKWAVE_DAMAGE;

  EnemyProjectile& proj = registry.enemyProjectiles.emplace(entity);
  proj.type             = ENTITY_TYPE::SHOCKWAVE;
  proj.has_timer        = true;
  proj.timer            = SHOCKWAVE_TIMER;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SHOCKWAVE, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity shootFireball(RenderSystem* renderer, vec2 pos, vec2 direction) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  float     angle    = atan2(direction.y, direction.x);
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.scale     = FIREBALL_SCALE * FIREBALL_BOUNDING_BOX;
  position.angle     = angle;

  // Setting initial motion values
  Motion& motion  = registry.motions.emplace(entity);
  motion.velocity = {cos(angle) * FIREBALL_MS, sin(angle) * FIREBALL_MS};

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = FIREBALL_DAMAGE;

  EnemyProjectile& proj = registry.enemyProjectiles.emplace(entity);
  proj.type             = ENTITY_TYPE::FIREBALL;
  proj.has_timer        = true;
  proj.timer            = FIREBALL_TIMER;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::FIREBALL, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity shootCanister(RenderSystem* renderer, vec2 pos, vec2 direction,
                     bool is_angry) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  float     angle    = atan2(direction.y, direction.x);
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.scale = OXYGEN_CANISTER_SCALE_FACTOR * OXYGEN_CANISTER_BOUNDING_BOX;
  position.angle = angle;

  // Setting initial motion values
  Motion& motion  = registry.motions.emplace(entity);
  motion.velocity = {cos(angle), sin(angle)};
  motion.velocity *= is_angry ? CTHULHU_RAGE_CANISTER_MS : CTHULHU_CANISTER_MS;

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = OXYGEN_CANISTER_DAMAGE;

  // aoe for explosion
  AreaOfEffect& aoe = registry.aoe.emplace(entity);
  aoe.radius        = OXYGEN_CANISTER_DAMAGE_RADIUS;

  EnemyProjectile& proj = registry.enemyProjectiles.emplace(entity);
  proj.type             = ENTITY_TYPE::OXYGEN_CANISTER;
  proj.has_timer        = true;
  proj.timer = is_angry ? CTHULHU_RAGE_CANISTER_TIMER : CTHULHU_CANISTER_TIMER;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::OXYGEN_CANISTER, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity shootRageProjectile(RenderSystem* renderer, vec2 pos, float angle) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.scale     = CTHULHU_RAGE_PROJ_SCALE * CTHULHU_RAGE_PROJ_BOUNDING_BOX;
  position.angle     = angle;

  // Setting initial motion values
  Motion& motion  = registry.motions.emplace(entity);
  motion.velocity = {cos(angle) * CTHULHU_RAGE_PROJ_MS,
                     sin(angle) * CTHULHU_RAGE_PROJ_MS};

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = CTHULHU_RAGE_PROJ_DAMAGE;

  EnemyProjectile& proj = registry.enemyProjectiles.emplace(entity);
  proj.type             = ENTITY_TYPE::RAGE_PROJ;
  proj.has_timer        = true;
  proj.timer            = CTHULHU_RAGE_PROJ_TIMER;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::RAGE_PROJ, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}