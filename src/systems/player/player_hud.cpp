#include "player_hud.hpp"

#include <iostream>

#include "player_factories.hpp"
#include "text_factories.hpp"
#include "tiny_ecs_registry.hpp"

//////////////////////////////////////////////////////////////
// Inventory
//////////////////////////////////////////////////////////////
/********************************************************************************
 * @brief creates the inventory part of the player HUD
 *
 * @param renderer
 ********************************************************************************/
void createInventoryHud(RenderSystem* renderer) {
  if (!registry.inventory.has(player)) {
    return;
  }

  auto inventoryHud = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(inventoryHud, &mesh);

  // Initialize the position and scale
  auto& inventoryHudPos    = registry.positions.emplace(inventoryHud);
  inventoryHudPos.angle    = 0.f;
  inventoryHudPos.position = INVENTORY_HUD_POS;
  inventoryHudPos.scale =
      INVENTORY_HUD_SCALE_FACTOR * INVENTORY_HUD_BOUNDING_BOX;
  inventoryHudPos.originalScale = inventoryHudPos.scale;

  // Make player HUD
  registry.playerHUD.emplace(inventoryHud);

  // request rendering
  registry.renderRequests.insert(
      inventoryHud, {TEXTURE_ASSET_ID::INVENTORY, EFFECT_ASSET_ID::TEXTURED,
                     GEOMETRY_BUFFER_ID::SPRITE});

  // PLAYER PROJECTILES

  // harpoon counter
  createHarpoonCounter(renderer);

  // net counter
  Entity netCounterText = requestText(
      renderer, getInvCountString(INVENTORY::NET), INVENTORY_COUNTER_TEXT_SCALE,
      UNSELECTED_COUNTER_TEXT_COLOUR, NET_COUNTER_POS);
  InventoryCounter& netCounter =
      registry.inventoryCounters.emplace(netCounterText);
  netCounter.inventoryType = INVENTORY::NET;

  // concussive counter
  Entity concussiveCounterText =
      requestText(renderer, getInvCountString(INVENTORY::CONCUSSIVE),
                  INVENTORY_COUNTER_TEXT_SCALE, UNSELECTED_COUNTER_TEXT_COLOUR,
                  CONCUSSIVE_COUNTER_POS);
  InventoryCounter& concussiveCounter =
      registry.inventoryCounters.emplace(concussiveCounterText);
  concussiveCounter.inventoryType = INVENTORY::CONCUSSIVE;

  // torpedo counter
  Entity torpedoCounterText =
      requestText(renderer, getInvCountString(INVENTORY::TORPEDO),
                  INVENTORY_COUNTER_TEXT_SCALE, UNSELECTED_COUNTER_TEXT_COLOUR,
                  TORPEDO_COUNTER_POS);
  InventoryCounter& torpedoCounter =
      registry.inventoryCounters.emplace(torpedoCounterText);
  torpedoCounter.inventoryType = INVENTORY::TORPEDO;

  // shrimp counter
  Entity shrimpCounterText =
      requestText(renderer, getInvCountString(INVENTORY::SHRIMP),
                  INVENTORY_COUNTER_TEXT_SCALE, UNSELECTED_COUNTER_TEXT_COLOUR,
                  SHRIMP_COUNTER_POS);
  InventoryCounter& shrimpCounter =
      registry.inventoryCounters.emplace(shrimpCounterText);
  shrimpCounter.inventoryType = INVENTORY::SHRIMP;
}

/********************************************************************************
 * @brief create harpoon counter display
 *
 * @param renderer
 ********************************************************************************/
void createHarpoonCounter(RenderSystem* renderer) {
  // harpoon counter (special case - infinity)
  auto harpoonCounter = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(harpoonCounter, &mesh);

  // Initialize the position and scale
  auto& harpoonCounterPos    = registry.positions.emplace(harpoonCounter);
  harpoonCounterPos.angle    = 0.f;
  harpoonCounterPos.position = HARPOON_COUNTER_POS;
  harpoonCounterPos.scale =
      HARPOON_COUNTER_SCALE_FACTOR * HARPOON_COUNTER_BOUNDING_BOX;
  harpoonCounterPos.originalScale = harpoonCounterPos.scale;

  // Make player HUD
  registry.playerHUD.emplace(harpoonCounter);

  // give colour
  registry.colors.insert(harpoonCounter, SELECTED_COUNTER_TEXT_COLOUR);

  // inventory type
  registry.inventoryCounters.emplace(harpoonCounter);
  registry.inventoryCounters.get(harpoonCounter).inventoryType =
      INVENTORY::HARPOON;

  // request rendering
  registry.renderRequests.insert(
      harpoonCounter, {TEXTURE_ASSET_ID::HARPOON_COUNTER,
                       EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
}

/********************************************************************************
 * @brief update an inventory counter display
 *
 * @param renderer
 * @param inventoryType - which inventory type is being updated
 ********************************************************************************/
void updateInventoryCounter(RenderSystem* renderer, INVENTORY inventoryType) {
  if (!registry.inventory.has(player)) {
    return;
  }
  for (Entity& inventoryCounterText : registry.inventoryCounters.entities) {
    if (registry.inventoryCounters.get(inventoryCounterText).inventoryType ==
            inventoryType &&
        registry.textRequests.has(inventoryCounterText)) {
      registry.textRequests.get(inventoryCounterText).text =
          getInvCountString(inventoryType);
    }
  }
}

/********************************************************************************
 * @brief gets a string for the current count of an inventory type
 *
 * @param inventoryType
 *
 * @note guard for player having inventory in updateInventoryCounter(...)
 ********************************************************************************/
std::string getInvCountString(INVENTORY inventoryType) {
  Inventory&   playerInventory = registry.inventory.get(player);
  unsigned int invCount        = 0;
  switch (inventoryType) {
    case INVENTORY::NET:
      invCount = playerInventory.nets;
      break;

    case INVENTORY::CONCUSSIVE:
      invCount = playerInventory.concussors;
      break;

    case INVENTORY::TORPEDO:
      invCount = playerInventory.torpedos;
      break;

    case INVENTORY::SHRIMP:
      invCount = playerInventory.shrimp;
      break;

    default:
      break;
  }

  std::string invCountString = std::to_string(invCount);
  while (invCountString.length() < 3) {
    invCountString.insert(0, "0");
  }

  // ensure cap is not violated
  assert(invCountString.length() <= 3);

  return invCountString;
}

/********************************************************************************
 * @brief update an inventory counter display
 *
 * @param inventoryType - which inventory type is being updated
 *
 * @note all counters have a color component
 * @note changes color component instead of font color (visually no difference)
 *       (avoids having to stick an entity into yet another component)
 ********************************************************************************/
void changeSelectedCounterColour(INVENTORY inventoryType) {
  if (inventoryType == INVENTORY::RED_KEY ||
      inventoryType == INVENTORY::BLUE_KEY ||
      inventoryType == INVENTORY::YELLOW_KEY) {
    return;
  }
  for (Entity& entity : registry.inventoryCounters.entities) {
    InventoryCounter& inventoryCounter = registry.inventoryCounters.get(entity);

    if (inventoryCounter.inventoryType != inventoryType) {
      registry.colors.get(entity) = UNSELECTED_COUNTER_TEXT_COLOUR;
    } else {
      registry.colors.get(entity) = SELECTED_COUNTER_TEXT_COLOUR;
    }
  }
}

/********************************************************************************
 * @brief render red collected key and update inventory
 *
 * @param renderer
 ********************************************************************************/
void collectRedKey(RenderSystem* renderer) {
  if (!registry.inventory.has(player)) {
    return;
  }
  Inventory& playerInventory = registry.inventory.get(player);
  Entity     redKey          = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(redKey, &mesh);

  // Set position values
  Position& redKeyPos     = registry.positions.emplace(redKey);
  redKeyPos.angle         = 0.f;
  redKeyPos.position      = RED_KEY_POS;
  redKeyPos.scale         = RED_KEY_SCALE_FACTOR * RED_KEY_BOUNDING_BOX;
  redKeyPos.originalScale = redKeyPos.scale;

  // make playerHUD
  registry.playerHUD.emplace(redKey);

  // render request
  playerInventory.redKey = true;
  registry.renderRequests.insert(
      redKey, {TEXTURE_ASSET_ID::RED_KEY, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});
}

/********************************************************************************
 * @brief render blue collected key and update inventory
 *
 * @param renderer
 ********************************************************************************/
void collectBlueKey(RenderSystem* renderer) {
  if (!registry.inventory.has(player)) {
    return;
  }
  Inventory& playerInventory = registry.inventory.get(player);
  Entity     blueKey         = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(blueKey, &mesh);

  // Set position values
  Position& blueKeyPos     = registry.positions.emplace(blueKey);
  blueKeyPos.angle         = 0.f;
  blueKeyPos.position      = BLUE_KEY_POS;
  blueKeyPos.scale         = BLUE_KEY_SCALE_FACTOR * BLUE_KEY_BOUNDING_BOX;
  blueKeyPos.originalScale = blueKeyPos.scale;

  // make playerHUD
  registry.playerHUD.emplace(blueKey);

  // render request
  playerInventory.blueKey = true;
  registry.renderRequests.insert(
      blueKey, {TEXTURE_ASSET_ID::BLUE_KEY, EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE});
}

/********************************************************************************
 * @brief render yellow collected key and update inventory
 *
 * @param renderer
 ********************************************************************************/
void collectYellowKey(RenderSystem* renderer) {
  if (!registry.inventory.has(player)) {
    return;
  }
  Inventory& playerInventory = registry.inventory.get(player);
  Entity     yellowKey       = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(yellowKey, &mesh);

  // Set position values
  Position& yellowKeyPos = registry.positions.emplace(yellowKey);
  yellowKeyPos.angle     = 0.f;
  yellowKeyPos.position  = YELLOW_KEY_POS;
  yellowKeyPos.scale     = YELLOW_KEY_SCALE_FACTOR * YELLOW_KEY_BOUNDING_BOX;
  yellowKeyPos.originalScale = yellowKeyPos.scale;

  // make playerHUD
  registry.playerHUD.emplace(yellowKey);

  // render request
  playerInventory.yellowKey = true;
  registry.renderRequests.insert(
      yellowKey, {TEXTURE_ASSET_ID::YELLOW_KEY, EFFECT_ASSET_ID::TEXTURED,
                  GEOMETRY_BUFFER_ID::SPRITE});
}

/********************************************************************************
 * @brief is the corresponding key currently collected
 *
 * @param keyType
 ********************************************************************************/
bool isKeyCollected(INVENTORY keyType) {
  if (!registry.inventory.has(player)) {
    printf("Player has no inventory\n");
    return false;
  }
  Inventory& playerInventory = registry.inventory.get(player);
  switch (keyType) {
    case INVENTORY::RED_KEY:
      return playerInventory.redKey;

    case INVENTORY::BLUE_KEY:
      return playerInventory.blueKey;

    case INVENTORY::YELLOW_KEY:
      return playerInventory.yellowKey;

    default:
      printf("Not a valid key type\n");
      return false;
  }
}