#include "Blob.h"
#include "Colour.h"
#include "Context.h"
#include "intersect.h"
#include "light.h"
#include "material.h"
#include "perlin.h"
#include "Perspective.h"
#include "ray.h"
#include "sphere.h"
#include "Scene.h"
#include "Tga.h"
#include "vector.h"

namespace rayman {
  bool init(char * inputName, scene & myScene) {
    myScene.sizex = 640;
    myScene.sizey = 480;
    myScene.complexity = 100;

    const double PIOVER180 = 0.017453292519943295769236907684886;
    myScene.persp.type = Perspective::Conic;
    myScene.persp.FOV = 90.0f;
    myScene.persp.projectionDistance = (0.5f * myScene.sizex / tanf(float(PIOVER180) * 0.5f * myScene.persp.FOV));
    myScene.persp.clearPoint = 450.0f;
    myScene.persp.dispersion = 10.0f;

    myScene.tonemap.fBlack = 0.1f;
    myScene.tonemap.fPower = 3.0f;
    myScene.tonemap.fMidPoint = 0.7f;

    if (myScene.tonemap.fBlack = 0.1f) {
      // if we have three user defined parameters
      // there is one parameter left. It's the final scale of the exponent
      // we define it to be such that the midlevel gray stays the same, no matter what the power
      // or the black level is (arbitrary). midpoint = 1 - exp(normExp) = 1 - exp(scale * normExp^power / (black + normExp^(power - 1)));
      float normExp = - logf(1.0f - myScene.tonemap.fMidPoint);
      myScene.tonemap.fPowerScale = -(1.0f + myScene.tonemap.fBlack / powf(normExp, myScene.tonemap.fPower - 1.0f));
    } else {
      myScene.tonemap.fPowerScale = -1.0f;
    }

    material m1 = {0.0f, 0.35f, 0.25f, 0.01f, 0.0f, 60.0f, material::turbulence, 0.5f, 0.5f, 0.01f, 0.0f, 0.0f, 1.0f};
    myScene.materialContainer.push_back(m1);
    material m2 = {1.0f, 0.0f, 0.0f, 0.0f, 1.2f, 60.0f, material::gouraud, 0.0f, 1.0f, 0.0f, 0.1f, 0.0f, 0.0f};
    myScene.materialContainer.push_back(m2);
    material m3 = {0.01f, 0.5f, 0.5f, 0.5f, 1.2f, 60.0f, material::marble, 0.15f, 0.01f, 0.15f, 0.0f, 0.0f, 1.0f};
    myScene.materialContainer.push_back(m3);
    material m4 = {0.9f, 0.0f, 0.0f, 0.0f, 1.2f, 60.0f, material::gouraud, 0.0f, 0.0f, 0.0f, 0.1f, 0.9f, 2.0f};
    myScene.materialContainer.push_back(m4);

    {
      sphere s = {{400.0, 290.0f, 320.0f}, 50.0f, m1};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{600.0f, 290.0f, 480.0f}, 100.0f, m2};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{450.0f, 140.0f, 400.0f}, 50.0f, m3};
      myScene.sphereContainer.push_back(s);
    }
    //    {
    //      sphere s = {{630.0f, 60.0f, 1000.0f}, 50.0f, m4};
    ////      myScene.sphereContainer.push_back(s);
    //    }
    //    {
    //      sphere s = {{300.0f, 450.0f, 420.0f}, 50.0f, m5};
    ////      myScene.sphereContainer.push_back(s);
    //    }

    {
      Blob b;
      {
        point p = {160.0, 290.0, 320.0};
        b.centerList.push_back(p);
      }
      {
        point p = {400.0, 290.0, 480.0};
        b.centerList.push_back(p);
      }
      {
        point p = {250.0, 140.0, 400.0};
        b.centerList.push_back(p);
      }
      b.size = 80.0f;
      b.mat = m4;
      b.invSizeSquare = 1.0f / (b.size * b.size);
      b.initBlobZones();
      myScene.blobContainer.push_back(b);
    }

    {
      light l = {{0.0f, 240.0f, 300.0f}, 5.0f, 5.0f, 5.0f};
      myScene.lightContainer.push_back(l);
    }
    {
      light l = {{640.0f, 480.0f, -100.0f}, 0.6f, 0.7f, 1.0f};
      myScene.lightContainer.push_back(l);
    }

    myScene.cm.Init();

    return true;
  }

  Colour ThrowRay(ray & viewRay, const scene & myScene, Context myContext) {
    Colour output = {0.0f, 0.0f, 0.0f};
    float coef = 1.0f;
    unsigned iter = 0;

    do {
      float t = 2000.0f;

      // For each blob
      const Blob * hitb = nullptr;
      // For each sphere
      const sphere * hits = nullptr;

      for (auto b = myScene.blobContainer.begin(); b != myScene.blobContainer.end(); ++b) {
        if (b->isBlobIntersected(viewRay, t)) {
          hitb = &(*b);
        }
      }

      for (auto s = myScene.sphereContainer.cbegin(); s != myScene.sphereContainer.cend(); ++s) {
        if (hitSphere(viewRay, *s, t)) {
          hits = &(*s);
          hitb = nullptr;
        }
      }

      point intersect;
      vector vNormal;
      material currentMat;

      if (hitb) {
        intersect  = viewRay.start + t * viewRay.dir;
        hitb->blobInterpolation(intersect, vNormal);
        float temp = vNormal * vNormal;

        if (temp == 0.0f) {
          break;
        }

        vNormal *= 1.0f / sqrtf(temp);
        currentMat = hitb->mat;
      } else if (hits) {
        // Calculate point of intersection.
        intersect = viewRay.start + t * viewRay.dir;

        // Determine the normal on the sphere at the point of intersection.
        vector norm = intersect - hits->pos;
        float temp = norm * norm;

        if (temp == 0.0f) {
          break;
        }

        temp = 1.0f / sqrtf(temp);
        vNormal = temp * norm;
        currentMat = hits->mat;
      } else {
        break;
      }


      float bInside;

      if (vNormal * viewRay.dir > 0.0f) {
        vNormal = -1.0f * vNormal;
        bInside = true;
      } else {
        bInside = false;
      }


      if (currentMat.bump) {
        float noiseCoefx = float(noise(0.1 * double(intersect.x), 0.1 * double(intersect.y), 0.1 * double(intersect.z)));
        float noiseCoefy = float(noise(0.1 * double(intersect.y), 0.1 * double(intersect.z), 0.1 * double(intersect.x)));
        float noiseCoefz = float(noise(0.1 * double(intersect.z), 0.1 * double(intersect.x), 0.1 * double(intersect.y)));

        vNormal.x = (1.0f - currentMat.bump) * vNormal.x + currentMat.bump * noiseCoefx;
        vNormal.y = (1.0f - currentMat.bump) * vNormal.y + currentMat.bump * noiseCoefy;
        vNormal.z = (1.0f - currentMat.bump) * vNormal.z + currentMat.bump * noiseCoefz;

        float temp = vNormal * vNormal;

        if (temp == 0.0f) {
          break;
        }

        temp = 1.0f / sqrtf(temp);
        vNormal = temp * vNormal;
      }

      float fViewProjection = viewRay.dir * vNormal;
      float fReflectance, fTransmittance;
      float fCosThetaI, fSinThetaI, fCosThetaT, fSinThetaT;

      if (currentMat.reflection != 0.0f || currentMat.refraction != 0.0f || currentMat.density != 0.0f) {
        float fDensity1 = myContext.fRefractionCoef;
        // We only consider the case where the ray is originating a medium close to the void (or air)
        // If inside, in theory, we should first determine if the current object is inside another one
        // but that's beyond the purpose of our code.
        float fDensity2 = bInside ? fDensity2 = Context::getDefaultAir().fRefractionCoef : currentMat.density;

        // Here we take into account that the light movement is symmetrical
        // From the observer to the source or from the source to the oberver.
        // We then do the computation of the coefficient by taking into account
        // the ray coming from the viewing point.
        fCosThetaI = fabsf(fViewProjection);

        if (fCosThetaI >= 0.999f) {
          // In this case the ray is coming parallel to the normal to the surface
          fReflectance = (fDensity1 - fDensity2) / (fDensity1 + fDensity2);
          fReflectance = fReflectance * fReflectance;
          fSinThetaI = 0.0f;
          fSinThetaT = 0.0f;
          fCosThetaT = 1.0f;
        } else {
          fSinThetaI = sqrtf(1 - fCosThetaI * fCosThetaI);
          // The sign of SinThetaI has no importance, it is the same as the one of SinThetaT
          // and they vanish in the computation of the reflection coefficient.
          fSinThetaT = (fDensity1 / fDensity2) * fSinThetaI;

          if (fSinThetaT * fSinThetaT > 0.9999f) {
            // Beyond that angle all surfaces are purely reflective
            fReflectance = 1.0f ; // pure reflectance at grazing angle
            fCosThetaT = 0.0f;
          } else {
            fCosThetaT = sqrtf(1 - fSinThetaT * fSinThetaT);
            // First we compute the reflectance in the plane orthogonal
            // to the plane of reflection.
            float fReflectanceOrtho =
              (fDensity2 * fCosThetaT - fDensity1 * fCosThetaI )
              / (fDensity2 * fCosThetaT + fDensity1  * fCosThetaI);
            fReflectanceOrtho = fReflectanceOrtho * fReflectanceOrtho;
            // Then we compute the reflectance in the plane parallel to the plane of reflection
            float fReflectanceParal =
              (fDensity1 * fCosThetaT - fDensity2 * fCosThetaI )
              / (fDensity1 * fCosThetaT + fDensity2 * fCosThetaI);
            fReflectanceParal = fReflectanceParal * fReflectanceParal;

            // The reflectance coefficient is the average of those two.
            // If we consider a light that hasn't been previously polarized.
            fReflectance =  0.5f * (fReflectanceOrtho + fReflectanceParal);
          }
        }
      } else {
        // Reflection in a metal-like material. Reflectance is equal in all directions.
        // Note, that metal are conducting electricity and as such change the polarity of the
        // reflected ray. But of course we ignore that..
        fReflectance = 1.0f;
        fCosThetaI = 1.0f;
        fCosThetaT = 1.0f;
      }

      fTransmittance = currentMat.refraction * (1.0f - fReflectance);
      fReflectance = currentMat.reflection * fReflectance;

      float fTotalWeight = fReflectance + fTransmittance;
      bool bDiffuse = false;

      if (fTotalWeight > 0.0f) {
        float fRoulette = (1.0f / RAND_MAX) * rand();

        if (fRoulette <= fReflectance) {
          coef *= currentMat.reflection;

          float fReflection = - 2.0f * fViewProjection;

          viewRay.start = intersect;
          viewRay.dir += fReflection * vNormal;
        } else if (fRoulette <= fTotalWeight) {
          coef *= currentMat.refraction;
          float fOldRefractionCoef = myContext.fRefractionCoef;

          if (bInside) {
            myContext.fRefractionCoef = Context::getDefaultAir().fRefractionCoef;
          } else {
            myContext.fRefractionCoef = currentMat.density;
          }

          // Here we compute the transmitted ray with the formula of Snell-Descartes
          viewRay.start = intersect;

          viewRay.dir = viewRay.dir + fCosThetaI * vNormal;
          viewRay.dir = (fOldRefractionCoef / myContext.fRefractionCoef) * viewRay.dir;
          viewRay.dir += (-fCosThetaT) * vNormal;
        } else {
          bDiffuse = true;
        }
      } else {
        bDiffuse = true;
      }

      if (!bInside && bDiffuse) {
        // Now the "regular lighting"


        // Check if the incident light is pointing back at us by checking the dot product.
        for (auto l = myScene.lightContainer.cbegin(); l != myScene.lightContainer.cend(); ++l) {
          vector dist = l->pos - intersect;
          float fLightProjection = vNormal * dist;

          if (fLightProjection <= 0.0f) {
            continue;
          }

          float t = sqrtf(dist * dist);

          if (t <= 0.0f) {
            continue;
          }

          ray lightRay = {intersect, (1.0f / t) * dist};
          fLightProjection *= 1.0f / t;

          // Are we in shadow?
          bool inShadow = false;

          for (auto s = myScene.sphereContainer.cbegin(); s != myScene.sphereContainer.cend(); ++s) {
            if (hitSphere(lightRay, *s, t)) {
              inShadow = true;
              break;
            }
          }

          for (auto b = myScene.blobContainer.cbegin(); !inShadow && b != myScene.blobContainer.cend(); ++b) {
            if (b->isBlobIntersected(lightRay, t)) {
              inShadow = true;
              break;
            }
          }

          if (!inShadow && fLightProjection > 0.0f) {
            // Apply lambertian reflectance.
            float lambert = (lightRay.dir * vNormal) * coef;
            float noiseCoef = 0.0f;

            switch (currentMat.type) {
              case material::turbulence:
                for (int level = 1; level < 10; ++level) {
                  noiseCoef += (1.0f / level) * fabsf(float(noise(level * 0.05 * intersect.x, level * 0.05 * intersect.y, level * 0.05 * intersect.z)));
                };

                output += coef * (lambert * l->colour) * (noiseCoef * currentMat.colour + (1.0f - noiseCoef) * currentMat.colour2);

                break;

              case material::marble:

                for (int level = 1; level < 10; level ++) {
                  noiseCoef +=  (1.0f / level) * fabsf(float(noise(level * 0.05 * intersect.x, level * 0.05 * intersect.y, level * 0.05 * intersect.z)));
                };

                noiseCoef = 0.5f * sinf((intersect.x + intersect.y) * 0.05f + noiseCoef) + 0.5f;

                output += coef * (lambert * l->colour) * (noiseCoef * currentMat.colour + (1.0f - noiseCoef) * currentMat.colour2);

                break;

                // TODO: Implement other procedural textures, e.g. fractal, random particle deposition, checkerboard, etc.
              default:
                output += lambert * l->colour * currentMat.colour;
            }

            // Apply Blinn-Phong reflection model.
            vector blinnDir = lightRay.dir - viewRay.dir;
            float temp = sqrtf(blinnDir * blinnDir);

            if (temp != 0.0f) {
              vector normalisedBlinnDir = (1.0f / temp) * blinnDir;
              float blinnTerm = std::max(normalisedBlinnDir * vNormal, 0.0f) ;
              blinnTerm = currentMat.specvalue * powf(blinnTerm, currentMat.specpower) * coef;
              output += blinnTerm * l->colour;
            }
          }
        }

        coef = 0.0f;
      }

      //// For reflections in the material.
      //coef *= currentMat.reflection;
      //float reflect = 2.0f * (viewRay.dir * vNormal);
      //viewRay.start = intersect;
      //viewRay.dir = viewRay.dir - reflect * vNormal;

    } while (coef > 0.0f && ++iter < 10);

    if (coef > 0.0f) {
      output += coef * myScene.cm.readCubemap(viewRay);
    }

    return output;
  }

  float AutoExposure(scene & myScene) {
#define ACCUMULATION_SIZE 16
    float exposure = -1.0f;
    float accumulationFactor = float(std::max(myScene.sizex, myScene.sizey));

    accumulationFactor = accumulationFactor / ACCUMULATION_SIZE;
    float mediumPoint = 0.0f;
    const float mediumPointWeight = 1.0f / (ACCUMULATION_SIZE * ACCUMULATION_SIZE);

    for (int y = 0; y < ACCUMULATION_SIZE; ++y) {
      for (int x = 0 ; x < ACCUMULATION_SIZE; ++x) {
        switch (myScene.persp.type) {
          case Perspective::Orthogonal: {
            ray viewRay = {{float(x) * accumulationFactor, float(y) * accumulationFactor, -1000.0f}, {0.0f, 0.0f, 1.0f}};
            Colour currentColor = ThrowRay(viewRay, myScene, Context::getDefaultAir());
            float luminance = 0.2126f * currentColor.red
                              + 0.715160f * currentColor.green
                              + 0.072169f * currentColor.blue;
            mediumPoint += mediumPointWeight * (luminance * luminance);
          }
          break;

          case Perspective::Conic: {
            vector dir = {(float(x)*accumulationFactor - 0.5f * myScene.sizex) / myScene.persp.projectionDistance,
                          (float(y) * accumulationFactor - 0.5f * myScene.sizey) / myScene.persp.projectionDistance,
                          1.0f
                         };

            float norm = dir * dir;

            // I don't think this can happen but we've never too prudent
            if (norm == 0.0f) {
              break;
            }

            dir *= 1.0f / sqrtf(norm);

            ray viewRay = { {0.5f * myScene.sizex,  0.5f * myScene.sizey, 0.0f}, {dir.x, dir.y, dir.z} };
            Colour currentColor = ThrowRay(viewRay, myScene, Context::getDefaultAir());
            float luminance = 0.2126f * currentColor.red
                              + 0.715160f * currentColor.green
                              + 0.072169f * currentColor.blue;
            mediumPoint = mediumPoint + mediumPointWeight * (luminance * luminance);
          }
          break;
        }
      }
    }

    float mediumLuminance = sqrtf(mediumPoint);

    if (mediumLuminance > 0.0f) {
      // put the medium luminance to an intermediate gray value
      //exposure = logf(0.6f) / mediumLuminance;
      exposure = - logf(1.0f - myScene.tonemap.fMidPoint) / mediumLuminance;

    }

    return exposure;
  }

  bool draw(char * outputName, scene & myScene) {

    Tga imageFile(outputName);
    imageFile.Init(myScene);

    float exposure = AutoExposure(myScene);

    for (unsigned y = 0; y < myScene.sizey; ++y) {
      for (unsigned x = 0; x < myScene.sizex; ++x) {
        Colour output = {0.0f, 0.0f, 0.0f};
        float sampleRatio(0.25f); // Each sample contributes 0.25 to the main pixel.

        // Compute using 4x Super Sampling in a 2x2 grid.
        for (float fragmentx = x; fragmentx < x + 1.0f; fragmentx += 0.5f) {
          for (float fragmenty = y; fragmenty < y + 1.0f; fragmenty += 0.5f) {
            Colour temp = {0.0f, 0.0f, 0.0f};

            switch (myScene.persp.type) {
              case Perspective::Orthogonal: {
                // Cast the ray.
                // There is no natural starting point (due to us using orthographic projection)
                // so arbitrarily put it 1000.0f behind the "centre" of the scene.
                ray viewRay = {{float(fragmentx), float(fragmenty), -1000.0f}, {0.0f, 0.0f, 1.0f}};

                for (int i = 0; i < myScene.complexity; ++i) {
                  temp += ThrowRay(viewRay, myScene, Context::getDefaultAir());
                }

                temp = (1.0f / myScene.complexity) * temp;
              }
              break;

              case Perspective::Conic: {
                vector dir = {(fragmentx - 0.5f * myScene.sizex) / myScene.persp.projectionDistance,
                              (fragmenty - 0.5f * myScene.sizey) / myScene.persp.projectionDistance,
                              1.0f
                             };

                float norm = dir * dir;

                if (norm == 0.0f) {
                  break;
                }

                dir = dir * (1.0f / sqrtf(norm));
                // The starting point is always the optical center of the camera.
                // We will add some perturbation later to simulate a depth of field effect.
                point start = {0.5f * myScene.sizex,  0.5f * myScene.sizey, 0.0f};

                // The point aimed is one of the invariant of the current pixel.
                // That means that by design every ray that contribute to the current pixel must go through that point in space (on the "sharp" plane).
                // Of course the divergence is caused by the direction of the ray itself.
                point ptAimed = start + myScene.persp.clearPoint * dir;

                for (int i = 0; i < myScene.complexity; ++i) {
                  ray viewRay = {{start.x, start.y, start.z}, {dir.x, dir.y, dir.z}};

                  if (myScene.persp.dispersion != 0.0f) {
                    vector vDisturbance;
                    vDisturbance.x = (myScene.persp.dispersion / RAND_MAX) * (1.0f * rand());
                    vDisturbance.y = (myScene.persp.dispersion / RAND_MAX) * (1.0f * rand());
                    vDisturbance.z = 0.0f;

                    viewRay.start = viewRay.start + vDisturbance;
                    viewRay.dir = ptAimed - viewRay.start;

                    // Normalise
                    norm = viewRay.dir * viewRay.dir;

                    if (norm == 0.0f) {
                      break;
                    }

                    viewRay.dir *= 1.0f / sqrtf(norm);
                  }

                  Colour rayResult = ThrowRay(viewRay, myScene, Context::getDefaultAir());
                  //fTotalWeight += 1.0f;
                  temp += rayResult;
                }

                temp = (1.0f / myScene.complexity) * temp;
              }
              break;
            }

            // pseudo photo exposure
            temp.blue   *= exposure;
            temp.red    *= exposure;
            temp.green  *= exposure;

            if (myScene.tonemap.fBlack > 0.0f) {
              temp.blue   = 1.0f - expf(myScene.tonemap.fPowerScale * powf(temp.blue, myScene.tonemap.fPower)
                                        / (myScene.tonemap.fBlack + powf(temp.blue, myScene.tonemap.fPower - 1.0f)) );
              temp.red    = 1.0f - expf(myScene.tonemap.fPowerScale * powf(temp.red, myScene.tonemap.fPower)
                                        / (myScene.tonemap.fBlack + powf(temp.red, myScene.tonemap.fPower - 1.0f)) );
              temp.green  = 1.0f - expf(myScene.tonemap.fPowerScale * powf(temp.green, myScene.tonemap.fPower)
                                        / (myScene.tonemap.fBlack + powf(temp.green, myScene.tonemap.fPower - 1.0f)) );
            } else {
              // If the black level is 0 then all other parameters have no effect
              temp.blue   = 1.0f - expf(myScene.tonemap.fPowerScale * temp.blue);
              temp.red    = 1.0f - expf(myScene.tonemap.fPowerScale * temp.red);
              temp.green  = 1.0f - expf(myScene.tonemap.fPowerScale * temp.green);
            }

            output += sampleRatio * temp;
          }
        }

        imageFile.write(output.blue).write(output.green).write(output.red);
      }
    }

    return true;
  }
}