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

  // add to keys
  registry.keys.emplace(redKey);

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

  // add to keys
  registry.keys.emplace(blueKey);

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

  // add to keys
  registry.keys.emplace(yellowKey);

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
      return true;
  }
}

//////////////////////////////////////////////////////////////
// Communications
//////////////////////////////////////////////////////////////

/********************************************************************************
 * @brief creates the communications part of the player HUD
 *
 * @param renderer
 ********************************************************************************/
void createCommunicationHud(RenderSystem* renderer) {
  auto communicationsHud = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(communicationsHud, &mesh);

  // Initialize the position and scale
  auto& communicationsHudPos    = registry.positions.emplace(communicationsHud);
  communicationsHudPos.angle    = 0.f;
  communicationsHudPos.position = COMMUNICATION_HUD_POS;
  communicationsHudPos.scale =
      COMMUNICATION_HUD_SCALE_FACTOR * COMMUNICATION_HUD_BOUNDING_BOX;
  communicationsHudPos.originalScale = communicationsHudPos.scale;

  // Make player HUD
  registry.playerHUD.emplace(communicationsHud);

  // request rendering
  registry.renderRequests.insert(
      communicationsHud,
      {TEXTURE_ASSET_ID::COMMUNUCATIONS, EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});

  requestText(renderer, "--COMMUNICATIONS--", COMMUNICATIONS_TEXT_SCALE,
              COMMUNICATIONS_TEXT_COLOUR, COMMUNICATIONS_TITLE_POS);
}

/********************************************************************************
 * @brief creates new dialogue in communications HUD
 *
 * @param renderer
 * @param line1 - assert length <= 77 to prevent overflow
 * @param line2 - assert length <= 77 to prevent overflow
 ********************************************************************************/
void createDialogue(RenderSystem* renderer, std::string line1,
                    std::string line2) {
  bool isLine1New = true;
  bool isLine2New = true;

  clearOldDialogue(isLine1New, isLine2New, line1, line2);

  assert(line1.length() <= LINE_LENGTH_LIMIT &&
         line2.length() <= LINE_LENGTH_LIMIT);

  if (isLine1New || isLine2New) {
    registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::NOTIFICATION));
  }

  if (isLine1New) {
    Entity lineText1 =
        requestText(renderer, line1, COMMUNICATIONS_TEXT_SCALE,
                    COMMUNICATIONS_TEXT_COLOUR, COMMUNICATIONS_LINE1_POS);
    Communication& communication = registry.communications.emplace(lineText1);
    communication.line = line1;
  }

  // don't create a second entity if not needed
  if (line2 != NO_SECOND_LINE && isLine2New) {
    Entity lineText2 =
        requestText(renderer, line2, COMMUNICATIONS_TEXT_SCALE,
                    COMMUNICATIONS_TEXT_COLOUR, COMMUNICATIONS_LINE2_POS);
    Communication& communication2 = registry.communications.emplace(lineText2);
    communication2.line = line2;
  }
}

/********************************************************************************
 * @brief clear dialogue in communications HUD if it is a new line
 ********************************************************************************/
void clearOldDialogue(bool& isLine1New, bool& isLine2New, std::string line1, std::string line2) {
  for (Entity entity : registry.communications.entities) {
    if (registry.communications.has(entity)) {
      if (registry.communications.get(entity).line.compare(line1) == 0) {
        isLine1New = false;
      } else if (registry.communications.get(entity).line.compare(line2) == 0) {
        isLine2New = false;
      } else {
        registry.remove_all_components_of(entity);
      }
    }
  }
}

//////////////////////////////////////////////////////////////
// Communications - createDialogue(...) prompts
//                  line length <= 77 to prevent overflow
//////////////////////////////////////////////////////////////

void tutorialRoomDialogue(RenderSystem* renderer) {
  createDialogue(renderer, TUTORIAL_LINE1, TUTORIAL_LINE2);
}

void enemyRoomDialogue(RenderSystem* renderer) {
  createDialogue(renderer, ENEMY_LINE1, ENEMY_LINE2);
}

void weaponSwitchingDisabledDialogue(RenderSystem* renderer) {
  createDialogue(renderer, SWITCH_DISABLED_LINE1, SWITCH_DISABLED_LINE2);
}

void multiFireDialogue(RenderSystem* renderer) {
  createDialogue(renderer, MULTI_FIRE_LINE1, MULTI_FIRE_LINE2);
}

void darkRoomDialogue(RenderSystem* renderer) {
  createDialogue(renderer, DARK_ROOM_LINE1, DARK_ROOM_LINE2);
}

void krabBossDialogue(RenderSystem* renderer) {
  createDialogue(renderer, KRAB_BOSS_LINE1, KRAB_BOSS_LINE2);
}

void sharkmanBossDialogue(RenderSystem* renderer) {
  createDialogue(renderer, SHARK_BOSS_LINE1, SHARK_BOSS_LINE2);
}

void keyLockedDialogue(RenderSystem* renderer) {
  createDialogue(renderer, KEY_LOCKED_LINE1, KEY_LOCKED_LINE2);
}
void plateLockedDialogue(RenderSystem* renderer) {
  createDialogue(renderer, PLATE_LOCKED_LINE1, PLATE_LOCKED_LINE1);
}
void bossLockedDialogue(RenderSystem* renderer) {
  createDialogue(renderer, BOSS_LOCKED_LINE1, BOSS_LOCKED_LINE2);
}

void tutorialLockedDialogue(RenderSystem* renderer) {
  createDialogue(renderer, TUTORIAL_LOCKED_LINE1, TUTORIAL_LOCKED_LINE2);
}

void collectNetDialogue(RenderSystem* renderer) {
  createDialogue(renderer, COLLECT_NET_LINE1, COLLECT_NET_LINE2);
}

void collectConcussiveDialogue(RenderSystem* renderer) {
  createDialogue(renderer, COLLECT_CONCUSSIVE_LINE1, COLLECT_CONCUSSIVE_LINE2);
}

void collectTorpedoDialogue(RenderSystem* renderer) { 
  createDialogue(renderer, COLLECT_TORPEDO_LINE1, COLLECT_TORPEDO_LINE2);
}

void collectShrimpDialogue(RenderSystem* renderer) {
  createDialogue(renderer, COLLECT_SHRIMP_LINE1, COLLECT_SHRIMP_LINE2);
}

void overCollectDialogue(RenderSystem* renderer) {
  createDialogue(renderer, OVER_COLLECT_LINE1, OVER_COLLECT_LINE2);
}
