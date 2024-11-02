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
#define INV_POS_Y window_height_px - 45.f
#define INV_SEPARATION_X window_width_px / 6.f + 50.f

#define INV_DIGIT_SCALE_FACTOR vec2(0.30f)
#define INV_DIGIT_BOUNDING_BOX vec2(90.f, 150.f)
#define INV_DIGIT_GAP vec2(5.f, 0.f)

// TODO: Change values based on sprite dimensions
#define INV_KEY_SCALE_FACTOR vec2(1.f)
#define INV_KEY_BOUNDING_BOX vec2(64.f, 26.f)
#define INV_KEY_POS vec2(50.f, INV_POS_Y)

// TODO: Change values based on Andy's sprite dimensions
#define INV_NET_SCALE_FACTOR vec2(1.)
#define INV_NET_BOUNDING_BOX vec2(64.f, 26.f)
#define INV_NET_POS vec2(INV_KEY_POS.x + INV_SEPARATION_X, INV_POS_Y)

// TODO: Change values based on Andy's sprite dimensions
#define INV_CONCUSSIVE_SCALE_FACTOR vec2(1.f)
#define INV_CONCUSSIVE_BOUNDING_BOX vec2(64.f, 26.f)
#define INV_CONCUSSIVE_POS vec2(INV_NET_POS.x + INV_SEPARATION_X, INV_POS_Y)

// TODO: Change values based on Andy's sprite dimensions
#define INV_TORPEDO_SCALE_FACTOR vec2(1.f)
#define INV_TORPEDO_BOUNDING_BOX vec2(64.f, 26.f)
#define INV_TORPEDO_POS vec2(INV_CONCUSSIVE_POS.x + INV_SEPARATION_X, INV_POS_Y)

// TODO: Change values based on Andy's sprite dimensions
#define INV_SHRIMP_SCALE_FACTOR vec2(1.f)
#define INV_SHRIMP_BOUNDING_BOX vec2(64.f, 26.f)
#define INV_SHRIMP_POS vec2(INV_TORPEDO_POS.x + INV_SEPARATION_X, INV_POS_Y)

void createInvHUD(RenderSystem* renderer, Entity& player);
void updateInventoryCounter(RenderSystem* renderer, INVENTORY invType,
                            unsigned int numInvType);
void updateCounterDigits(RenderSystem* renderer, Entity& invTypeIcon,
                         InventoryHUD& invTypeIconHUD, unsigned int numInvType);