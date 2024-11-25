#pragma once

#include "entity_type.hpp"
#include "physics.hpp"
#include "render_system.hpp"

struct EntityState {
  float        oxygen = 0.0;
  Position     position;
  unsigned int group = 0;
  ENTITY_TYPE  type;
};

class EntitySave {
  public:
  struct EntityState es;
  EntitySave(Entity e);
  EntitySave(EntityState es);
  void respawn(RenderSystem* renderer);
};
