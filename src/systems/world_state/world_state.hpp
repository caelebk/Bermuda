#pragma once

#include "common.hpp"
#include "level_system.hpp"
#include "physics.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

// Overlay Timer Duration
#define OVERLAY_TIMER_DURATION 5000.f

// create an overlay entity
Entity createOverlay(RenderSystem* renderer);

// enter a game state
void overlayState(TEXTURE_ASSET_ID overlayTextureID);

// check if texture is an overlay texture
bool isOverlayTexture(TEXTURE_ASSET_ID textureID);

// enter a room transition
void roomTransitionState(RenderSystem* renderer, ScreenState& screen,
                         LevelSystem* level,
                         float        elapsed_ms_since_last_update);

// overlay transition states, return true when transition is complete
bool darkenTransitionState(ScreenState& screen,
                           float        elapsed_ms_since_last_update);
bool brightenTransitionState(ScreenState& screen,
                             float        elapsed_ms_since_last_update);

extern bool krab_boss_encountered;
extern bool sharkman_encountered;
extern bool   is_intro;
extern bool   is_start;
extern bool   is_paused;
extern bool   is_krab_cutscene;
extern bool   is_sharkman_cutscene;
extern bool   is_death;
extern bool   is_end;
extern Entity overlay;
extern bool   room_transitioning;
extern Entity rt_entity;