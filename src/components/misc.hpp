#pragma once
#include "common.hpp"
// Data structure for toggling debug mode
struct Debug {
  bool in_debug_mode = 0;
  bool in_freeze_mode = 0;
};

extern Debug debugging;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent {
  // Note, an empty struct has size 1
};

struct Cursor {

};

struct Emoting {
  Entity child;
};
