
#include "renderer.h"
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

bool 
parseImagesFile(const std::string &path, std::vector<std::string> *imagePaths)
{
  assert(imagePaths != nullptr);
  std::ifstream imagesFile;
  imagesFile.open(path);
  if (!imagesFile.is_open()) {
    std::cerr << "The images file: " << path << " could not be opened.\n"
      "Exiting...\n";
    return false;
  }

  while (! imagesFile.eof()) {
    char buf[256];
    imagesFile.getline(buf, 256);
    std::string s{ buf };
    if (! s.empty())
      imagePaths->push_back(s);
  }

  imagesFile.close();

  return true;
}

int main(int argc, char *argv[])
{ 
  if (argc < 2) {
    std::cerr << "Please provide a text file with absolute image paths." << "\n";
    return 1;
  }

  std::vector<std::string> paths;
  if (! parseImagesFile(argv[1], &paths)) {
    return 1;
  }

  Renderer renderer{ 1280, 720, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED };
  if (renderer.init() < 0) {
    std::cerr << "Could not create Renderer! Exiting..." << std::endl;
    return 1;
  }

  renderer.loadImages(paths);
  renderer.loop();

  return 0;
}

