#pragma once

struct perlin {
  static perlin & getInstance();
  static double fade(double t);
  static double lerp(double t, double a, double b);
  static double grad(int hash, double x, double y, double z);

private:
  int p[512];
  perlin(void);

  friend double noise(double x, double y, double z);
};


double noise(double x, double y, double z);