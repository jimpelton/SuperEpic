
#include "renderer.h"
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <vector>

bool 
parseImagesFile(const std::string &path, std::vector<std::string> &imagePaths)
{
  std::ifstream imagesFile;
  imagesFile.open(path);
  if (!imagesFile.is_open()) {
    std::cerr << "The images file: " << path << " could not be opened.\n"
      "Exiting...\n";
    return 1;
  }

  while (! imagesFile.eof()) {
    char buf[1024];
    imagesFile.getline(buf, 1024);
    std::string s{ buf };
    if (! s.empty())
      imagePaths.push_back(s);
  }
  
}

int main(int argc, char *argv[])
{ 
  if (argc < 2) {
    std::cerr << "Please provide a text file with absolute image paths." << "\n";
    return 1;
  }
  std::vector<std::string> paths;
  parseImagesFile(argv[1], paths);

  Renderer renderer{ 1280, 720, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED };
  if (renderer.init() < 0) {
    std::cerr << "Could not create Renderer! Exiting..." << std::endl;
    return -1;
  }

  renderer.loadImages(paths);
  renderer.loop();

  return 0;
}

