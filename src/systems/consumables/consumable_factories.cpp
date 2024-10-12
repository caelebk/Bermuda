#include "consumable_factories.hpp"
#include <iostream>

/////////////////////////////////////////////////////////////////
// Oxygen Tank
/////////////////////////////////////////////////////////////////
/**
 * @brief creates an oxygen tank at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createOxygenTankPos(RenderSystem *renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  registry.consumables.emplace(entity);

  // Add stats
  auto &damage = registry.damageTouch.emplace(entity);
  damage.damage = OXYGEN_TANK_QTY;

  // physics and pos
  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = OXYGEN_TANK_SCALE_FACTOR * OXYGEN_TANK_BOUNDING_BOX;

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::OXYGEN_TANK,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}
