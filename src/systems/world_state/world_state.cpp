#include "world_state.hpp"

#include <iostream>

#include "player_hud.hpp"
#include "text_factories.hpp"
#include "tiny_ecs_registry.hpp"

/********************************************************************************
 * @brief create overlay entity
 *
 * @param renderer
 ********************************************************************************/
Entity createOverlay(RenderSystem* renderer) {
  auto overlay = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(overlay, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(overlay);
  position.position  = vec2(window_width_px / 2.f, window_height_px / 2.f);
  position.angle     = 0.f;
  position.scale     = vec2(window_width_px, window_height_px);

  registry.overlays.emplace(overlay);

  return overlay;
}

/********************************************************************************
 * @brief enter an overlay
 ********************************************************************************/
void overlayState(TEXTURE_ASSET_ID overlayTextureID) {
  if (!isOverlayTexture(overlayTextureID)) {
    return;
  }
  if (!registry.renderRequests.has(overlay)) {
    registry.renderRequests.insert(overlay,
                                   {overlayTextureID, EFFECT_ASSET_ID::TEXTURED,
                                    GEOMETRY_BUFFER_ID::SPRITE});
  }
}

/********************************************************************************
 * @brief guard to check if provided texture id belongs to an overlay
 *
 * @param textureID
 ********************************************************************************/
bool isOverlayTexture(TEXTURE_ASSET_ID textureID) {
  const unsigned int num_overlay_textures = 7;

  TEXTURE_ASSET_ID overlay_textures[num_overlay_textures] = {
      TEXTURE_ASSET_ID::INTRO_OVERLAY,    TEXTURE_ASSET_ID::START_OVERLAY,
      TEXTURE_ASSET_ID::PAUSE_OVERLAY,    TEXTURE_ASSET_ID::KRAB_OVERLAY,
      TEXTURE_ASSET_ID::SHARKMAN_OVERLAY, TEXTURE_ASSET_ID::DEATH_OVERLAY,
      TEXTURE_ASSET_ID::END_OVERLAY};

  std::vector<TEXTURE_ASSET_ID> overlayTexturesVector(
      overlay_textures, overlay_textures + num_overlay_textures);

  return find(overlayTexturesVector.begin(), overlayTexturesVector.end(),
              textureID) != overlayTexturesVector.end();
}

/********************************************************************************
 * @brief enter a room transition
 *
 * @param screen
 * @param level
 * @param elapsed_ms_since_last_update
 ********************************************************************************/
void roomTransitionState(RenderSystem* renderer, ScreenState& screen,
                         LevelSystem* level,
                         float        elapsed_ms_since_last_update) {
  if (screen.darken_screen_factor < 1.f &&
      registry.roomTransitions.has(rt_entity)) {
    screen.darken_screen_factor =
        min(1.f, screen.darken_screen_factor +
                     0.001f * elapsed_ms_since_last_update);
  } else if (screen.darken_screen_factor >= 1.f &&
             registry.roomTransitions.has(rt_entity)) {
    RoomTransition& roomTransition = registry.roomTransitions.get(rt_entity);
    level->enter_room(roomTransition.door_connection);
    // show overlays
    if (level->current_room_editor_id == "5" && !krab_boss_encountered) {
      is_krab_cutscene = true;
      krab_boss_encountered = true;
      krabBossDialogue(renderer);
    } else if (level->current_room_editor_id == "10" && !sharkman_encountered) {
      is_sharkman_cutscene = true;
      sharkman_encountered = true;
      SharkmanBossDialogue(renderer);
    } else if (level->current_room_editor_id == "15") {
      is_end = true;
      // TODO: add final boss diaglogue
    }
    registry.remove_all_components_of(rt_entity);
  } else if (screen.darken_screen_factor > 0.f &&
             !registry.roomTransitions.has(rt_entity)) {
    screen.darken_screen_factor =
        max(0.f, screen.darken_screen_factor -
                     0.001f * elapsed_ms_since_last_update);
  } else if (screen.darken_screen_factor <= 0.f &&
             !registry.roomTransitions.has(rt_entity)) {
    room_transitioning = false;
  }
}

/********************************************************************************
 * @brief darken screen transition
 *
 * @param screen
 * @param elapsed_ms_since_last_update
 ********************************************************************************/
bool darkenTransitionState(ScreenState& screen,
                           float        elapsed_ms_since_last_update) {
  if (screen.darken_screen_factor < 1.f) {
    screen.darken_screen_factor =
        min(1.f, screen.darken_screen_factor +
                     0.001f * elapsed_ms_since_last_update);
    return false;
  } else {
    screen.darken_screen_factor = 1.f;
    return true;
  }
}

/********************************************************************************
 * @brief brighten screen transition
 *
 * @param screen
 * @param elapsed_ms_since_last_update
 ********************************************************************************/
bool brightenTransitionState(ScreenState& screen,
                             float        elapsed_ms_since_last_update) {
  if (screen.darken_screen_factor > 0.f) {
    screen.darken_screen_factor =
        max(0.f, screen.darken_screen_factor -
                     0.001f * elapsed_ms_since_last_update);
    return false;
  } else {
    screen.darken_screen_factor = 0.f;
    return true;
  }
}