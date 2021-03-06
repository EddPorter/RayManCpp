#pragma once
#include "Scene.h"
#include <cstdint>
#include <iostream>
#include <fstream>

namespace rayman {
  class Tga
  {
  public:
    Tga(const char const * outputName);
    ~Tga(void);

    void Init(const scene & myScene);

    Tga& write(float colour);
  private:
    std::ofstream imageFile;

    Tga& put(uint8_t byte);
    float srgbEncode(float colour);
  };
}