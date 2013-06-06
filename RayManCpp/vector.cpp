#include "vector.h"

namespace rayman {
  vector & vector::operator+=(const vector & v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  vector & vector::operator-=(const vector & v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  vector & vector::operator*=(float f) {
    x *= f;
    y *= f;
    z *= f;
    return *this;
  }

  vector operator+(vector lhs, const vector & rhs) {
    lhs += rhs;
    return lhs;
  }

  point operator+(point p, const vector & v) {
    point newp = {p.x + v.x, p.y + v.y, p.z + v.z};
    return newp;
  }

  vector operator-(vector lhs, const vector & rhs) {
    lhs -= rhs;
    return lhs;
  }

  vector operator-(const point & lhs, const point & rhs) {
    vector v = {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    return v;
  }

  point operator-(point p, const vector & v) {
    point newp = {p.x - v.x, p.y - v.y, p.z - v.z};
    return newp;
  }

  vector operator*(vector v, float f) {
    v *= f;
    return v;
  }

  vector operator*(float f, vector v) {
    v *= f;
    return v;
  }

  float operator*(const vector & lhs, const vector & rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
  }
}