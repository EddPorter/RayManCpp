#pragma once

#include "point.h"
#include "vector.h"

namespace rayman {
  struct ray {
    point start;
    vector dir;
  };
}
