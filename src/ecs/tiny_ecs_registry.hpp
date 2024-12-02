#pragma once
#include <vector>

#include "abilities.hpp"
#include "ai.hpp"
#include "audio.hpp"
#include "components.hpp"
#include "enemy.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "status.hpp"
#include "tiny_ecs.hpp"

class ECSRegistry {
  // Callbacks to remove a particular or all entities in the system
  std::vector<ContainerInterface*> registry_list;

  public:
  // Manually created list of all components this game has
  // physics related
  ComponentContainer<Motion>    motions;
  ComponentContainer<Position>  positions;
  ComponentContainer<Collision> collisions;
  ComponentContainer<Mass>      masses;

  // player related
  ComponentContainer<DeathTimer>          deathTimers;
  ComponentContainer<Player>              players;
  ComponentContainer<PlayerCollisionMesh> playersCollisionMeshes;
  ComponentContainer<PlayerWeapon>        playerWeapons;
  ComponentContainer<PlayerProjectile>    playerProjectiles;
  ComponentContainer<Explosion>           explosions;
  ComponentContainer<Inventory>           inventory;
  ComponentContainer<Key>                 keys;
  ComponentContainer<PlayerHUD>           playerHUD;
  ComponentContainer<InventoryCounter>    inventoryCounters;
  ComponentContainer<Communication>       communications;
  ComponentContainer<Notification>        notifications;

  // enemy related
  ComponentContainer<Deadly>          deadlys;
  ComponentContainer<EnemyProjectile> enemyProjectiles;
  ComponentContainer<EnemySupport>    enemySupports;
  ComponentContainer<Boss>            bosses;
  ComponentContainer<ModifyOxygenCD>  modifyOxygenCd;
  ComponentContainer<Lobster>         lobsters;

  // oxygen related
  ComponentContainer<Oxygen>         oxygen;
  ComponentContainer<OxygenModifier> oxygenModifiers;

  // ai related
  ComponentContainer<Wander>             wanders;
  ComponentContainer<WanderLine>         wanderLines;
  ComponentContainer<WanderSquare>       wanderSquares;
  ComponentContainer<TracksPlayer>       trackPlayer;
  ComponentContainer<TracksPlayerRanged> trackPlayerRanged;
  ComponentContainer<Group>              groups;
  ComponentContainer<EntityGroup>        entityGroups;
  ComponentContainer<Shooter>            shooters;

  // abilities related
  ComponentContainer<Stun>             stuns;
  ComponentContainer<KnockBack>        knockbacks;
  ComponentContainer<AreaOfEffect>     aoe;
  ComponentContainer<ActsAsProjectile> actsAsProjectile;

  // render related
  ComponentContainer<Mesh*>         meshPtrs;
  ComponentContainer<RenderRequest> renderRequests;
  ComponentContainer<vec3>          colors;
  ComponentContainer<ScreenState>   screenStates;
  ComponentContainer<TextRequest>   textRequests;
  ComponentContainer<SaveStatus>    saveStatuses;

  // level related
  ComponentContainer<SpaceBoundingBox> bounding_boxes;
  ComponentContainer<Vector>           vectors;
  ComponentContainer<Space>            spaces;
  ComponentContainer<DoorConnection>   doorConnections;
  ComponentContainer<ActiveWall>       activeWalls;
  ComponentContainer<ActiveDoor>       activeDoors;
  ComponentContainer<Interactable>     interactable;
  ComponentContainer<Floor>            floors;
  ComponentContainer<Geyser>           geysers;
  ComponentContainer<Bubble>           bubbles;
  ComponentContainer<Breakable>        breakables;
  ComponentContainer<PressurePlate>    pressurePlates;
  ComponentContainer<Ambient>    ambient;

  // status related
  ComponentContainer<LowOxygen>   lowOxygen;
  ComponentContainer<Stunned>     stunned;
  ComponentContainer<KnockedBack> knockedback;
  ComponentContainer<Attacked>    attacked;

  // audio related
  ComponentContainer<Sound> sounds;
  ComponentContainer<Music> musics;

  // other
  ComponentContainer<Consumable>     consumables;
  ComponentContainer<Item>           items;
  ComponentContainer<Drop>           drops;
  ComponentContainer<WeaponDrop>     weaponDrops;
  ComponentContainer<DebugComponent> debugComponents;
  ComponentContainer<Emoting>        emoting;
  ComponentContainer<GameCursor>     cursors;
  ComponentContainer<Overlay>        overlays;
  ComponentContainer<RoomTransition> roomTransitions;

  // constructor that adds all containers for looping over them
  // IMPORTANT: Don't forget to add any newly added containers!
  ECSRegistry() {
    // physics related
    registry_list.push_back(&motions);
    registry_list.push_back(&collisions);
    registry_list.push_back(&positions);
    registry_list.push_back(&masses);
    // player related
    registry_list.push_back(&deathTimers);
    registry_list.push_back(&players);
    registry_list.push_back(&playersCollisionMeshes);
    registry_list.push_back(&playerWeapons);
    registry_list.push_back(&playerProjectiles);
    registry_list.push_back(&explosions);
    registry_list.push_back(&inventory);
    registry_list.push_back(&keys);
    registry_list.push_back(&playerHUD);
    registry_list.push_back(&inventoryCounters);
    registry_list.push_back(&communications);
    registry_list.push_back(&notifications);
    // enemy related
    registry_list.push_back(&deadlys);
    registry_list.push_back(&enemyProjectiles);
    registry_list.push_back(&enemySupports);
    registry_list.push_back(&bosses);
    registry_list.push_back(&modifyOxygenCd);
    registry_list.push_back(&groups);
    registry_list.push_back(&entityGroups);
    // oxygen related
    registry_list.push_back(&oxygen);
    registry_list.push_back(&oxygenModifiers);
    // ai related
    registry_list.push_back(&wanders);
    registry_list.push_back(&wanderLines);
    registry_list.push_back(&wanderSquares);
    registry_list.push_back(&trackPlayer);
    registry_list.push_back(&trackPlayerRanged);
    registry_list.push_back(&shooters);
    // abilities related
    registry_list.push_back(&stuns);
    registry_list.push_back(&knockbacks);
    registry_list.push_back(&aoe);
    // render related
    registry_list.push_back(&meshPtrs);
    registry_list.push_back(&renderRequests);
    registry_list.push_back(&screenStates);
    registry_list.push_back(&colors);
    registry_list.push_back(&textRequests);
    registry_list.push_back(&saveStatuses);
    // level related
    registry_list.push_back(&bounding_boxes);
    registry_list.push_back(&vectors);
    registry_list.push_back(&spaces);
    registry_list.push_back(&doorConnections);
    registry_list.push_back(&activeWalls);
    registry_list.push_back(&activeDoors);
    registry_list.push_back(&interactable);
    registry_list.push_back(&geysers);
    registry_list.push_back(&bubbles);
    registry_list.push_back(&floors);
    registry_list.push_back(&breakables);
    registry_list.push_back(&pressurePlates);
    registry_list.push_back(&ambient);
    // status related
    registry_list.push_back(&lowOxygen);
    registry_list.push_back(&stunned);
    registry_list.push_back(&knockedback);
    registry_list.push_back(&attacked);
    // audio related
    registry_list.push_back(&sounds);
    registry_list.push_back(&musics);
    // other
    registry_list.push_back(&drops);
    registry_list.push_back(&weaponDrops);
    registry_list.push_back(&cursors);
    registry_list.push_back(&debugComponents);
    registry_list.push_back(&consumables);
    registry_list.push_back(&items);
    registry_list.push_back(&emoting);
    registry_list.push_back(&overlays);
    registry_list.push_back(&roomTransitions);
  }

  void clear_all_components() {
    for (ContainerInterface* reg : registry_list) reg->clear();
  }

  void list_all_components() {
    printf("Debug info on all registry entries:\n");
    for (ContainerInterface* reg : registry_list)
      if (reg->size() > 0)
        printf("%4d components of type %s\n", (int)reg->size(),
               typeid(*reg).name());
  }

  void list_all_components_of(Entity e) {
    printf("Debug info on components of entity %u:\n", (unsigned int)e);
    for (ContainerInterface* reg : registry_list)
      if (reg->has(e)) printf("type %s\n", typeid(*reg).name());
  }

  void remove_all_components_of(Entity e) {
    // player, collision, emoting oxygen
    if (oxygen.has(e)) {
      Oxygen& o = oxygen.get(e);
      remove_all_components_of(o.oxygenBar);
      remove_all_components_of(o.backgroundBar);
    }

    if (entityGroups.has(e)) {
      // remove from group if they are in one
      EntityGroup& eg = entityGroups.get(e);
      if (groups.has(eg.group)) {
        Group& g = groups.get(eg.group);
        g.members.erase(std::remove(g.members.begin(), g.members.end(), e),
                        g.members.end());
      }
    }

    if (players.has(e)) {
      Player& p = players.get(e);
      remove_all_components_of(p.weapon);
    }

    if (emoting.has(e)) {
      Emoting& ee = emoting.get(e);
      remove_all_components_of(ee.child);
    }

    for (ContainerInterface* reg : registry_list) reg->remove(e);
  }
};

extern ECSRegistry registry;
