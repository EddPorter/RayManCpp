#pragma once

#include "material.h"
#include "point.h"
#include "ray.h"
#include "vector.h"
#include <vector>

namespace rayman {
// Each blob is defined by a list of point source (in the centerList) that affects its potential field.
// We define each source to be equally potent, but we could have a different potential at each point source
// this is for illustration purpose only.
  struct Blob {
    std::vector<point> centerList;
    float size;
    material mat;
    float invSizeSquare;
    
    bool isBlobIntersected(const ray & r, float & t) const;
    void blobInterpolation(const point & pos, vector & vOut) const;
    void initBlobZones();
  };
}