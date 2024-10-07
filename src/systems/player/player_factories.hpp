#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

#include "abilities.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "tiny_ecs.hpp"



// the player
Entity createPlayer(RenderSystem *renderer, vec2 pos);
