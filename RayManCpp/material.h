#pragma once

#include "Colour.h"

namespace rayman {
  struct material {
    float reflection;
    Colour colour;

    float specvalue;
    float specpower;

    enum {
      gouraud = 0,
      noise = 1,
      marble = 2,
      turbulence = 3
    } type;

    Colour colour2;

    float bump;
  };
}