#include "Blob.h"

#include <algorithm>

namespace rayman {
  // A second degree polynom is defined by its coeficient
  // a * x^2 + b * x + c
  struct poly {
    float a, b, c, fDistance, fDeltaFInvSquare;
  };

  const int zoneNumber = 10;

  // Space around a source of potential is divided into concentric spheric zones
  // Each zone will define the gamma and beta number that approaches
  // linearly (f(x) = gamma * x + beta) the curves of 1 / dist^2
  // Since those coefficients are independant of the actual size of the spheres
  // we can compute it once and only once in the initBlobZones function

  // fDeltaInvSquare is the maximum value that the current point source in the current
  // zone contributes to the potential field (defined incrementally)
  // Adding them for each zone that we entered and exit will give us
  // a conservative estimate of the value of that field per zone
  // which allows us to exit early later if there is no chance
  // that the potential hits our equipotential value.
  struct {
    float fCoef, fDeltaFInvSquare, fGamma, fBeta;
  } zoneTab[zoneNumber] = {
    {10.0f,     0, 0, 0},
    {5.0f,      0, 0, 0},
    {3.33333f,  0, 0, 0},
    {2.5f,      0, 0, 0},
    {2.0f,      0, 0, 0},
    {1.66667f,  0, 0, 0},
    {1.42857f,  0, 0, 0},
    {1.25f,     0, 0, 0},
    {1.1111f,   0, 0, 0},
    {1.0f,      0, 0, 0}
  };

  bool Blob::isBlobIntersected(const ray & r, float & t) const {
    // Having a static structure helps double performance. It obviously wouldn't work if we were running in multiple threads.
    static std::vector<poly> polynomMap;
    polynomMap.resize(0);

    float rSquare, rInvSquare;
    rSquare = size * size;
    rInvSquare = invSizeSquare;
    float maxEstimatedPotential = 0.0f;

    // outside of all the influence spheres, the potential is zero
    float A = 0.0f;
    float B = 0.0f;
    float C = 0.0f;

    for (unsigned int i = 0; i < centerList.size(); i++) {
      point currentPoint = centerList[i];

      vector vDist = currentPoint - r.start;
      const float A = 1.0f;
      const float B = - 2.0f * r.dir * vDist;
      const float C = vDist * vDist;
      // Accelerate delta computation by keeping common computation outside of the loop
      const float BSquareOverFourMinusC = 0.25f * B * B - C;
      const float MinusBOverTwo = -0.5f * B;
      const float ATimeInvSquare = A * rInvSquare;
      const float BTimeInvSquare = B * rInvSquare;
      const float CTimeInvSquare = C * rInvSquare;

      // the current sphere, has N zones of influences
      // we go through each one of them, as long as we've detected
      // that the intersecting ray has hit them
      // Since all the influence zones of many spheres
      // are imbricated, we compute the influence of the current sphere
      // by computing the delta of the previous polygon
      // that way, even if we reorder the zones later by their distance
      // on the ray, we can still have our estimate of
      // the potential function.
      // What is implicit here is that it only works because we've approximated
      // 1/dist^2 by a linear function of dist^2
      for (int j = 0; j < zoneNumber - 1; j++) {
        // We compute the "delta" of the second degree equation for the current
        // spheric zone. If it's negative it means there is no intersection
        // of that spheric zone with the intersecting ray
        const float fDelta = BSquareOverFourMinusC + zoneTab[j].fCoef * rSquare;

        if (fDelta < 0.0f) {
          // Zones go from bigger to smaller, so that if we don't hit the current one,
          // there is no chance we hit the smaller one
          break;
        }

        const float sqrtDelta = sqrtf(fDelta);
        const float t0 = MinusBOverTwo - sqrtDelta;
        const float t1 = MinusBOverTwo + sqrtDelta;

        // because we took the square root (a positive number), it's implicit that
        // t0 is smaller than t1, so we know which is the entering point (into the current
        // sphere) and which is the exiting point.
        poly poly0 = {zoneTab[j].fGamma * ATimeInvSquare ,
                      zoneTab[j].fGamma * BTimeInvSquare ,
                      zoneTab[j].fGamma * CTimeInvSquare + zoneTab[j].fBeta,
                      t0,
                      zoneTab[j].fDeltaFInvSquare
                     };
        poly poly1 = { - poly0.a, - poly0.b, - poly0.c,
                       t1,
                       -poly0.fDeltaFInvSquare
                     };

        maxEstimatedPotential += zoneTab[j].fDeltaFInvSquare;

        // just put them in the vector at the end
        // we'll sort all those point by distance later
        polynomMap.push_back(poly0);
        polynomMap.push_back(poly1);
      };
    }

    if (polynomMap.size() < 2 || maxEstimatedPotential < 1.0f) {
      return false;
    }

    // sort the various entry/exit points per distance
    // by going from the smaller distance to the bigger
    // we can reconstruct the field approximately along the way
    std::sort(polynomMap.begin(), polynomMap.end(), [](poly & elem1, poly & elem2) {
      return elem1.fDistance < elem2.fDistance;
    });

    maxEstimatedPotential = 0.0f;
    bool bResult = false;
    auto it = polynomMap.cbegin();
    auto itNext = it + 1;

    for (; itNext != polynomMap.end(); it = itNext, ++itNext) {
      // A * x2 + B * y + C, defines the condition under which the intersecting
      // ray intersects the equipotential surface. It works because we designed it that way
      // (refer to the article).
      A += it->a;
      B += it->b;
      C += it->c;
      maxEstimatedPotential += it->fDeltaFInvSquare;

      if (maxEstimatedPotential < 1.0f) {
        // No chance that the potential will hit 1.0f in this zone, go to the next zone
        // just go to the next zone, we may have more luck
        continue;
      }

      const float fZoneStart =  it->fDistance;
      const float fZoneEnd = itNext->fDistance;

      // the current zone limits may be outside the ray start and the ray end
      // if that's the case just go to the next zone, we may have more luck
      if (t > fZoneStart &&  0.01f < fZoneEnd ) {
        // This is the exact resolution of the second degree
        // equation that we've built
        // of course after all the approximation we've done
        // we're not going to have the exact point on the iso surface
        // but we should be close enough to not see artifacts
        float fDelta = B * B - 4.0f * A * (C - 1.0f) ;

        if (fDelta < 0.0f) {
          continue;
        }

        const float fInvA = (0.5f / A);
        const float fSqrtDelta = sqrtf(fDelta);

        const float t0 = fInvA * (- B - fSqrtDelta);
        const float t1 = fInvA * (- B + fSqrtDelta);

        if ((t0 > 0.01f ) && (t0 >= fZoneStart ) && (t0 < fZoneEnd) && (t0 <= t )) {
          t = t0;
          bResult = true;
        }

        if ((t1 > 0.01f ) && (t1 >= fZoneStart ) && (t1 < fZoneEnd) && (t1 <= t )) {
          t = t1;
          bResult = true;
        }

        if (bResult) {
          return true;
        }
      }
    }

    return false;
  }
  void Blob::blobInterpolation(const point & pos, vector & vOut) const {
    vector gradient = {0.0f, 0.0f, 0.0f};

    float fRSquare = size * size;

    for (unsigned int i = 0; i < centerList.size(); i++) {
      // This is the true formula of the gradient in the
      // potential field and not an estimation.
      // gradient = normal to the iso surface
      vector normal = pos - centerList[i];
      float fDistSquare = normal * normal;

      if (fDistSquare <= 0.001f) {
        continue;
      }

      float fDistFour = fDistSquare * fDistSquare;
      normal = (fRSquare / fDistFour) * normal;

      gradient = gradient + normal;
    }

    vOut = gradient;
  }

  void Blob::initBlobZones() {
    float fLastGamma = 0.0f, fLastBeta = 0.0f;
    float fLastInvRSquare = 0.0f;

    for (int i = 0; i < zoneNumber - 1; i++) {
      float fInvRSquare = 1.0f / zoneTab[i + 1].fCoef;
      zoneTab[i].fDeltaFInvSquare = fInvRSquare - fLastInvRSquare;
      // fGamma is the ramp between the entry point and the exit point.
      // We only store the difference compared to the previous zone
      // that way we can reconstruct the estimate more easily later..
      float temp = (fLastInvRSquare - fInvRSquare) / (zoneTab[i].fCoef - zoneTab[i + 1].fCoef);
      zoneTab[i].fGamma = temp - fLastGamma;
      fLastGamma = temp ;

      // fBeta is the value of the line approaching the curve for dist = 0 (f = fGamma * x + fBeta)
      // similarly we only store the difference with the fBeta of the previous curve
      zoneTab[i].fBeta = fInvRSquare - fLastGamma * zoneTab[i + 1].fCoef - fLastBeta;
      fLastBeta = zoneTab[i].fBeta + fLastBeta;

      fLastInvRSquare = fInvRSquare;
    };

    // The last zone acts as a simple terminator
    // (no need to evaluate the field there, because we know that it exceed
    // the equipotential value.. by design)
    zoneTab[zoneNumber - 1].fGamma = 0.0f;

    zoneTab[zoneNumber - 1].fBeta = 0.0f;
  }
}