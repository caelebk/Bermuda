#include "respawn.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "physics.hpp"
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
        assert(false); // literally just die
    }
}

void EntitySave::respawn(RenderSystem *renderer) {
   this->respawnFn(renderer, this->es);
}
