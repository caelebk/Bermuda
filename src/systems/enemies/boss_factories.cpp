#include "boss_factories.hpp"
#include <functional>
#include <vector>

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
  Entity& b = registry.bosses.entities[0];

  auto& tracking        = registry.trackPlayerRanged.emplace(b);
  tracking.tracking_cd  = KRAB_BOSS_TRACKING_CD;
  tracking.spot_radius  = KRAB_BOSS_SPOT_RADIUS * 2;
  tracking.min_distance = KRAB_BOSS_SPOT_RADIUS / 2;
  tracking.leash_radius = KRAB_BOSS_LEASH_RADIUS * 2;
  tracking.acceleration = KRAB_BOSS_TRACKING_ACCELERATION;
  printf("Crab boss is trying to shoot player!\n");

  addCrabBossWander();
}

Entity createCrabBossPos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto  entity = Entity();
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = KRAB_BOSS_SCALE * KRAB_BOSS_BOUNDING_BOX;

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  registry.deadlys.emplace(entity);
  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = KRAB_BOSS_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = KRAB_BOSS_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-KRAB_BOSS_MS, 0};
  motion.acceleration = {0, 0};

  Boss& boss = registry.bosses.emplace(entity);
  addCrabBossWander();

  boss.ai_cd = KRAB_BOSS_AI_CD;
  boss.ai    = std::vector<std::function<void()>>({addCrabMelee, addCrabMelee, addCrabMelee, addCrabMelee, addCrabRanged, addCrabRanged, addCrabBossWander});

  // TODO: add the room
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::KRAB, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, KRAB_BOSS_HEALTH,
                         KRAB_BOSS_HEALTH_SCALE, KRAB_BOSS_HEALTH_BAR_SCALE,
                         KRAB_BOSS_HEALTH_BOUNDING_BOX);
  return entity;
}
