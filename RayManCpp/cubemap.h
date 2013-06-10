#pragma once

#include "Colour.h"
#include "ray.h"
#include <memory>
#include <string>
#include <vector>

namespace rayman {
  struct cubemap {
    enum {
      up = 0,
      down = 1,
      right = 2,
      left = 3,
      forward = 4,
      backward = 5
    };
    unsigned sizeX;
    unsigned sizeY;
    std::vector<Colour> colour;
    float exposure;
    bool bExposed;
    bool bsRGB;
    std::string name[6];

    cubemap();
    ~cubemap();
    bool Init();
    void setExposure(float newExposure);
    Colour readCubemap(const ray & myRay) const;
    static Colour readTexture(const std::vector<Colour> & tab, unsigned offset, float u, float v, int sizeU, int sizeV);
  };
}