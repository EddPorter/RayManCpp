#pragma once

#include "Colour.h"

namespace rayman {
  struct Context {
    float fRefractionCoef;
    Colour cLightScattering;
    int level;

    static const Context & getDefaultAir();
  };
}