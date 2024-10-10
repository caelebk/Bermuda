#include "oxygen_system.hpp"
#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief creates an oxygen tank for the player
 * TODO: change render request insertion
 *
 * @param renderer
 * @param pos - determines position of oxygen tank on screen
 * @return created oxygen tank
 */
Entity createOxygenTank(RenderSystem *renderer, vec2 pos)
{
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the position, scale, and physics components
    auto &posComp = registry.positions.emplace(entity);
    posComp.angle = 0.f;
    posComp.position = pos;
    posComp.scale = PLAYER_OXYGEN_BOUNDING_BOX;

    // the player uses the default oxygen values
    auto &oxygen = registry.oxygen.emplace(entity);
    oxygen.capacity = PLAYER_OXYGEN;
    oxygen.level = PLAYER_OXYGEN;
    oxygen.rate = PLAYER_OXYGEN_RATE;

    // TODO: change to proper texture
    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::TEXTURE_COUNT,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

/**
 * @brief gets the oxygen level of an entity
 */
float getOxygenLevel(Entity entity)
{
    if (!registry.oxygen.has(entity))
    {
        return -1;
    }
    auto &oxygen = registry.oxygen.get(entity);
    return oxygen.level;
}

/**
 * @brief sets the oxygen capacity of an entity
 * TODO: confirm if this is necessary since health bars may use this not the player
 *
 * @param capacity - the new capacity of the entity
 * @return true if the capacity is valid, false otherwise
 */
bool setOxygenCapacity(Entity entity, float capacity)
{
    if (!registry.oxygen.has(entity))
    {
        return false;
    }

    auto &oxygen = registry.oxygen.get(entity);
    if (capacity <= 0)
    {
        return false;
    }
    oxygen.capacity = capacity;
    oxygen.level = capacity;
    return true;
}

/**
 * @brief reduces player's oxygen from idle/basic movement, registers death
 * NOTE: Possibly modified for damage over time in the future
 */
void depleteOxygen(Entity entity)
{
    if (registry.oxygen.has(entity))
    {
        auto &oxygen = registry.oxygen.get(entity);
        oxygen.level -= oxygen.rate;

        if (oxygen.level <= 0)
        {
            if (!registry.deathTimers.has(entity))
            {
                registry.deathTimers.emplace(entity);
                oxygen.level = 0;
            }
        }
    }
}

/**
 * @brief reduces oxygen from an affector, registers death
 * @details tentative plan:
 *  - dash costs 30 per 1s
 *  - firing harpoon costs 25
 *  - various standard weapons cost 30-50
 *  - pistol shrimp costs 100
 *  - non-boss damage can cost anywhere between 20-100
 *  - boss damage can cost 100-500
 *
 * @param entity - the entity that is losing oxygen
 * @param affector - the entity that is affecting the player's oxygen
 */
void consumeOxygen(Entity entity, Entity affector)
{
    if (registry.oxygen.has(entity) && registry.damageTouch.has(affector))
    {
        auto &oxygen = registry.oxygen.get(entity);
        auto &damage = registry.damageTouch.get(affector);
        oxygen.level -= damage.damage;

        if (oxygen.level <= 0)
        {
            if (!registry.deathTimers.has(entity))
            {
                registry.deathTimers.emplace(entity);
                oxygen.level = 0;
            }
        }
    }
}

/**
 * @brief refills the oxygen of an entity based on affector
 * @details tentative plan:
 *  - geysers fill 100 per 1s
 *  - each oxygen tank fills 50 per 1s for up to 10s (500 max)
 * TODO: implement after we implement oxygen refilling entities
 *
 * @param entity - the entity that is having its oxygen refilled
 * @param affector - the entity that is refilling the player's oxygen
 */
void refillOxygen(Entity entity, Entity affector)
{
    return;
}