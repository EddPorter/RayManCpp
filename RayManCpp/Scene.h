#pragma once

#include "Blob.h"
#include "cubemap.h"
#include "light.h"
#include "material.h"
#include "Perspective.h"
#include "sphere.h"
#include <vector>

namespace rayman {

  struct scene {
    std::vector<material> materialContainer;
    std::vector<sphere>   sphereContainer;
    std::vector<light>    lightContainer;
    std::vector<Blob>     blobContainer;
    int sizex, sizey;
    cubemap cm;
    Perspective persp;
    unsigned complexity;
  };

  bool init(char * inputName, scene & myScene);
  bool draw(char * outputName, scene & myScene);
}