#pragma once
#include <functional>
#include "physics.hpp"
#include "render_system.hpp"

struct EntityState {
    float oxygen = 0.0;
    Position position;
};

class EntitySave {
private:
    struct EntityState es;
    std::function<void(RenderSystem *renderer, struct EntityState es)> respawnFn;
public:
    EntitySave(Entity e);
    void respawn(RenderSystem *renderer);
};
