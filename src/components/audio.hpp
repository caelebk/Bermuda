#pragma once

struct Sound {
    SOUND_ASSET_ID id;
    float max_time = 2000;

    Sound(SOUND_ASSET_ID asset_id) : id(asset_id) {}
    Sound(SOUND_ASSET_ID asset_id, float max_time) {
        this->id = asset_id;
        this->max_time = max_time;
    }
};

struct Music {
    MUSIC_ASSET_ID id;

    Music(MUSIC_ASSET_ID asset_id) : id(asset_id) {}
};