#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 300.f;
	motion.scale.y *= 1; // point front to the right

	// create an empty Salmon component for our character
	registry.players.emplace(entity);
	// TODO: Change this
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::SALMON });

	return entity;
}