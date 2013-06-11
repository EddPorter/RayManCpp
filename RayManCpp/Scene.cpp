#include "Colour.h"
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
    myScene.complexity = 10;

    const double PIOVER180 = 0.017453292519943295769236907684886;
    myScene.persp.type = Perspective::Conic;
    myScene.persp.FOV = 90.0f;
    myScene.persp.projectionDistance = (0.5f * myScene.sizex / tanf(float(PIOVER180) * 0.5f * myScene.persp.FOV));
    myScene.persp.clearPoint = 330.0f;
    myScene.persp.dispersion = 10.0f;

    material m1 = {0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 60.0f, material::marble, 1.0f, 0.0f, 0.0f, 0.0f};
    myScene.materialContainer.push_back(m1);
    material m2 = {0.2f, 0.0f, 1.0f, 1.0f, 1.0f, 60.0f, material::gouraud, 0.0f, 1.0f, 0.0f, 0.5f};
    myScene.materialContainer.push_back(m2);
    material m3 = {0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 60.0f, material::turbulence, 0.0f, 0.0f, 1.0f, 0.0f};
    myScene.materialContainer.push_back(m3);
    material m4 = {0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 60.0f, material::gouraud, 1.0f, 1.0f, 1.0f, 0.0f};
    myScene.materialContainer.push_back(m4);
    material m5 = {0.0f, 1.0f, 0.5f, 0.5f, 1.0f, 50.0f, material::turbulence, 0.0f, 1.0f, 1.0f, 0.0f};
    myScene.materialContainer.push_back(m5);

    {
      sphere s = {{250.0f, 290.0f, 280.0f}, 100.0f, m1};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{500.0f, 330.0f, 480.0f}, 100.0f, m2};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{320.0f, 140.0f, 400.0f}, 100.0f, m3};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{630.0f, 60.0f, 1000.0f}, 50.0f, m4};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{300.0f, 450.0f, 420.0f}, 50.0f, m5};
      myScene.sphereContainer.push_back(s);
    }

    {
      light l = {{0.0f, 240.0f, -100.0f}, 1.0f, 1.0f, 1.0f};
      myScene.lightContainer.push_back(l);
    }
    {
      light l = {{640.0f, 240.0f, -10000.0f}, 0.6f, 0.7f, 1.0f};
      myScene.lightContainer.push_back(l);
    }

    myScene.cm.Init();

    return true;
  }

  Colour ThrowRay(ray & viewRay, const scene & myScene) {
    Colour output = {0.0f, 0.0f, 0.0f};
    float coef = 1.0f;
    unsigned iter = 0;

    do {
      float t = 2000.0f;

      // For each sphere
      const sphere * hits = nullptr;
      for (auto s = myScene.sphereContainer.cbegin(); s != myScene.sphereContainer.cend(); ++s) {
        if (hitSphere(viewRay, *s, t)) {
          hits = &(*s);
        }
      }
      if (hits == nullptr) {
        // No geometry hit, instead we simulate a virtual environment by looking the color in a environment cube map.
        output += coef * myScene.cm.readCubemap(viewRay);
        break;
      }

      // Calculate point of intersection.
      point intersect = viewRay.start + t * viewRay.dir;

      // Determine the normal on the sphere at the point of intersection.
      vector norm = intersect - hits->pos;
      float temp = norm * norm;
      if (temp == 0.0f) {
        break;
      }
      temp = 1.0f / sqrtf(temp);
      vector normalisedNorm = temp * norm;

      if (hits->mat.bump) {
        float noiseCoefx = float(noise(0.1 * double(intersect.x), 0.1 * double(intersect.y), 0.1 * double(intersect.z)));
        float noiseCoefy = float(noise(0.1 * double(intersect.y), 0.1 * double(intersect.z), 0.1 * double(intersect.x)));
        float noiseCoefz = float(noise(0.1 * double(intersect.z), 0.1 * double(intersect.x), 0.1 * double(intersect.y)));

        normalisedNorm.x = (1.0f - hits->mat.bump) * normalisedNorm.x + hits->mat.bump * noiseCoefx;
        normalisedNorm.y = (1.0f - hits->mat.bump) * normalisedNorm.y + hits->mat.bump * noiseCoefy;
        normalisedNorm.z = (1.0f - hits->mat.bump) * normalisedNorm.z + hits->mat.bump * noiseCoefz;

        temp = normalisedNorm * normalisedNorm;
        if (temp == 0.0f) {
          break;
        }
        temp = 1.0f / sqrtf(temp);
        normalisedNorm = temp * normalisedNorm;
      }

      // Check if the incident light is pointing back at us by checking the dot product.
      for (auto l = myScene.lightContainer.cbegin(); l != myScene.lightContainer.cend(); ++l) {
        vector dist = l->pos - intersect;
        if (normalisedNorm * dist <= 0.0f) {
          continue;
        }

        float t = sqrtf(dist * dist);
        if (t <= 0.0f) {
          continue;
        }
        ray lightRay = {intersect, (1 / t) * dist};

        // Are we in shadow?
        bool inShadow = false;
        for (auto s = myScene.sphereContainer.cbegin(); s != myScene.sphereContainer.cend(); ++s) {
          if (hitSphere(lightRay, *s, t)) {
            inShadow = true;
            break;
          }
        }
        if (!inShadow) {
          // Apply lambertian reflectance.
          float lambert = (lightRay.dir * normalisedNorm) * coef;
          float noiseCoef = 0.0f;

          switch (hits->mat.type) {
            case material::turbulence:
              for (int level = 1; level < 10; ++level) {
                noiseCoef += (1.0f / level) * fabsf(float(noise(level * 0.05 * intersect.x, level * 0.05 * intersect.y, level * 0.05 * intersect.z)));
              };
              output += coef * (lambert * l->colour) * (noiseCoef * hits->mat.colour + (1.0f - noiseCoef) * hits->mat.colour2);

              break;
            case material::marble:

              for (int level = 1; level < 10; level ++) {
                noiseCoef +=  (1.0f / level) * fabsf(float(noise(level * 0.05 * intersect.x, level * 0.05 * intersect.y, level * 0.05 * intersect.z)));
              };
              noiseCoef = 0.5f * sinf((intersect.x + intersect.y) * 0.05f + noiseCoef) + 0.5f;

              output += coef * (lambert * l->colour) * (noiseCoef * hits->mat.colour + (1.0f - noiseCoef) * hits->mat.colour2);

              break;
              // TODO: Implement other procedural textures, e.g. fractal, random particle deposition, checkerboard, etc.
            default:
              output += lambert * l->colour * hits->mat.colour;
          }

          // Apply Blinn-Phong reflection model.
          vector blinnDir = lightRay.dir - viewRay.dir;
          float temp = sqrtf(blinnDir * blinnDir);
          if (temp != 0.0f) {
            vector normalisedBlinnDir = (1.0f / temp) * blinnDir;
            float blinnTerm = std::max(normalisedBlinnDir * normalisedNorm, 0.0f) ;
            blinnTerm = hits->mat.specvalue * powf(blinnTerm, hits->mat.specpower) * coef;
            output += blinnTerm * l->colour;
          }
        }
      }

      // For reflections in the material.
      coef *= hits->mat.reflection;
      float reflect = 2.0f * (viewRay.dir * normalisedNorm);
      viewRay.start = intersect;
      viewRay.dir = viewRay.dir - reflect * normalisedNorm;

    } while (coef > 0.0f && ++iter < 10);

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
        ray viewRay = {{float(x) * accumulationFactor, float(y) * accumulationFactor, -1000.0f}, {0.0f, 0.0f, 1.0f}};
        Colour currentColor = ThrowRay(viewRay, myScene);
        float luminance = 0.2126f * currentColor.red
                          + 0.715160f * currentColor.green
                          + 0.072169f * currentColor.blue;
        mediumPoint += mediumPointWeight * (luminance * luminance);
      }
    }

    float mediumLuminance = sqrtf(mediumPoint);
    if (mediumLuminance > 0.001f) {
      // put the medium luminance to an intermediate gray value
      exposure = logf(0.6f) / mediumLuminance;
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
        float coef(0.25f); // Each sample contributes 0.25 to the main pixel.

        // Compute using 4x Super Sampling in a 2x2 grid.
        for (float fragmentx = x; fragmentx < x + 1.0f; fragmentx += 0.5f) {
          for (float fragmenty = y; fragmenty < y + 1.0f; fragmenty += 0.5f) {

            switch (myScene.persp.type) {
              case Perspective::Orthogonal: {
                // Cast the ray.
                // There is no natural starting point (due to us using orthographic projection)
                // so arbitrarily put it 1000.0f behind the "centre" of the scene.
                ray viewRay = {{float(fragmentx), float(fragmenty), -1000.0f}, {0.0f, 0.0f, 1.0f}};
                Colour temp = ThrowRay(viewRay, myScene);

                temp.blue = 1.0f - expf(temp.blue * exposure);
                temp.red = 1.0f - expf(temp.red * exposure);
                temp.green = 1.0f - expf(temp.green * exposure);

                output += coef * temp;
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

                Colour temp = {0.0f, 0.0f, 0.0f};
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
                  Colour rayResult = ThrowRay(viewRay, myScene); //, context::getDefaultAir());
                  //fTotalWeight += 1.0f;
                  temp += rayResult;
                }
                temp = (1.0f / myScene.complexity) * temp;
                output += coef * temp;
              }
              break;
            }
          }
        }
        imageFile.write(output.blue).write(output.green).write(output.red);
      }
    }
    return true;
  }
}