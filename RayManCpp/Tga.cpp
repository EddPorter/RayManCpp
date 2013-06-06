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

  Tga & Tga::put(uint8_t byte) {
    imageFile.put(byte);
    return *this;
  }
}