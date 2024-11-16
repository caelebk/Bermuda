#pragma once

struct Sound {
    SOUND_ASSET_ID id;

    Sound(SOUND_ASSET_ID asset_id) : id(asset_id) {}
};

struct Music {
    MUSIC_ASSET_ID id;

    Music(MUSIC_ASSET_ID asset_id) : id(asset_id) {}
};