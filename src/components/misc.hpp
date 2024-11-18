#pragma once
#include "common.hpp"
// Data structure for toggling debug mode
struct Debug {
  bool in_debug_mode  = 0;
  bool in_freeze_mode = 0;
};

extern Debug debugging;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent {
  // Note, an empty struct has size 1
};

struct GameCursor {};

struct Emoting {
  Entity child;
};

struct Overlay {};

// Enum for overlay types
enum class OVERLAY {
  INTRO    = 0,
  START    = INTRO + 1,
  PAUSE    = START + 1,
  KRAB     = PAUSE + 1,
  SHARKMAN = KRAB + 1,
  DEATH    = SHARKMAN + 1,
  END      = DEATH + 1,
};