#pragma once

namespace rayman {
  struct Perspective {
    enum {
      Orthogonal,
      Conic
    } type;
    float FOV;
    float projectionDistance;
    float clearPoint;
    float dispersion;
  };
}