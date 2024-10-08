#pragma once
#include <vector>

#include "abilities.hpp"
#include "ai.hpp"
#include "components.hpp"
#include "enemy.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "tiny_ecs.hpp"

class ECSRegistry {
  // Callbacks to remove a particular or all entities in the system
  std::vector<ContainerInterface *> registry_list;

public:
  // Manually created list of all components this game has
  // physics related
  ComponentContainer<Motion> motions;
  ComponentContainer<Position> positions;
  ComponentContainer<Collision> collisions;

  // player related
  ComponentContainer<DeathTimer> deathTimers;
  ComponentContainer<Player> players;
  ComponentContainer<PlayerWeapon> playerWeapons;
  ComponentContainer<PlayerProjectile> playerProjectiles;
  ComponentContainer<Oxygen> oxygen;

  // enemy related
  ComponentContainer<Deadly> deadlys;
  ComponentContainer<DamageOnTouch> damageTouch;

  // ai related
  ComponentContainer<Wander> wanders;

  // abilities related
  ComponentContainer<Stun> stuns;

  // render related
  ComponentContainer<Mesh *> meshPtrs;
  ComponentContainer<RenderRequest> renderRequests;
  ComponentContainer<vec3> colors;
  ComponentContainer<ScreenState> screenStates;

  // other
  ComponentContainer<Consumable> consumables;
  ComponentContainer<DebugComponent> debugComponents;

  // constructor that adds all containers for looping over them
  // IMPORTANT: Don't forget to add any newly added containers!
  ECSRegistry() {
    // physics related
    registry_list.push_back(&motions);
    registry_list.push_back(&collisions);
    registry_list.push_back(&positions);
    // player related
    registry_list.push_back(&deathTimers);
    registry_list.push_back(&players);
    registry_list.push_back(&playerWeapons);
    registry_list.push_back(&playerProjectiles);
    registry_list.push_back(&oxygen);
    // enemy related
    registry_list.push_back(&deadlys);
    registry_list.push_back(&damageTouch);
    // ai related
    registry_list.push_back(&wanders);
    // abilities related
    registry_list.push_back(&stuns);
    // render related
    registry_list.push_back(&meshPtrs);
    registry_list.push_back(&renderRequests);
    registry_list.push_back(&screenStates);
    registry_list.push_back(&colors);
    // other
    registry_list.push_back(&debugComponents);
    registry_list.push_back(&consumables);
  }

  void clear_all_components() {
    for (ContainerInterface *reg : registry_list)
      reg->clear();
  }

  void list_all_components() {
    printf("Debug info on all registry entries:\n");
    for (ContainerInterface *reg : registry_list)
      if (reg->size() > 0)
        printf("%4d components of type %s\n", (int)reg->size(),
               typeid(*reg).name());
  }

  void list_all_components_of(Entity e) {
    printf("Debug info on components of entity %u:\n", (unsigned int)e);
    for (ContainerInterface *reg : registry_list)
      if (reg->has(e))
        printf("type %s\n", typeid(*reg).name());
  }

  void remove_all_components_of(Entity e) {
    for (ContainerInterface *reg : registry_list)
      reg->remove(e);
  }
};

extern ECSRegistry registry;
