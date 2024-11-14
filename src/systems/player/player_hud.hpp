#pragma once

#include "abilities.hpp"
#include "common.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

//////////////////////////////////////////////////////////////
// Inventory
//////////////////////////////////////////////////////////////

#define INVENTORY_HUD_POS vec2(320.f, window_height_px - 49.f)
#define INVENTORY_HUD_SCALE_FACTOR vec2(1.f)
#define INVENTORY_HUD_BOUNDING_BOX vec2(629.f, 90.f)

#define HARPOON_COUNTER_POS vec2(57.f, window_height_px - 18.f)
#define HARPOON_COUNTER_SCALE_FACTOR vec2(0.04f)
#define HARPOON_COUNTER_BOUNDING_BOX vec2(876.f, 408.f)

#define INVENTORY_COUNTER_TEXT_SCALE 0.20f
#define SELECTED_COUNTER_TEXT_COLOUR vec3(0.1f, 0.7f, 0.1f)
#define UNSELECTED_COUNTER_TEXT_COLOUR vec3(0.6f, 0.9f, 0.9f)

#define NET_COUNTER_POS vec2(123.f, window_height_px - 11.f)
#define CONCUSSIVE_COUNTER_POS vec2(200.f, window_height_px - 11.f)
#define TORPEDO_COUNTER_POS vec2(279.f, window_height_px - 11.f)
#define SHRIMP_COUNTER_POS vec2(358.f, window_height_px - 11.f)

#define RED_KEY_POS vec2(439.f, window_height_px - 48.f)
#define RED_KEY_SCALE_FACTOR vec2(0.25f)
#define RED_KEY_BOUNDING_BOX vec2(120.f, 296.f)

#define BLUE_KEY_POS vec2(518.f, window_height_px - 48.f)
#define BLUE_KEY_SCALE_FACTOR vec2(0.25f)
#define BLUE_KEY_BOUNDING_BOX vec2(120.f, 296.f)

#define YELLOW_KEY_POS vec2(597.f, window_height_px - 48.f)
#define YELLOW_KEY_SCALE_FACTOR vec2(0.25f)
#define YELLOW_KEY_BOUNDING_BOX vec2(120.f, 296.f)

// create the inventory part of the player HUD
void createInventoryHud(RenderSystem* renderer);

// create harpoon counter
void createHarpoonCounter(RenderSystem* renderer);

// update inventory counter displays
void updateInventoryCounter(RenderSystem* renderer, INVENTORY inventoryType);

// get counter string
std::string getInvCountString(INVENTORY inventoryType);

// change selected inventory text colour
void changeSelectedCounterColour(INVENTORY inventoryType);

// COLLECT KEY FUNCTIONS MADE AS SEPARATE FUNCTIONS
// POSITION AND RENDER REQUEST BOTH DIFFER BETWEEN KEYS
// collect red key
void collectRedKey(RenderSystem* renderer);

// collect blue key
void collectBlueKey(RenderSystem* renderer);

// collect yellow key
void collectYellowKey(RenderSystem* renderer);

// is the corresponding key currently collected
bool isKeyCollected(INVENTORY keyType);

//////////////////////////////////////////////////////////////
// Dialogue
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////

extern Entity player;
