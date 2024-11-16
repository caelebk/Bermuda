#include "audio_system.hpp"

AudioSystem::~AudioSystem() {
  for (auto it = music_map.begin(); it != music_map.end(); ++it) {
    Mix_FreeMusic(music_map[it->first]);
  }

  for (auto it = sound_map.begin(); it != sound_map.end(); ++it) {
    Mix_FreeChunk(sound_map[it->first]);
  }

  Mix_CloseAudio();
}

void AudioSystem::init_audio_maps() {  
  for (uint i = 0; i < music_count; ++i) {
    const std::string path = music_path(music_names[i] + ".wav");
    music_map[static_cast<MUSIC_ASSET_ID>(i)] = Mix_LoadMUS(path.c_str());
  }

  for (uint i = 0; i < sound_count; ++i) {
    const std::string path = sound_path(sound_names[i] + ".wav");
    sound_map[static_cast<SOUND_ASSET_ID>(i)] = Mix_LoadWAV(path.c_str());
  }
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

  int num_channels = 16;
  Mix_AllocateChannels(num_channels);

  init_audio_maps();

  for (auto it = music_map.begin(); it != music_map.end(); ++it) {
    if (music_map[it->first] == nullptr) {
      fprintf(stderr, "Failed to load music: %s\n", music_names[static_cast<int>(it->first)].c_str());
    }
  }

  for (auto it = sound_map.begin(); it != sound_map.end(); ++it) {
    if (sound_map[it->first] == nullptr) {
      fprintf(stderr, "Failed to load sound: %s\n", sound_names[static_cast<int>(it->first)].c_str());
    }
  }

  // Initial background music
  Mix_PlayMusic(music_map[MUSIC_ASSET_ID::MUSIC1], -1);
  fprintf(stderr, "Loaded music\n");
}

void AudioSystem::step(float elapsed_ms) {
  for (Entity entity : registry.sounds.entities) {
    Sound sound   = registry.sounds.get(entity);
    int channel = Mix_PlayChannelTimed(-1, sound_map[sound.id], 0, 2000);

    //uncomment this to see debug info
    printf("Play Sound: %s\n", sound_names[static_cast<int>(sound.id)].c_str());
    printf("Channel: %d\n", channel);
    printf("Size: %zu\n", registry.sounds.entities.size());
    if (channel == -1) {
      printf("All audio channels are allocated. %s was not played\n", sound_names[static_cast<int>(sound.id)].c_str());
    }
    Mix_Volume(channel, 64);
  }
  registry.sounds.clear();

  for (Entity entity : registry.musics.entities) {
    Music music = registry.musics.get(entity);
    // This replaces the current looping background music.
    Mix_PlayMusic(music_map[music.id], -1);
  }
  registry.musics.clear();
}