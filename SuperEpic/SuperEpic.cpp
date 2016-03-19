
#include <SDL.h>

#include <iostream>
#include <Kinect.h>
#include <thread>

#include "KinectSensor.h"


namespace
{
  const int WINDOW_WIDTH{ 640 };
  const int WINDOW_HEIGHT{ 480 };
} // namespace

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << 
        "\nExiting..." << std::endl;
    return 1;
  }

  // Set log messages
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

  SDL_Window *window{ nullptr };
  window = SDL_CreateWindow(
    "SuperEpic",                       // window title
    SDL_WINDOWPOS_UNDEFINED,           // initial x position
    SDL_WINDOWPOS_UNDEFINED,           // initial y position
    WINDOW_WIDTH,                      // width, in pixels
    WINDOW_HEIGHT,                     // height, in pixels
    SDL_WINDOW_OPENGL                  // flags - see below
    );

  if (window == nullptr) {
    std::cerr << "Coult not create window: " << SDL_GetError() << std::endl;
  }

  SDL_DestroyWindow(window);
  SDL_Quit();


  //KinectSensor::handCoords = new float[3];
  KinectSensor::handCoords[0] = 0.0f;
  KinectSensor::handCoords[1] = 0.0f;
  KinectSensor::handCoords[2] = 0.0f;

  std::thread t = std::thread(&KinectSensor::updateHandPosition);
  int * cursor = { new int[2] };
  while (1) {
	  KinectSensor::mapHandToCursor(KinectSensor::handCoords, 1920, 1080, cursor);
	  std::cout << cursor[0] << "\t\t" << cursor[1] << std::endl;
	  Sleep(1000);
  }
	
  return 0;
}

