#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// request text on the screen
Entity requestText(RenderSystem *renderer, std::string text, float textScale, vec3 textColor, vec2 textPosition);

// display save status after attempting save in pause menu
void displaySaveStatus(RenderSystem* renderer, bool isSaveSuccessful);

// clear save status display
void clearSaveStatus();