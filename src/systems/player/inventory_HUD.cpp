#include "inventory_HUD.hpp"

#include <iostream>

#include "tiny_ecs_registry.hpp"
#include "player_factories.hpp"

// texture assets for inventory count
TEXTURE_ASSET_ID digitTextures[10] = {
    TEXTURE_ASSET_ID::ZERO,  TEXTURE_ASSET_ID::ONE,   TEXTURE_ASSET_ID::TWO,
    TEXTURE_ASSET_ID::THREE, TEXTURE_ASSET_ID::FOUR,  TEXTURE_ASSET_ID::FIVE,
    TEXTURE_ASSET_ID::SIX,   TEXTURE_ASSET_ID::SEVEN, TEXTURE_ASSET_ID::EIGHT,
    TEXTURE_ASSET_ID::NINE};

/********************************************************************************
 * @brief creates the inventory part of the player HUD
 *
 * @param renderer
 * @param player - provides player inventory data
 ********************************************************************************/
void createInvHUD(RenderSystem* renderer, Entity& player) {
  if (!registry.inventory.has(player)) {
    return;
  }
  Inventory& player_inventory = registry.inventory.get(player);

  auto keyIcon        = Entity();
  auto netIcon        = Entity();
  auto concussiveIcon = Entity();
  auto torpedoIcon    = Entity();
  auto shrimpIcon     = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(keyIcon, &mesh);
  registry.meshPtrs.emplace(netIcon, &mesh);
  registry.meshPtrs.emplace(concussiveIcon, &mesh);
  registry.meshPtrs.emplace(torpedoIcon, &mesh);
  registry.meshPtrs.emplace(shrimpIcon, &mesh);

  // Initialize the position, scale, and physics components
  auto& keyIconPos         = registry.positions.emplace(keyIcon);
  keyIconPos.angle         = 0.f;
  keyIconPos.position      = INV_KEY_POS;
  keyIconPos.scale         = INV_KEY_SCALE_FACTOR * INV_KEY_BOUNDING_BOX;
  keyIconPos.originalScale = keyIconPos.scale;

  auto& netIconPos         = registry.positions.emplace(netIcon);
  netIconPos.angle         = 0.f;
  netIconPos.position      = INV_NET_POS;
  netIconPos.scale         = INV_NET_SCALE_FACTOR * NET_BOUNDING_BOX;
  netIconPos.originalScale = netIconPos.scale;

  auto& concussiveIconPos    = registry.positions.emplace(concussiveIcon);
  concussiveIconPos.angle    = 0.f;
  concussiveIconPos.position = INV_CONCUSSIVE_POS;
  concussiveIconPos.scale =
      INV_CONCUSSIVE_SCALE_FACTOR * CONCUSSIVE_BOUNDING_BOX;
  concussiveIconPos.originalScale = concussiveIconPos.scale;

  auto& torpedoIconPos    = registry.positions.emplace(torpedoIcon);
  torpedoIconPos.angle    = 0.f;
  torpedoIconPos.position = INV_TORPEDO_POS;
  torpedoIconPos.scale    = INV_TORPEDO_SCALE_FACTOR * TORPEDO_BOUNDING_BOX;
  torpedoIconPos.originalScale = torpedoIconPos.scale;

  auto& shrimpIconPos    = registry.positions.emplace(shrimpIcon);
  shrimpIconPos.angle    = 0.f;
  shrimpIconPos.position = INV_SHRIMP_POS;
  shrimpIconPos.scale    = INV_SHRIMP_SCALE_FACTOR * SHRIMP_BOUNDING_BOX;
  shrimpIconPos.originalScale = shrimpIconPos.scale;

  registry.playerHUD.emplace(keyIcon);
  registry.playerHUD.emplace(netIcon);
  registry.playerHUD.emplace(concussiveIcon);
  registry.playerHUD.emplace(torpedoIcon);
  registry.playerHUD.emplace(shrimpIcon);

  InventoryHUD& keyHUD = registry.invHUD.emplace(keyIcon);
  keyHUD.inv_type      = INVENTORY::KEY;

  InventoryHUD& netHUD = registry.invHUD.emplace(netIcon);
  netHUD.inv_type      = INVENTORY::NET;

  InventoryHUD& concussiveHUD = registry.invHUD.emplace(concussiveIcon);
  concussiveHUD.inv_type      = INVENTORY::CONCUSSIVE;

  InventoryHUD& torpedoHUD = registry.invHUD.emplace(torpedoIcon);
  torpedoHUD.inv_type      = INVENTORY::TORPEDO;

  InventoryHUD& shrimpHUD = registry.invHUD.emplace(shrimpIcon);
  shrimpHUD.inv_type      = INVENTORY::SHRIMP;

  registry.renderRequests.insert(
      keyIcon, {TEXTURE_ASSET_ID::KEY, EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      netIcon, {TEXTURE_ASSET_ID::NET, EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE});
  registry.renderRequests.insert(
      concussiveIcon, {TEXTURE_ASSET_ID::CONCUSSIVE, EFFECT_ASSET_ID::TEXTURED,
                       GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      torpedoIcon, {TEXTURE_ASSET_ID::TORPEDO, EFFECT_ASSET_ID::TEXTURED,
                    GEOMETRY_BUFFER_ID::SPRITE});
  registry.renderRequests.insert(
      shrimpIcon, {TEXTURE_ASSET_ID::SHRIMP, EFFECT_ASSET_ID::TEXTURED,
                   GEOMETRY_BUFFER_ID::SPRITE});

  updateInventoryCounter(renderer, INVENTORY::KEY, player_inventory.keys);
  updateInventoryCounter(renderer, INVENTORY::NET, player_inventory.nets);
  updateInventoryCounter(renderer, INVENTORY::CONCUSSIVE,
                         player_inventory.concussors);
  updateInventoryCounter(renderer, INVENTORY::TORPEDO,
                         player_inventory.torpedos);
  updateInventoryCounter(renderer, INVENTORY::SHRIMP, player_inventory.shrimp);
}

/********************************************************************************
 * @brief renders the count for inventory icons
 *
 * @param renderer
 * @param invType: the type of inventory item
 * @param numInvType: the number of that item in inventory
 ********************************************************************************/
void updateInventoryCounter(RenderSystem* renderer, INVENTORY invType,
                            unsigned int numInvType) {
  for (Entity& invTypeIcon : registry.invHUD.entities) {
    InventoryHUD& invTypeIconHUD = registry.invHUD.get(invTypeIcon);
    if (invType == invTypeIconHUD.inv_type) {
      updateCounterDigits(renderer, invTypeIcon, invTypeIconHUD, numInvType);
    }
  }
}

/********************************************************************************
 * @brief renders the digits for inventory icon counts
 *
 * @param renderer
 * @param invTypeIcon: the entity for the inventory icon on the HUD
 * @param invTypeIconHUD: HUD component for invTypeIcon
 * @param numInvType: the number of that item in inventory
 ********************************************************************************/
void updateCounterDigits(RenderSystem* renderer, Entity& invTypeIcon,
                         InventoryHUD& invTypeIconHUD,
                         unsigned int  numInvType) {
  // remove components of old digits
  for (const auto& digitKeyValue : invTypeIconHUD.count_digits) {
    registry.renderRequests.remove(digitKeyValue.second);
    registry.positions.remove(digitKeyValue.second);
  }

  // position data of inventory icon
  Position& invTypeIconPos = registry.positions.get(invTypeIcon);

  // convert digits to an iterable string
  std::string numInvTypeDigits = std::to_string(numInvType);

  for (std::string::size_type i = 0; i < numInvTypeDigits.size(); ++i) {
    // converts digit char to unsigned int
    // https://stackoverflow.com/questions/5029840/convert-char-to-int-in-c-and-c
    unsigned int numInvTypeDigit = numInvTypeDigits[i] - '0';
    auto         digitEntity     = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(digitEntity, &mesh);

    // Initialize the position, scale, and physics components for digit
    Position& digitEntityPos = registry.positions.emplace(digitEntity);
    digitEntityPos.angle     = 0.f;
    digitEntityPos.scale     = INV_DIGIT_SCALE_FACTOR * INV_DIGIT_BOUNDING_BOX;
    if (i > 0) {
      auto prevDigitPos =
          registry.positions.get(invTypeIconHUD.count_digits[i - 1]).position;
      digitEntityPos.position =
          prevDigitPos + vec2(digitEntityPos.scale.x, 0.f) + INV_DIGIT_GAP;
    } else {
      digitEntityPos.position =
          invTypeIconPos.position + vec2(invTypeIconPos.scale.x / 2.f, 0.f) +
          vec2(digitEntityPos.scale.x / 2.f, 0.f) + INV_ICON_DIGIT_GAP;
    }
    digitEntityPos.originalScale = digitEntityPos.scale;

    // render with HUD
    registry.playerHUD.emplace(digitEntity);

    // render request
    registry.renderRequests.insert(
        digitEntity, {digitTextures[numInvTypeDigit], EFFECT_ASSET_ID::TEXTURED,
                      GEOMETRY_BUFFER_ID::SPRITE});

    // place digit associated icon
    invTypeIconHUD.count_digits[i] = digitEntity;
  }
}