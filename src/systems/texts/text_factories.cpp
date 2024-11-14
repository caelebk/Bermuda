#include "text_factories.hpp"
#include "tiny_ecs_registry.hpp"

Entity requestText(RenderSystem* renderer, std::string text, float textScale,
                   vec3 textColor, vec2 textPosition) {
  // Reserve en entity
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Initialize the position
  auto& position    = registry.positions.emplace(entity);
  position.angle    = 0.f;
  position.position = textPosition;
  position.scale    = vec2(1.f);

  // choose color
  auto& color = registry.colors.emplace(entity);
  color       = textColor;

  // make text request
  auto& textRequest     = registry.textRequests.emplace(entity);
  textRequest.text      = text;
  textRequest.textScale = textScale;

  return entity;
}