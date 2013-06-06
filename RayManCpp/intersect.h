#pragma once

#include "ray.h"
#include "sphere.h"

namespace rayman {
  bool hitSphere(const ray & r, const sphere & s, float & t);
}