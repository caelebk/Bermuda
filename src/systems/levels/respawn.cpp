#include "respawn.hpp"
#include <cstdio>

#include "ai.hpp"
#include "boss_factories.hpp"
#include "consumable_factories.hpp"
#include "enemy_factories.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "map_factories.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"

const std::unordered_map<
    ENTITY_TYPE, std::function<Entity(RenderSystem* renderer, EntityState es)>>
    respawnFnMap = {
        {ENTITY_TYPE::JELLY, respawnJelly},
        {ENTITY_TYPE::FISH, respawnFish},
        {ENTITY_TYPE::SHARK, respawnShark},
        {ENTITY_TYPE::KRAB, respawnKrab},
        {ENTITY_TYPE::URCHIN, respawnUrchin},
        {ENTITY_TYPE::SEAHORSE, respawnSeahorse},
        {ENTITY_TYPE::LOBSTER, respawnLobster},
        {ENTITY_TYPE::BREAKABLE_CRATE, respawnCrate},
        {ENTITY_TYPE::METAL_CRATE, respawnMetalCrate},
        {ENTITY_TYPE::ROCK, respawnRock},
        {ENTITY_TYPE::PRESSURE_PLATE, respawnPressurePlate},
        {ENTITY_TYPE::OXYGEN_CANISTER, respawnOxygenCanister},
        {ENTITY_TYPE::GEYSER, respawnGeyser},
        {ENTITY_TYPE::CONCUSSIVE, respawnConcussiveDrop},
        {ENTITY_TYPE::NET, respawnNetDrop},
        {ENTITY_TYPE::TORPEDO, respawnTorpedoDrop},
        {ENTITY_TYPE::SHRIMP, respawnShrimpDrop},
        {ENTITY_TYPE::RED_KEY, createRedKeyRespawnFn},
        {ENTITY_TYPE::BLUE_KEY, createBlueKeyRespawnFn},
        {ENTITY_TYPE::YELLOW_KEY, createYellowKeyRespawnFn},
        {ENTITY_TYPE::KRAB_BOSS, respawnCrabBoss},
        {ENTITY_TYPE::SHARKMAN, respawnSharkman},
        {ENTITY_TYPE::KELP, respawnKelp},
        {ENTITY_TYPE::CORAL, respawnCoral},
        {ENTITY_TYPE::JUNK, respawnJunk},
        {ENTITY_TYPE::BONES, respawnBones},
};

EntitySave::EntitySave(EntityState es) {
  this->es = es;
}

EntitySave::EntitySave(Entity e) {
  if (registry.oxygen.has(e)) {
    Oxygen& o       = registry.oxygen.get(e);
    this->es.oxygen = o.level;
  } else {
    this->es.oxygen = -1.0;
  }

  if (registry.positions.has(e)) {
    Position& pos     = registry.positions.get(e);
    this->es.position = pos;
  } 

  // brute force it
  if (registry.deadlys.has(e)) {
    Deadly& d     = registry.deadlys.get(e);
    this->es.type = d.type;
  } else if (registry.consumables.has(e)) {
    Consumable& c = registry.consumables.get(e);
    this->es.type = c.type;
  } else if (registry.items.has(e)) {
    Item& i       = registry.items.get(e);
    this->es.type = i.type;
  } else if (registry.interactable.has(e)) {
    Interactable& i = registry.interactable.get(e);
    this->es.type   = i.type;
  } else if (registry.breakables.has(e)) {
    Breakable& b  = registry.breakables.get(e);
    this->es.type = b.type;
  } else if (registry.ambient.has(e)) {
    Ambient& a  = registry.ambient.get(e);
    this->es.type = a.type;
  } else {
    assert("You're trying to record an EntitySave but it isn't configured" &&
           false);  // literally just die
  }

  if (registry.entityGroups.has(e)) {
    EntityGroup& eg = registry.entityGroups.get(e);
    this->es.group  = eg.group;
  }
}

void EntitySave::respawn(RenderSystem* renderer) {
  auto respawn_it =
      respawnFnMap.find(this->es.type);
  if (respawn_it == respawnFnMap.end()) {
    return;
  }

  std::function<Entity(RenderSystem * renderer, EntityState es)> respawnFn = respawn_it->second;

  Entity e = respawnFn(renderer, this->es);

  // if was in a group, reassign
  if ((unsigned int)this->es.group != 0) {
    Entity g = Entity(this->es.group);
    if (!registry.groups.has(g)) {
      registry.groups.emplace(g);
    }
    Group&       group = registry.groups.get(g);
    EntityGroup& eg    = registry.entityGroups.emplace(e);
    group.members.push_back(e);
    eg.group         = g;
    eg.active_dir_cd = randomFloat(0.f, eg.change_dir_cd);
  }
}
