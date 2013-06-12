#include "Context.h"

namespace rayman {
  const Context & Context::getDefaultAir() {
    static Context airContext = {1.0f, {0.0f, 0.0f, 0.0f}, 0};
    return airContext;
  }
}