#pragma once

#include <vector>

#include "types.hpp"

struct Hallway : Entity {
    Hallway();
};

class HallwayBuilder : public SpaceBuilder<Hallway> {
    public:
        HallwayBuilder();
};