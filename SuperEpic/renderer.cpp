

#include "renderer.h"
#include "logger.h"

#include <SDL_image.h>

#include <iostream>


namespace
{
  const int INIT_WIN_WIDTH{ 640 };
  const int INIT_WIN_HEIGHT{ 480 };
  const int INIT_WIN_X{ SDL_WINDOWPOS_UNDEFINED };
  const int INIT_WIN_Y{ SDL_WINDOWPOS_UNDEFINED };

  const int NUM_IMAGES_TO_DRAW{ 5 };
} // namespace


////////////////////////////////////////////////////////////////////////////
Renderer::Renderer()
  : Renderer(INIT_WIN_WIDTH, INIT_WIN_HEIGHT, INIT_WIN_X, INIT_WIN_Y)
{
}


////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(int winWidth, int winHeight, int winX, int winY)
  : m_window{ nullptr }
  , m_renderer{ nullptr }
  , m_winDims{ winWidth, winHeight }
  , m_winPos{ winX, winY }
{
}


////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer()
{

  for (auto &tex : m_images) {
    SDL_DestroyTexture(tex);
  }

  if (m_renderer != nullptr) {
    SDL_DestroyRenderer(m_renderer);
  }

  if (m_window != nullptr) {
    SDL_DestroyWindow(m_window);
  }

  SDL_Quit();
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::loadImages(const std::vector<std::string>& images)
{
  for(auto &file : images) {
    loadSingleTexture(file);
  }
}


////////////////////////////////////////////////////////////////////////////
int 
Renderer::init()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return -1;
  }

  // Set log messages
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

  SDL_Window *window{ nullptr };
  window = SDL_CreateWindow(
    "SuperEpic",                       // window title
    m_winPos.x,                        // initial x position
    m_winPos.y,                        // initial y position
    m_winDims.x,                       // width, in pixels
    m_winDims.y,                       // height, in pixels
    SDL_WINDOW_OPENGL                  // flags
    );

  if (window == nullptr) {
    std::cerr << "Coult not create window: " << SDL_GetError() << "\n";
    return -1;
  }

  SDL_Renderer *renderer{ nullptr };
  renderer = SDL_CreateRenderer(window, -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if(renderer == nullptr) {
    std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << "\n";
    return -1;
  }

  m_window = window;
  m_renderer = renderer;

  return 0;
}


////////////////////////////////////////////////////////////////////////////
void 
Renderer::loop()
{
  bool shouldQuit{ false };
  while (!shouldQuit) {

    // pop events from the SDL event queue.
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
      case SDL_QUIT:
        shouldQuit = true;
        break;
      }
    }

    // Check gesture coordinates
    // ...

    renderTextures();

    SDL_RenderPresent(m_renderer);
    
  }

  std::cout << "Exiting render loop..." << "\n";
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderTextures()
{
  const int imgWidth{ m_winDims.x / NUM_IMAGES_TO_DRAW };
  const float halfWin{ m_winDims.y * 0.5f };
  int xpos{ 0 };
  for (auto &tex : m_images) {
    int texWidth, texHeight;
    SDL_QueryTexture(tex, nullptr, nullptr, &texWidth, &texHeight);
    
    SDL_Rect dest;
    dest.w = imgWidth;
    dest.h = imgWidth / (texWidth / (float)texHeight);  // h = width / aspect_ratio
    dest.x = xpos;
    dest.y = halfWin - (dest.h*0.5f);  // center image vertically
    
    SDL_RenderCopy(m_renderer, tex, nullptr, &dest);
    xpos += imgWidth;
  }
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderSingleTexture(SDL_Texture* tex, int x, int y, int w, int h) const
{
  //TODO: need to scale texture.
  SDL_Rect dest;
  dest.x = x; 
  dest.y = y;
  dest.w = w;
  dest.h = h;
  SDL_RenderCopy(m_renderer, tex, nullptr, &dest);
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::loadSingleTexture(const std::string &path)
{
  SDL_Texture *tex{ IMG_LoadTexture(m_renderer, path.c_str()) };
  
  if (tex == nullptr) {
    std::cerr << "Could not load image texture: " << SDL_GetError() << std::endl;
    return;
  }

  std::cout << "Loaded image: " << path << "\n";
  m_images.push_back(tex);
}



