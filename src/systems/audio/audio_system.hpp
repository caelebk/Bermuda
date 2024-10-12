#pragma once

// internal
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// sounds
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// sound name definitions
#define death_sound "death_sound"
#define eat_sound "eat_sound"
#define blast_sound "blast_sound"

// music name definitions
#define background_music "music"
#define background_music2 "music2"

class AudioSystem
{

private:
	std::unordered_map<std::string, Mix_Chunk*> sound_map;
	std::unordered_map<std::string, Mix_Music*> music_map;

	void init_audio_maps();

public:

    ~AudioSystem();

	void init();
	void step(float elapsed_ms);
};