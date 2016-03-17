

#include "renderer.h"

#include <SDL_image.h>

#include <iostream>
#include <cassert>
#include <algorithm>


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
  , m_mode{ DisplayMode::Gallery }
  , m_galleryStartIndex{ 0 }
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
  bool fullScreen{ false };

  while (!shouldQuit) {

    // pop events from the SDL event queue.
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      PrintEvent(&event);
      
      if (event.type == SDL_QUIT) {
        shouldQuit = true;
      }

      else if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
        
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          m_winDims.x = event.window.data1;
          m_winDims.y = event.window.data2;
          break;
        
        } // switch
      }

      else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        
        case SDLK_ESCAPE:
          shouldQuit = true;
          break;

        case SDLK_f:
          if (!fullScreen) {
            SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            fullScreen = true;
          }
          else {
            SDL_SetWindowFullscreen(m_window, 0);
            fullScreen = false;
          }
          break;
        } // switch
      }
    } // while(SDL_Poll...

    
    // TODO: Check gesture coordinates
    

    if (m_mode == DisplayMode::Gallery) {
      renderGalleryMode();
    } 
    else {
      renderImageMode();
    }

    SDL_RenderPresent(m_renderer);
    
  }

  std::cout << "Exiting render loop..." << "\n";
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderGalleryMode()
{
  renderTextures();
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderImageMode()
{
  // renderSingleTexture();
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderTextures()
{
  int imgWidth{ m_winDims.x / NUM_IMAGES_TO_DRAW };
  float halfWinY{ m_winDims.y * 0.5f };
  
  int xpos{ 0 };

  // get the range of images we want to view
  auto beg = std::begin(m_images) + m_galleryStartIndex;
  auto end = std::end(m_images);
  if (std::distance(beg, end) > NUM_IMAGES_TO_DRAW) {
    end = beg + NUM_IMAGES_TO_DRAW;
  }
  
  std::for_each(beg, end,
    [&](SDL_Texture *tex)
  {
    int texWidth, texHeight;
    SDL_QueryTexture(tex, nullptr, nullptr, &texWidth, &texHeight);
    float aspect_ratio{ texWidth / static_cast<float>(texHeight) };

    SDL_Rect dest;
    dest.w = imgWidth;
    dest.h = static_cast<int>(imgWidth / aspect_ratio);
    dest.x = xpos;
    dest.y = static_cast<int>(halfWinY - (dest.h*0.5f));  // center image vertically

    SDL_RenderCopy(m_renderer, tex, nullptr, &dest);
    xpos += imgWidth;
  });
  
}


////////////////////////////////////////////////////////////////////////////
//void
//Renderer::renderSingleTexture(SDL_Texture* tex, int x, int y, int w, int h) const
//{
//  assert(tex != nullptr);
//
//  SDL_Rect dest;
//  dest.x = x; 
//  dest.y = y;
//  dest.w = w;
//  dest.h = h;
//  SDL_RenderCopy(m_renderer, tex, nullptr, &dest);
//}


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

void 
Renderer::PrintEvent(const SDL_Event * event)
{
  if (event->type == SDL_WINDOWEVENT) {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SHOWN:
      printf("Window %d shown\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_HIDDEN:
      printf("Window %d hidden\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_EXPOSED:
      printf("Window %d exposed\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_MOVED:
      printf("Window %d moved to %d,%d\n",
        event->window.windowID, event->window.data1,
        event->window.data2);
      break;
    case SDL_WINDOWEVENT_RESIZED:
      printf("Window %d resized to %dx%d\n",
        event->window.windowID, event->window.data1,
        event->window.data2);
      break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      printf("Window %d size changed to %dx%d\n",
        event->window.windowID, event->window.data1,
        event->window.data2);
      break;
    case SDL_WINDOWEVENT_MINIMIZED:
      printf("Window %d minimized\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_MAXIMIZED:
      printf("Window %d maximized\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_RESTORED:
      printf("Window %d restored\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_ENTER:
      printf("Mouse entered window %d\n",
        event->window.windowID);
      break;
    case SDL_WINDOWEVENT_LEAVE:
      printf("Mouse left window %d\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
      printf("Window %d gained keyboard focus\n",
        event->window.windowID);
      break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
      printf("Window %d lost keyboard focus\n",
        event->window.windowID);
      break;
    case SDL_WINDOWEVENT_CLOSE:
      printf("Window %d closed\n", event->window.windowID);
      break;
    default:
      printf("Window %d got unknown event %d\n",
        event->window.windowID, event->window.event);
      break;
    }
  }

}

