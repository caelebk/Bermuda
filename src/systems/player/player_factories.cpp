#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"

/********************************************************************************
 * Create a New Player
 *
 * @param renderer
 * @param pos: position to spawn player
 * @param projectile: projectile to start with (Ex. HARPOON_PROJECTILE)
 ********************************************************************************/
Entity createPlayer(RenderSystem *renderer, vec2 pos, int projectile) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.position = pos;
  position.angle = 0.f;
  position.scale = PLAYER_SCALE_FACTOR * PLAYER_BOUNDING_BOX;

  // Setting initial motion values
  Motion &motion = registry.motions.emplace(entity);
  motion.velocity = {0.f, 0.f};
  motion.acceleration = {0, 0};

	// Make Player
	Player &player = registry.players.emplace(entity);
  player.weapon = createLoadedGun(renderer, position.position, projectile);

  // Request Render
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });
    
	return entity;
}

/********************************************************************************
 * Create a New Loaded Gun
 *
 * @param renderer
 * @param playerPosition: position of corresponding Player
 * @param projectile: projectile to start with (Ex. HARPOON_PROJECTILE)
 ********************************************************************************/
Entity createLoadedGun(RenderSystem *renderer, vec2 playerPosition, int projectile) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.position = playerPosition + GUN_RELATIVE_POS_FROM_PLAYER; // TODO: REMOVE GUN_RELATIVE_POS_FROM_PLAYER once Gun position/angle
  position.angle = 0.f;                                              //       is updated accordingly based on corresponding Player
  position.scale = GUN_SCALE_FACTOR * GUN_BOUNDING_BOX;

  // Setting initial motion values
  Motion &motion = registry.motions.emplace(entity); // TODO: REMOVE once Gun position/angle is updated accordingly based on corresponding Player
  motion.velocity = {0.f, 0.f};
  motion.acceleration = {0, 0};

  // Make Weapon
  PlayerWeapon &weapon = registry.playerWeapons.emplace(entity);
  switch(projectile) {
    case HARPOON_PROJECTILE:
    weapon.projectile = loadHarpoon(renderer, position.position);
    break;

    // TODO: Add other projectiles (i.e. nets, pistol shrimp, etc.)
  }

  // Request Render
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GUN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

/********************************************************************************
 * Load Harpoon into Gun
 *
 * @param renderer
 * @param gunPosition: position of corresponding Player
 ********************************************************************************/
Entity loadHarpoon(RenderSystem *renderer, vec2 gunPosition) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial positon values
  Position& position = registry.positions.emplace(entity);
  position.position = gunPosition + HARPOON_RELATIVE_POS_FROM_GUN;   // TODO: REMOVE HARPOON_RELATIVE_POS_FROM_GUN once Harpoon position/angle
  position.angle = 0.f;                                              //       is updated accordingly based on corresponding Gun
  position.scale = HARPOON_SCALE_FACTOR * HARPOON_BOUNDING_BOX;

  // Setting initial motion values
  // Motion will be used when acting as a projectile and is not loaded into a Gun
  Motion &motion = registry.motions.emplace(entity);
  motion.velocity = {0.f, 0.f};
  motion.acceleration = {0, 0};

  // Make Projectile
  PlayerProjectile &projectile = registry.playerProjectiles.emplace(entity);
  // Status: projectile is currently loaded and position/angle should be updated accordingly based on corresponding Gun
  projectile.is_loaded = true; 
	
  // Request Render
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HARPOON,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

/********************************************************************************
 * Get Reference to Player Weapon
 *
 * @param player
 ********************************************************************************/
Entity &getPlayerWeapon(Entity& player) {
  return registry.players.get(player).weapon;
}

/********************************************************************************
 * Get Reference to Player Projectile
 *
 * @param player
 ********************************************************************************/
Entity &getPlayerProjectile(Entity& player) {
  Entity &player_weapon = getPlayerWeapon(player);
  return registry.playerWeapons.get(player_weapon).projectile;
}