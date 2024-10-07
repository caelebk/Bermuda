#pragma once

#include <vector>

#include "space.hpp"

struct Hallway : Entity {
    Hallway();
};

class HallwayBuilder : public SpaceBuilder<Hallway> {
    public:
        HallwayBuilder();
};