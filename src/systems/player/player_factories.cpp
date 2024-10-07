#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"

Entity createPlayer(RenderSystem *renderer, vec2 pos) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial motion values
  Position& position = registry.positions.emplace(entity);
  position.position = pos;
  position.angle = 0.f;
  position.scale = vec2(0.7) * vec2(42.f, 64.f); // format: vec2(scale_factor) * vec2(PNG_dimensions)

  Motion &motion = registry.motions.emplace(entity);
  motion.velocity = {0.f, 0.f};

	// create an empty Salmon component for our character
	registry.players.emplace(entity);
	// TODO: Change this
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}
