#include "Tga.h"

using namespace std;

namespace rayman {
  Tga::Tga(const char const * outputName) :
    imageFile(outputName, ios_base::binary) {
  }

  Tga::~Tga(void) {
  }

  void Tga::Init(const scene & myScene) {
    // Addition of the TGA header
    put(0).put(0);
    put(2);        /* RGB not compressed */

    put(0).put(0);
    put(0).put(0);
    put(0);

    put(0).put(0); /* origin X */
    put(0).put(0); /* origin Y */

    put((unsigned char)(myScene.sizex & 0x00FF)).put((unsigned char)((myScene.sizex & 0xFF00) / 256));
    put((unsigned char)(myScene.sizey & 0x00FF)).put((unsigned char)((myScene.sizey & 0xFF00) / 256));
    put(24);       /* 24 bit bitmap */
    put(0);
    // end of the TGA header
  }

  Tga & Tga::write(float colour) {
    float f = srgbEncode(colour);
    return put((unsigned char)std::min(f * 255.0f, 255.0f));
  }

  float Tga::srgbEncode(float c) {
    if (c <= 0.0031308f) {
      return 12.92f * c;
    } else {
      return 1.055f * powf(c, 0.4166667f) - 0.055f; // Inverse gamma 2.4
    }
  }

  Tga & Tga::put(uint8_t byte) {
    imageFile.put(byte);
    return *this;
  }
}