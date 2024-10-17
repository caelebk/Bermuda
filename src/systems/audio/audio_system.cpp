#include "audio_system.hpp"

/******INSTRUCTIONS FOR ADDING AUDIO*******
 * 1. Add audio to ./data/audio/
 * 2. Define a name for the audio in the header
 * 3. Add a mapping to init_audio_maps()
 ********************************************/

AudioSystem::~AudioSystem() {
  for (auto it = music_map.begin(); it != music_map.end(); ++it) {
    Mix_FreeMusic(music_map[it->first]);
  }

  for (auto it = sound_map.begin(); it != sound_map.end(); ++it) {
    Mix_FreeChunk(sound_map[it->first]);
  }

  Mix_CloseAudio();
}

/*FORMAT for adding mappings for sound/music:

   Music:
   music_map[<defined_music_name>] =
   Mix_LoadMUS(audio_path(<filename>).c_str());

   Sound:
   sound_map[<defined_sound_name>] =
   Mix_LoadWAV(audio_path(<filename>).c_str());

*/
void AudioSystem::init_audio_maps() {
  music_map[background_music] = Mix_LoadMUS(audio_path("music.wav").c_str());

  sound_map[death_sound] = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
  sound_map[eat_sound]   = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());
  sound_map[blast_sound] = Mix_LoadWAV(audio_path("blast_sound.wav").c_str());
  sound_map[dash_sound]  = Mix_LoadWAV(audio_path("dash.wav").c_str());
  sound_map[flat_line_sound] = Mix_LoadWAV(audio_path("flatline.wav").c_str());
  sound_map[hurt_sound]    = Mix_LoadWAV(audio_path("hurt_sound.wav").c_str());
  sound_map[deplete_audio] = Mix_LoadWAV(audio_path("deplete.wav").c_str());
  sound_map[fast_heart_audio] =
      Mix_LoadWAV(audio_path("fastheart.wav").c_str());
  sound_map[slow_heart_audio] =
      Mix_LoadWAV(audio_path("slowheart.wav").c_str());
}

void AudioSystem::init() {
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Failed to initialize SDL Audio");
    return;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
    fprintf(stderr, "Failed to open audio device");
    return;
  }

  init_audio_maps();

  for (auto it = music_map.begin(); it != music_map.end(); ++it) {
    if (music_map[it->first] == nullptr) {
      fprintf(stderr, "Failed to load music: %s\n", it->first.c_str());
    }
  }

  for (auto it = sound_map.begin(); it != sound_map.end(); ++it) {
    if (sound_map[it->first] == nullptr) {
      fprintf(stderr, "Failed to load sound: %s\n", it->first.c_str());
    }
  }

  // Initial background music
  Mix_PlayMusic(music_map[background_music], -1);
  fprintf(stderr, "Loaded music\n");
}

void AudioSystem::step(float elapsed_ms) {
  for (Entity entity : registry.sounds.entities) {
    Sound sound   = registry.sounds.get(entity);
    int   channel = Mix_PlayChannelTimed(-1, sound_map[sound.name], 0, 4000);
    Mix_Volume(channel, 64);
    registry.sounds.remove(entity);
  }

  for (Entity entity : registry.musics.entities) {
    Music music = registry.musics.get(entity);
    // This replaces the current looping background music.
    Mix_PlayMusic(music_map[music.name], -1);
    registry.musics.remove(entity);
  }
}