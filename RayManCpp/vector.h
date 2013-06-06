#pragma once

#include "point.h"

namespace rayman {

  struct vector {
    float x;
    float y;
    float z;

    vector & operator+=(const vector & v);
    vector & operator-=(const vector & v);
    vector & operator*=(float f);
  };

  vector operator+(vector lhs, const vector & rhs);

  point operator+(point p, const vector & v);
  
  vector operator-(vector lhs, const vector & rhs);

  vector operator-(const point & lhs, const point & rhs);

  point operator-(point p, const vector & v);

  vector operator*(vector v, float f);

  vector operator*(float f, vector v);
  
  float operator*(const vector & lhs, const vector & rhs);
}