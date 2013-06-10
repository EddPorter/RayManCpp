#pragma once

#include "Colour.h"
#include "point.h"

namespace rayman {
  struct light {
    point pos;
    Colour colour;
  };
}