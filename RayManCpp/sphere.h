#pragma once
#include "material.h"
#include "point.h"
#include <memory>

namespace rayman {
  struct sphere {
    point pos;
    float size;
    material mat;
  };
}