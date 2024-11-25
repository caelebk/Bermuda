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
// Communications
//////////////////////////////////////////////////////////////

#define COMMUNICATION_HUD_POS vec2(window_width_px - 320.f, window_height_px - 49.f)
#define COMMUNICATION_HUD_SCALE_FACTOR vec2(1.f)
#define COMMUNICATION_HUD_BOUNDING_BOX vec2(629.f, 90.f)

#define COMMUNICATIONS_TEXT_COLOUR vec3(0.6f, 0.9f, 0.9f)

#define COMMUNICATIONS_TEXT_SCALE 0.16f
#define COMMUNICATIONS_TITLE_POS vec2(window_width_px - 625.f, window_height_px - 69.f)

#define LINE_LENGTH_LIMIT (size_t)77
#define COMMUNICATIONS_LINE1_POS vec2(window_width_px - 625.f, window_height_px - 43.f)
#define COMMUNICATIONS_LINE2_POS vec2(window_width_px - 625.f, window_height_px - 23.f)

void createCommunicationHud(RenderSystem* renderer);

void createDialogue(RenderSystem* renderer, std::string line1, std::string line2);

void clearOldDialogue(bool& isLine1New, bool& isLine2New, std::string line1, std::string line2);

//////////////////////////////////////////////////////////////
// Communications - createDialogue(...) prompts
//                  line length <= 77 to prevent overflow
//////////////////////////////////////////////////////////////

#define NO_SECOND_LINE (std::string) ""

#define TUTORIAL_LINE1 "The information in these carvings might be useful."
#define TUTORIAL_LINE2 "Hopefully HQ gets these transmissions eventually, there's no signal here."
void tutorialRoomDialogue(RenderSystem* renderer);

#define ENEMY_LINE1 "No signs of the crew or our submarine here."
#define ENEMY_LINE2 NO_SECOND_LINE
void enemyRoomDialogue(RenderSystem* renderer);

#define SWITCH_DISABLED_LINE1 "Interesting... Groundbreaking even..."
#define SWITCH_DISABLED_LINE2 "HQ, I can use weapons I have available on-hand, but not imaginary ones..."
void weaponSwitchingDisabledDialogue(RenderSystem* renderer);

#define MULTI_FIRE_LINE1 "It takes time to reload a gun underwater, around the time my last shot hits."
#define MULTI_FIRE_LINE2 "On the other hand, if I have different guns already reloaded..."
void multiFireDialogue(RenderSystem* renderer);

#define COLLECT_NET_LINE1 "These nets should stun an enemy for awhile."
#define COLLECT_NET_LINE2 NO_SECOND_LINE
void collectNetDialogue(RenderSystem* renderer);

#define COLLECT_CONCUSSIVE_LINE1 "These concussives should push back several enemies for a moment."
#define COLLECT_CONCUSSIVE_LINE2 NO_SECOND_LINE
void collectConcussiveDialogue(RenderSystem* renderer);

#define COLLECT_TORPEDO_LINE1 "These torpedoes should do damage in an area on impact."
#define COLLECT_TORPEDO_LINE2 NO_SECOND_LINE
void collectTorpedoDialogue(RenderSystem* renderer);

#define COLLECT_SHRIMP_LINE1 "There is a lot of power packed in this pistol shrimp."
#define COLLECT_SHRIMP_LINE2 "Using it should eviscerate almost anything in front of me."
void collectShrimpDialogue(RenderSystem* renderer);

#define OVER_COLLECT_LINE1 "I don't need that many... I can't carry that many..."
#define OVER_COLLECT_LINE2 "I don't even know how I acquired that many..."
void overCollectDialogue(RenderSystem* renderer);

#define DARK_ROOM_LINE1 "It's dark in this part of the caves."
#define DARK_ROOM_LINE2 "It will be difficult to see without lugging this light source around."
void darkRoomDialogue(RenderSystem* renderer);       // TODO: call on enter dark room

#define KRAB_BOSS_LINE1 "That is not the shadow from before, but those are pieces of our submarine."
#define KRAB_BOSS_LINE2 "Better not get backed into a corner, this crab is !@#$%&* HUGE."
void krabBossDialogue(RenderSystem* renderer); 

#define SHARK_BOSS_LINE1 "That shark is !@#$%&* FAST, and those limbs belong to some of the crew..."
#define SHARK_BOSS_LINE2 "I wonder how hard it'll be for it to stop if it's charging straight at me..."
void sharkmanBossDialogue(RenderSystem* renderer); 

#define KEY_LOCKED_LINE1 "This door isn't open."
#define KEY_LOCKED_LINE2 "It appears I need to find a matching key."
void keyLockedDialogue(RenderSystem* renderer);

#define PLATE_LOCKED_LINE1 "This door isn't open."
#define PLATE_LOCKED_LINE2 "Maybe the pressure plate I saw earlier will open the door."
void plateLockedDialogue(RenderSystem* renderer);    // TODO: call on door collide

#define BOSS_LOCKED_LINE1 "This door isn't open."
#define BOSS_LOCKED_LINE2 "Maybe I can squeeze through the bars, but not while fighting this thing."
void bossLockedDialogue(RenderSystem* renderer);     // TODO: change dialogue when door render updates are implemented

#define TUTORIAL_LOCKED_LINE1 "This door isn't open."
#define TUTORIAL_LOCKED_LINE2 "Maybe I can squeeze through the bars, but first let me kill this jellyfish."
void tutorialLockedDialogue(RenderSystem* renderer); // TODO: change dialogue when door render updates are implemented

//////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////

extern Entity player;
