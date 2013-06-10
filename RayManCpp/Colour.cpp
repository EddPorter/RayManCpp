#include "Colour.h"

namespace rayman {

  Colour & Colour::operator+=(const Colour & other) {
    red += other.red;
    green += other.green;
    blue += other.blue;
    return *this;
  }

  Colour & Colour::operator*=(const Colour & other) {
    red *= other.red;
    green *= other.green;
    blue *= other.blue;
    return *this;
  }

  Colour operator+(Colour lhs, const Colour & rhs) {
    lhs += rhs;
    return lhs;
  }

  Colour operator*(Colour lhs, const Colour & rhs) {
    lhs *= rhs;
    return lhs;
  }

  Colour operator*(float fac, Colour col) {
    col.red *= fac;
    col.green *= fac;
    col.blue *= fac;
    return col;
  }

}