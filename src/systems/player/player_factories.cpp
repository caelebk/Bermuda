#include "player_factories.hpp"

#include "tiny_ecs_registry.hpp"

/********************************************************************************
 * Create a New Player
 *
 * @param renderer
 * @param pos: position to spawn player
 * @param projectile: projectile to start with (Ex. HARPOON_PROJECTILE)
 ********************************************************************************/
Entity createPlayer(RenderSystem* renderer, vec2 pos) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.angle     = 0.f;
  position.scale     = PLAYER_SCALE_FACTOR * PLAYER_BOUNDING_BOX;

  // Setting initial motion values
  Motion& motion      = registry.motions.emplace(entity);
  motion.velocity     = {0.f, 0.f};
  motion.acceleration = {0, 0};

  // Set Player Mass
  Mass& mass = registry.masses.emplace(entity);
  mass.mass  = PLAYER_MASS;

  // Make Player and Harpoon Gun
  Player& player = registry.players.emplace(entity);
  player.weapon =
      createLoadedGun(renderer, position.position, PROJECTILES::HARPOON);

  registry.renderRequests.insert(
    entity, {TEXTURE_ASSET_ID::PLAYER1, EFFECT_ASSET_ID::PLAYER,
              GEOMETRY_BUFFER_ID::SPRITE});

  auto collisionEntity = Entity();
  Mesh& collisionMesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER);
  registry.meshPtrs.emplace(collisionEntity, &collisionMesh);

  // Setting initial position values
  Position& collisionMeshPosition = registry.positions.emplace(collisionEntity);
  collisionMeshPosition.position  = pos;
  collisionMeshPosition.angle     = 0.f;
  collisionMeshPosition.scale     = PLAYER_SCALE_FACTOR * PLAYER_BOUNDING_BOX;

  PlayerCollisionMesh& collision_mesh_comp = registry.playersCollisionMeshes.emplace(collisionEntity);
  // Uncomment to render the collision mesh.
  // registry.renderRequests.insert(
  //     collisionEntity, {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::COLLISION_MESH,
  //              GEOMETRY_BUFFER_ID::PLAYER});

  player.collisionMesh = collisionEntity;

  return entity;
}

/********************************************************************************
 * Create a New Loaded Gun
 *
 * @param renderer
 * @param playerPosition: position of corresponding Player
 * @param projectile: projectile to start with (Ex. HARPOON_PROJECTILE)
 ********************************************************************************/
Entity createLoadedGun(RenderSystem* renderer, vec2 playerPosition,
                       PROJECTILES projectile) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.scale     = GUN_SCALE_FACTOR * GUN_BOUNDING_BOX;

  // Setting initial motion values
  Motion& motion = registry.motions.emplace(entity);
  motion.velocity     = {0.f, 0.f};
  motion.acceleration = {0, 0};

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = HARPOON_GUN_OXYGEN_COST;

  // Make Weapon
  PlayerWeapon& weapon = registry.playerWeapons.emplace(entity);
  switch (projectile) {
    case PROJECTILES::HARPOON:
      weapon.projectile = loadHarpoon(renderer, position.position);
      break;
    case PROJECTILES::NET:
      weapon.projectile = loadNet(renderer);
      break;
    case PROJECTILES::CONCUSSIVE:
      weapon.projectile = loadConcussive(renderer);
      break;
    case PROJECTILES::TORPEDO:
      weapon.projectile = loadTorpedo(renderer);
      break;
    case PROJECTILES::SHRIMP:
      weapon.projectile = loadShrimp(renderer);
      break;
  }

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::HARPOON_GUN, EFFECT_ASSET_ID::PLAYER,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity createConsumableGun(RenderSystem* renderer, float oxy_cost,
                           PROJECTILES projectile) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Make Weapon
  PlayerWeapon& weapon = registry.playerWeapons.emplace(entity);
  switch (projectile) {
    case PROJECTILES::NET:
      weapon.projectile = net;
      break;
    case PROJECTILES::CONCUSSIVE:
      weapon.projectile = concussive;
      break;
    case PROJECTILES::TORPEDO:
      weapon.projectile = torpedo;
      break;
    case PROJECTILES::SHRIMP:
      weapon.projectile = shrimp;
      break;
  }

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = oxy_cost;

  return entity;
}

/********************************************************************************
 * Load Harpoon into Gun
 *
 * @param renderer
 * @param gunPosition: position of corresponding Player
 ********************************************************************************/
Entity loadHarpoon(RenderSystem* renderer, vec2 gunPosition) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial positon values
  Position& position = registry.positions.emplace(entity);
  position.scale     = HARPOON_SCALE_FACTOR * HARPOON_BOUNDING_BOX;

  // Setting initial motion values
  // Motion will be used when acting as a projectile and is not loaded into a
  // Gun
  Motion& motion      = registry.motions.emplace(entity);
  motion.velocity     = {0.f, 0.f};
  motion.acceleration = {0, 0};

  // Make Projectile
  PlayerProjectile& projectile = registry.playerProjectiles.emplace(entity);
  // Status: projectile is currently loaded and position/angle should be updated
  // accordingly based on corresponding Gun
  projectile.is_loaded = true;
  projectile.type      = PROJECTILES::HARPOON;

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = HARPOON_GUN_OXYGEN_COST;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::HARPOON, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/********************************************************************************
 * Create net entity
 *
 * @param renderer
 * @param gunPosition: position of corresponding Player
 ********************************************************************************/
Entity loadNet(RenderSystem* renderer) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Make Projectile
  PlayerProjectile& projectile = registry.playerProjectiles.emplace(entity);
  // Status: projectile is currently loaded and position/angle should be updated
  // accordingly based on corresponding Gun
  projectile.is_loaded = true;
  projectile.type      = PROJECTILES::NET;

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = NET_OXYGEN_COST;

  Stun& stun    = registry.stuns.emplace(entity);
  stun.duration = NET_STUN_DURATION;

  return entity;
}

/********************************************************************************
 * Create concussive entity
 *
 * @param renderer
 ********************************************************************************/
Entity loadConcussive(RenderSystem* renderer) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Make Projectile
  PlayerProjectile& projectile = registry.playerProjectiles.emplace(entity);
  // Status: projectile is currently loaded and position/angle should be updated
  // accordingly based on corresponding Gun
  projectile.is_loaded = true;
  projectile.type      = PROJECTILES::CONCUSSIVE;

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = CONCUSSIVE_OXYGEN_COST;

  KnockBack& knockback = registry.knockbacks.emplace(entity);
  knockback.duration = CONCUSSIVE_KNOCKBACK_DURATION;

  return entity;
}

/********************************************************************************
 * Create torpedo projectile entity
 *
 * @param renderer
 * @param gunPosition: position of corresponding Player
 ********************************************************************************/
Entity loadTorpedo(RenderSystem* renderer) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Make Projectile
  PlayerProjectile& projectile = registry.playerProjectiles.emplace(entity);
  // Status: projectile is currently loaded and position/angle should be updated
  // accordingly based on corresponding Gun
  projectile.is_loaded = true;
  projectile.type      = PROJECTILES::TORPEDO;

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = TORPEDO_OXYGEN_COST;

  AreaOfEffect& aoe = registry.aoe.emplace(entity);
  aoe.radius = TORPEDO_DAMAGE_RADIUS;

  return entity;
}

/********************************************************************************
 * Create the goated projectile entity
 *
 * @param renderer
 * @param gunPosition: position of corresponding Player
 ********************************************************************************/
Entity loadShrimp(RenderSystem* renderer) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Make Projectile
  PlayerProjectile& projectile = registry.playerProjectiles.emplace(entity);
  // Status: projectile is currently loaded and position/angle should be updated
  // accordingly based on corresponding Gun
  projectile.is_loaded = true;
  projectile.type      = PROJECTILES::SHRIMP;

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = SHRIMP_OXYGEN_COST;

  return entity;
}

/********************************************************************************
 * @brief creates an oxygen tank for the player
 * TODO: change render request insertion
 *
 * @param renderer
 * @param pos - determines position of oxygen tank on screen
 * @return created oxygen tank
 ********************************************************************************/
void createOxygenTank(RenderSystem* renderer, Entity& player, vec2 pos) {
  auto playerOxygenBar     = Entity();
  auto playerBackgroundBar = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(playerOxygenBar, &mesh);
  registry.meshPtrs.emplace(playerBackgroundBar, &mesh);

  // Initialize the position, scale, and physics components
  auto& posComp    = registry.positions.emplace(playerOxygenBar);
  posComp.angle    = 0.f;
  posComp.position = pos;
  posComp.scale    = PLAYER_OXYGEN_SCALE_FACTOR * PLAYER_OXYGEN_BOUNDING_BOX;
  posComp.originalScale =
      PLAYER_OXYGEN_SCALE_FACTOR * PLAYER_OXYGEN_BOUNDING_BOX;

  auto& backgroundPos    = registry.positions.emplace(playerBackgroundBar);
  backgroundPos.angle    = 0.f;
  backgroundPos.position = pos + vec2(0.f, -43.f);
  backgroundPos.scale =
      PLAYER_OXYGEN_TANK_SCALE_FACTOR * PLAYER_OXYGEN_TANK_BOUNDING_BOX;

  // Add Oxygen Meter to Player HUD (Order Matters for Rendering)
  registry.playerHUD.emplace(playerBackgroundBar);
  registry.playerHUD.emplace(playerOxygenBar);

  // the player uses the default oxygen values
  auto& oxygen         = registry.oxygen.emplace(player);
  oxygen.capacity      = PLAYER_OXYGEN;
  oxygen.level         = PLAYER_OXYGEN;
  oxygen.rate          = PLAYER_OXYGEN_RATE;
  oxygen.isRendered    = true;
  oxygen.oxygenBar     = playerOxygenBar;
  oxygen.backgroundBar = playerBackgroundBar;

  registry.renderRequests.insert(
      playerOxygenBar,
      {TEXTURE_ASSET_ID::PLAYER_OXYGEN_BAR, EFFECT_ASSET_ID::TEXTURED_OXYGEN,
       GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      playerBackgroundBar,
      {TEXTURE_ASSET_ID::PLAYER_OXYGEN_TANK, EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});
}

/********************************************************************************
 * Get Reference to Player Weapon
 *
 * @param player
 ********************************************************************************/
Entity& getPlayerWeapon() {
  return registry.players.get(player).weapon;
}

/********************************************************************************
 * Get Reference to Player Projectile
 *
 * @param player
 ********************************************************************************/
Entity& getPlayerProjectile() {
  Entity& player_weapon = getPlayerWeapon();
  return registry.playerWeapons.get(player_weapon).projectile;
}
