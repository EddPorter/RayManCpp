#pragma once

namespace rayman {
  struct material {
    float reflection;
    float red;
    float green;
    float blue;

    float specvalue;
    float specpower;

    enum {
      gouraud = 0,
      noise = 1,
      marble = 2,
      turbulence = 3
    } type;

    float red2;
    float green2;
    float blue2;
  };
}