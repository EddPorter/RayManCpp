#include "cubemap.h"
#include <fstream>

using namespace std;

namespace rayman {
  cubemap::cubemap() :
    sizeX(0), sizeY(0), exposure(1.0f), bExposed(true), bsRGB(true) {
    name[cubemap::backward] = "alpback.tga";
    name[cubemap::forward] = "alpforward.tga";
    name[cubemap::up] = "alpup.tga";
    name[cubemap::down] = "alpdown.tga";
    name[cubemap::left] = "alpleft.tga";
    name[cubemap::right] = "alpright.tga";
  }


  cubemap::~cubemap() {
  }

  void cubemap::setExposure(float newExposure) {
    exposure = newExposure;
  }

  bool dummyTGAHeader(ifstream & currentfile, unsigned & sizeX, unsigned & sizeY) {
    char temp;
    currentfile.get(temp).get(temp);
    currentfile.get(temp);                  /* uncompressed RGB */
    if (temp != 2) {
      return false;
    }
    currentfile.get(temp).get(temp);
    currentfile.get(temp).get(temp);
    currentfile.get(temp);
    currentfile.get(temp).get(temp);           /* origin X */
    currentfile.get(temp).get(temp);           /* origin Y */
    currentfile.get(temp);
    sizeX = temp;
    currentfile.get(temp);
    sizeX += temp * 256;

    currentfile.get(temp);
    sizeY = temp;
    currentfile.get(temp);
    sizeY += temp * 256;

    currentfile.get(temp);                 /* 24 bit bitmap */
    currentfile.get(temp);
    return true;
  }

  bool cubemap::Init() {
    if (colour.size() > 0) {
      return true;
    }
    std::ifstream currentfile;
    int x, y;
    unsigned dummySizeX, dummySizeY;

    currentfile.open(name[up].c_str(), std::ios_base::binary);
    if ((!currentfile) || (!dummyTGAHeader(currentfile, sizeX, sizeY))) {
      return false;
    }
    if (sizeX <= 0 || sizeY <= 0) {
      return false;
    }

    int number = cubemap::up * sizeX * sizeY;
    for (auto n = number; n < number + sizeX * sizeY; ++n) {
      Colour currentColor;
      currentColor.blue = currentfile.get() / 255.0f;
      currentColor.green = currentfile.get() / 255.0f;
      currentColor.red = currentfile.get() / 255.0f;
      colour.push_back(currentColor);
    }
    currentfile.close();

    for (unsigned i = cubemap::down; i <= cubemap::backward; ++i) {
      number = i * sizeX * sizeY;
      currentfile.open(name[i].c_str(), ios_base::binary);
      if ((!currentfile) || (!dummyTGAHeader(currentfile, dummySizeX, dummySizeY)) || sizeX != dummySizeX || sizeY != dummySizeY) {
        // The textures for each face have to be of the same size..
        colour.clear();
        return false;
      }

      for (auto n = number; n < number + sizeX * sizeY; ++n) {
        Colour currentColor;
        currentColor.blue = currentfile.get() / 255.f;
        currentColor.green = currentfile.get() / 255.0f;
        currentColor.red = currentfile.get() / 255.0f;
        colour.push_back(currentColor);
      }
      currentfile.close();
    }

    return true;
  }

  Colour cubemap::readCubemap(const ray & myRay) const {
    Colour outputColour = {0.0f, 0.0f, 0.0f};

    if (colour.size() == 0) {
      return outputColour;
    }
    if ((fabsf(myRay.dir.x) >= fabsf(myRay.dir.y)) && (fabsf(myRay.dir.x) >= fabsf(myRay.dir.z))) {
      if (myRay.dir.x > 0.0f) {
        outputColour = readTexture(colour, cubemap::right * sizeX * sizeY,
                                   1.0f - (myRay.dir.z / myRay.dir.x + 1.0f) * 0.5f,
                                   (myRay.dir.y / myRay.dir.x + 1.0f) * 0.5f, sizeX, sizeY);
      } else if (myRay.dir.x < 0.0f) {
        outputColour = readTexture(colour, cubemap::left * sizeX * sizeY,
                                   1.0f - (myRay.dir.z / myRay.dir.x + 1.0f) * 0.5f,
                                   1.0f - ( myRay.dir.y / myRay.dir.x + 1.0f) * 0.5f, sizeX, sizeY);
      }
    } else if ((fabsf(myRay.dir.y) >= fabsf(myRay.dir.x)) && (fabsf(myRay.dir.y) >= fabsf(myRay.dir.z))) {
      if (myRay.dir.y > 0.0f) {
        outputColour = readTexture(colour, cubemap::up * sizeX * sizeY,
                                   (myRay.dir.x / myRay.dir.y + 1.0f) * 0.5f,
                                   1.0f - (myRay.dir.z / myRay.dir.y + 1.0f) * 0.5f, sizeX, sizeY);
      } else if (myRay.dir.y < 0.0f) {
        outputColour = readTexture(colour, cubemap::down * sizeX * sizeY,
                                   1.0f - (myRay.dir.x / myRay.dir.y + 1.0f) * 0.5f,
                                   (myRay.dir.z / myRay.dir.y + 1.0f) * 0.5f, sizeX, sizeY);
      }
    } else if ((fabsf(myRay.dir.z) >= fabsf(myRay.dir.x)) && (fabsf(myRay.dir.z) >= fabsf(myRay.dir.y))) {
      if (myRay.dir.z > 0.0f) {
        outputColour = readTexture(colour, cubemap::forward * sizeX * sizeY,
                                   (myRay.dir.x / myRay.dir.z + 1.0f) * 0.5f,
                                   (myRay.dir.y / myRay.dir.z + 1.0f) * 0.5f, sizeX, sizeY);
      } else if (myRay.dir.z < 0.0f) {
        outputColour = readTexture(colour, cubemap::backward * sizeX * sizeY,
                                   (myRay.dir.x / myRay.dir.z + 1.0f) * 0.5f,
                                   1.0f - (myRay.dir.y / myRay.dir.z + 1) * 0.5f, sizeX, sizeY);
      }
    }

    if (bsRGB)
    {
       // We make sure the data that was in sRGB storage mode is brought back to a  linear format.
      // We don't need the full accuracy of the sRGBEncode function so a powf should be sufficient enough.
       outputColour.blue   = powf(outputColour.blue, 2.2f);
       outputColour.red    = powf(outputColour.red, 2.2f);
       outputColour.green  = powf(outputColour.green, 2.2f);
    }

    if (bExposed)
    {
        // The LDR (low dynamic range) images were supposedly already exposed, but we need to make the inverse transformation
        // so that we can expose them a second time.
        outputColour.blue  = -logf(1.001f - outputColour.blue);
        outputColour.red   = -logf(1.001f - outputColour.red);
        outputColour.green = -logf(1.001f - outputColour.green);
    }

    outputColour.blue  /= exposure;
    outputColour.red   /= exposure;
    outputColour.green /= exposure;

    return outputColour;
  }

  Colour cubemap::readTexture(const std::vector<Colour> & tab, unsigned offset, float u, float v, int sizeU, int sizeV) {
    u = fabsf(u);
    v = fabsf(v);
    int umin = int(sizeU * u);
    int vmin = int(sizeV * v);
    int umax = int(sizeU * u) + 1;
    int vmax = int(sizeV * v) + 1;
    float ucoef = fabsf(sizeU * u - umin);
    float vcoef = fabsf(sizeV * v - vmin);

    // The texture is being addressed on [0,1].
    // There should be an addressing type in order to determine how we should access texels when the coordinates are beyond those boundaries.

    // Clamping is our current default and the only implemented addressing type for now.
    // Clamping is done by bringing anything below zero to the coordinate zero and everything beyond one, to one.
    umin = std::min(std::max(umin, 0), sizeU - 1);
    umax = std::min(std::max(umax, 0), sizeU - 1);
    vmin = std::min(std::max(vmin, 0), sizeV - 1);
    vmax = std::min(std::max(vmax, 0), sizeV - 1);

    // What follows is a bilinear interpolation along two coordinates u and v.

    Colour output =
      (1.0f - vcoef) *
      ((1.0f - ucoef) * tab[offset + umin + sizeU * vmin]
       + ucoef * tab[offset + umax + sizeU * vmin])
      + vcoef *
      ((1.0f - ucoef) * tab[offset + umin + sizeU * vmax]
       + ucoef * tab[offset + umax + sizeU * vmax]);
    return output;
  }
}