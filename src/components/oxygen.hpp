#pragma once

struct Oxygen {
  float  capacity;
  float  level;
  float  rate;
  bool   isRendered;
  Entity oxygenBar;
  Entity backgroundBar;
};

struct OxygenModifier {
  float amount = 0.0;
};

// prevents entities from constantly modifying oxygen
struct ModifyOxygenCD {
  float default_cd = 0.f;
  float curr_cd    = 0.f;
};
