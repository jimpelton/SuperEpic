
#include "renderer.h"

#include <SDL.h>
#include <Strsafe.h>
#include <Windows.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <vector>

bool parseImagesFile(const std::string &path,
                     std::vector<std::string> *imagePaths) {
  assert(imagePaths != nullptr);
  std::ifstream imagesFile;
  imagesFile.open(path);
  if (!imagesFile.is_open()) {
    std::cerr << "The images file: " << path << " could not be opened.\n"
                                                "Exiting...\n";
    return false;
  }

  while (!imagesFile.eof()) {
    char buf[256];
    imagesFile.getline(buf, 256);
    std::string s{buf};
    if (!s.empty())
      imagePaths->push_back(s);
  }

  imagesFile.close();

  return true;
}

bool parseImagesDirectory(const char *directoryPath,
                          std::vector<std::string> *imagePaths) {
  assert(imagePaths != nullptr);

  DWORD retval = 0;
  TCHAR buffer[4096] = TEXT("");
  TCHAR **lppPart = {NULL};
  TCHAR szDir[MAX_PATH];
  WIN32_FIND_DATA ffd;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  std::string path;

  retval = GetFullPathName(directoryPath, 4096, buffer, lppPart);
  if (retval == 0) {
    // Handle an error condition.
    printf("GetFullPathName failed (%d)\n", GetLastError());
    return false;
  }
  path = std::string(buffer) + "\\";

  StringCchCopy(szDir, MAX_PATH, directoryPath);
  StringCchCat(szDir, MAX_PATH, TEXT("\\*"));
  hFind = FindFirstFile(szDir, &ffd);
  if (INVALID_HANDLE_VALUE == hFind) {
    printf("%s\n", "FindFirstFileFailed");
    return false;
  }
  do {
    if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
	  std::string filename(ffd.cFileName);
	  std::cout << path + filename << std::endl;
	  imagePaths->push_back(path + filename);
    }
  } while (FindNextFile(hFind, &ffd) != 0);

  FindClose(hFind);
  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Please provide a text file with absolute image paths."
              << "\n";
    return 1;
  }

  std::vector<std::string> paths;
  if (!parseImagesDirectory(argv[1], &paths)) {
    return 1;
  }

  Renderer renderer{1280, 720, SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED};
  if (renderer.init() < 0) {
    std::cerr << "Could not create Renderer! Exiting..." << std::endl;
    return 1;
  }

  renderer.loadImages(paths);
  renderer.loop();

  return 0;
}
