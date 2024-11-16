#pragma once
#include <functional>
#include "physics.hpp"
#include "render_system.hpp"

struct EntityState {
    float oxygen = 0.0;
    Position position;
    unsigned int group = 0;
};

class EntitySave {
private:
    struct EntityState es;
    std::function<Entity(RenderSystem *renderer, struct EntityState es)> respawnFn;
public:
    EntitySave(Entity e);
    void respawn(RenderSystem *renderer);
};
