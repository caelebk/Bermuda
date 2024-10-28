#pragma once

struct Oxygen {
  float  capacity;
  float  level;
  float  rate;
  Entity oxygenBar;
  Entity backgroundBar;
};

struct OxygenModifier {
  float amount = 0.0;
};
