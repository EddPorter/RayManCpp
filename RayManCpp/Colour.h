#pragma once

namespace rayman {
  struct Colour {
    float red, green, blue;

    Colour & operator+=(const Colour & other);
    Colour & operator*=(const Colour & other);
  };

  Colour operator+(Colour lhs, const Colour & rhs);
  Colour operator*(Colour lhs, const Colour & rhs);
  Colour operator*(float fac, Colour col);
}