#include "Scene.h"
using namespace rayman;

int main() {
  scene myScene;
  init("scene.txt", myScene);
  draw("output.tga", myScene);
  return 0;
}