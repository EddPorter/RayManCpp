#include "intersect.h"

namespace rayman {
// r - the ray
// s - the sphere to intersect
// t - [in] the maximum distance to check, [out] the closest intersect distance if function returns true
// returns: true if a positive intersect was found (sets t to the distance), false if not
  bool hitSphere(const ray & r, const sphere & s, float & t) {
    // In an infinite precision world, this would be 0.0f, but we add a little fudge to prevent
    // us from detecting that reflections "intersect" with themselves (when they don't).
    const float MINIMUM_POSITIVE_DISTANCE = 0.1f;

    // A basic quadratic formula solver.
    vector dist = s.pos - r.start;
    double B = r.dir * dist;
    double D = B * B - dist * dist + s.size * s.size;
    if (D < 0.0f) {
      // No solution.
      return false;
    }

    double t0 = B - sqrt(D);
    double t1 = B + sqrt(D);
    bool retvalue = false;
    if ((t0 > MINIMUM_POSITIVE_DISTANCE) && (t0 < t)) {
      t = static_cast<float>(t0);
      retvalue = true;
    }
    if ((t1 > MINIMUM_POSITIVE_DISTANCE) && (t1 < t)) {
      t = static_cast<float>(t1);
      retvalue = true;
    }
    return retvalue;
  }
}