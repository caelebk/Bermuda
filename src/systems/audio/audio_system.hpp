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

/******INSTRUCTIONS FOR ADDING AUDIO*******
 * 1. Add audio to ./data/audio/sound/ or ./data/audio/music
 * 2. Add to SOUND_ASSET_ID + sound_names or MUSIC_ASSET_ID + music_names
 ********************************************/

// order matters, filename
const std::array<std::string, sound_count> sound_names = {
	"blast",
	"glide",
	"slow_heart",
	"fast_heart",
	"hurt",
	"death",
	"flatline",
	"eat",
	"deplete",
	"empty_gun",
	"concussive",
	"torpedo",
	"explosion",
	"shrimp",
	"enemy_death",
	"door",
	"key",
	"net",
	"crate_death",
	"crate_hit",
	"metal_crate_death",
	"metal_crate_hit",
	"lobster_shield",
	"urchin",
	"seahorse",
  "turtle",
	"pressure_plate",
	"boss",
	"cthulhu_intro",
	"cthulhu_angry",
	"cthulhu_shockwave",
	"cthulhu_death",
	"death_screen",
	"end_screen",
	"notification",
	"save",
	"siren",
	"pickup",
	"hitmarker",
	"sharkman",
};

//order matters
const std::array<std::string, music_count> music_names = {
	"intro_music",
	"shark_alert",
	"level2_music",
	"krab_music",
	"level3_music",
	"cthulhu_music",
	"cthulhu2_music",
};

class AudioSystem
{

private:
	std::unordered_map<SOUND_ASSET_ID, Mix_Chunk*> sound_map;
	std::unordered_map<MUSIC_ASSET_ID, Mix_Music*> music_map;

	void init_audio_maps();

public:

    ~AudioSystem();

	void init();
	void step(float elapsed_ms);
};