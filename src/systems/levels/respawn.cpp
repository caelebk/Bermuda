#include "respawn.hpp"
#include "ai.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"

EntitySave::EntitySave(Entity e) {
    if (registry.oxygen.has(e)) {
        Oxygen &o = registry.oxygen.get(e);
        this->es.oxygen = o.level;
    } else {
        this->es.oxygen = -1.0;
    }

    if (registry.positions.has(e)) {
        Position &pos = registry.positions.get(e);
        this->es.position = pos;
    }

    // brute force it
    if (registry.deadlys.has(e)) {
        Deadly &d = registry.deadlys.get(e);
        this->respawnFn = d.respawnFn;
    } else if (registry.consumables.has(e)) {
        Consumable &c = registry.consumables.get(e);
        this->respawnFn = c.respawnFn;
    } else if (registry.items.has(e)) {
        Item &i = registry.items.get(e);
        this->respawnFn = i.respawnFn;
    } else if (registry.interactable.has(e)) {
        Interactable &i = registry.interactable.get(e);
        this->respawnFn = i.respawnFn;
    } else if (registry.breakables.has(e)) {
        Breakable &b = registry.breakables.get(e);
        this->respawnFn = b.respawnFn;
    } else {
        assert("You're trying to record an EntitySave but it isn't configured" && false); // literally just die
    }

    if (registry.entityGroups.has(e)) {
        EntityGroup &eg = registry.entityGroups.get(e);
        this->es.group = eg.group;
    }
}

void EntitySave::respawn(RenderSystem *renderer) {
   Entity e = this->respawnFn(renderer, this->es);

    // if was in a group, reassign
    if ((unsigned int)this->es.group != 0) {
        Entity g = Entity(this->es.group);
        if (!registry.groups.has(g)) {
            registry.groups.emplace(g);
        }
        Group &group = registry.groups.get(g);
        EntityGroup &eg = registry.entityGroups.emplace(e);
        group.members.push_back(e);
        eg.group = g;
        eg.active_dir_cd = randomFloat(0.f, eg.change_dir_cd);
    }
}
