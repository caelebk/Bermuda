#include "text_factories.hpp"

#include "tiny_ecs_registry.hpp"

Entity requestText(RenderSystem* renderer, std::string text, float textScale,
                   vec3 textColor, vec2 textPosition) {
  // Reserve en entity
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Initialize the position
  auto& position    = registry.positions.emplace(entity);
  position.angle    = 0.f;
  position.position = textPosition;
  position.scale    = vec2(1.f);

  // choose color
  auto& color = registry.colors.emplace(entity);
  color       = textColor;

  // make text request
  auto& textRequest     = registry.textRequests.emplace(entity);
  textRequest.text      = text;
  textRequest.textScale = textScale;

  return entity;
}

void displaySaveStatus(RenderSystem* renderer, bool isSaveSuccessful) {
  clearSaveStatus();
  std::string saveStatusText = isSaveSuccessful
                                   ? "SUCCESS: Your Game Has Been Saved"
                                   : "ERROR: Save Was Unsuccessful";
  vec3        saveStatusColour =
      isSaveSuccessful ? vec3(0.f, 0.8f, 0.3f) : vec3(7.f, 0.3f, 0.3f);

  Entity saveStatus =
      requestText(renderer, saveStatusText, 0.34f, saveStatusColour,
                  vec2(window_width_px / 2 + 25.f, window_height_px - 80.f));

  registry.saveStatuses.emplace(saveStatus);

  if (isSaveSuccessful) {
    registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::SAVE));
  }
}

void clearSaveStatus() {
  while (registry.saveStatuses.entities.size() > 0) {
    registry.remove_all_components_of(registry.saveStatuses.entities.back());
  }
}
