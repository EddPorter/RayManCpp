#include "intersect.h"
#include "light.h"
#include "material.h"
#include "ray.h"
#include "sphere.h"
#include "Scene.h"
#include "Tga.h"
#include "vector.h"

namespace rayman {
  bool init(char * inputName, scene & myScene) {
    myScene.sizex = 640;
    myScene.sizey = 480;

    material m1 = {0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 60.0f};
    myScene.materialContainer.push_back(m1);
    material m2 = {0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 60.0f};
    myScene.materialContainer.push_back(m2);
    material m3 = {0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 60.0f};
    myScene.materialContainer.push_back(m3);

    {
      //sphere s = {{233.0f, 290.0f, 0.0f}, 100.0f, m1};
      sphere s = {{150.0f, 290.0f, 0.0f}, 100.0f, m1};
      myScene.sphereContainer.push_back(s);
    }
    {
      //sphere s = {{407.0f, 290.0f, 0.0f}, 100.0f, m2};
      sphere s = {{500.0f, 330.0f, 0.0f}, 100.0f, m2};
      myScene.sphereContainer.push_back(s);
    }
    {
      sphere s = {{320.0f, 140.0f, 0.0f}, 100.0f, m3};
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
    return true;
  }

  bool draw(char * outputName, scene & myScene) {

    Tga imageFile(outputName);
    imageFile.Init(myScene);

    for (unsigned y = 0; y < myScene.sizey; ++y) {
      for (unsigned x = 0; x < myScene.sizex; ++x) {
        float blue(0), red(0), green(0);

        // Compute using 4x Super Sampling in a 2x2 grid.
        for (float fragmentx = x; fragmentx < x + 1.0f; fragmentx += 0.5f) {
          for (float fragmenty = y; fragmenty < y + 1.0f; fragmenty += 0.5f) {
            // Each sample contributes 0.25 to the main pixel.
            float coef(0.25f);
            unsigned iter = 0;

            // Cast the ray.
            // There is no natural starting point (due to us using orthographic projection)
            // so arbitrarily put it 1000.0f behind the "centre" of the scene.
            ray viewRay = {{float(fragmentx), float(fragmenty), -1000.0f}, {0.0f, 0.0f, 1.0f}};
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
                  red += lambert * l->red * hits->mat.red;
                  green += lambert * l->green * hits->mat.green;
                  blue += lambert * l->blue * hits->mat.blue;

                  // Apply Blinn-Phong reflection model.
                  vector blinnDir = lightRay.dir - viewRay.dir;
                  float temp = sqrtf(blinnDir * blinnDir);
                  if (temp != 0.0f) {
                    vector normalisedBlinnDir = (1.0f / temp) * blinnDir;
                    float blinnTerm = std::max(normalisedBlinnDir * normalisedNorm, 0.0f) ;
                    blinnTerm = hits->mat.specvalue * powf(blinnTerm, hits->mat.specpower) * coef;
                    red += blinnTerm * l->red;
                    green += blinnTerm * l->green;
                    blue += blinnTerm * l->blue;
                  }
                }
              }

              // For reflections in the material.
              coef *= hits->mat.reflection;
              float reflect = 2.0f * (viewRay.dir * normalisedNorm);
              viewRay.start = intersect;
              viewRay.dir = viewRay.dir - reflect * normalisedNorm;

            } while (coef > 0.0f && ++iter < 10);
          }
        }
        imageFile.write(blue).write(green).write(red);
      }
    }
    return true;
  }
}