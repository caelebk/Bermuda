#include "common.hpp"
#include "json.hpp"
#include "level_builder.hpp"
#include "level_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

bool save_game_to_file();
bool load_game_from_file();
bool load_game_from_file_fresh();
void init_save_system(LevelBuilder* lb, LevelSystem* ls, RenderSystem* r);
unsigned int get_seed_from_save_file();

extern Entity player;
extern Entity harpoon_gun;
extern bool   krab_boss_encountered;
extern bool   sharkman_encountered;
