

#include "renderer.h"

#include <SDL_image.h>

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>


namespace
{
  const int DEFAULT_WIN_WIDTH{ 640 };
  const int DEFAULT_WIN_HEIGHT{ 480 };
  const int DEFAULT_WIN_X{ SDL_WINDOWPOS_UNDEFINED };
  const int DEFAULT_WIN_Y{ SDL_WINDOWPOS_UNDEFINED };

  const float CURSOR_SCALE{ 0.11f };
  const int NUM_IMAGES_TO_DRAW{ 5 };

  const std::string cursorTexturePath{ "res/crosshair.png" };

} // namespace


////////////////////////////////////////////////////////////////////////////
Renderer::Renderer()
  : Renderer(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, DEFAULT_WIN_X, DEFAULT_WIN_Y)
{
}


////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(int winWidth, int winHeight, int winX, int winY)
  : m_window{ nullptr }
  , m_renderer{ nullptr }
  , m_winDims{ winWidth, winHeight }
  , m_winPos{ winX, winY }
  , m_cursPos{ winWidth/2, winHeight/2 }
  , m_cursDims{ int(winHeight*CURSOR_SCALE), int(winHeight*CURSOR_SCALE) }
  , m_mode{ DisplayMode::Gallery }
  , m_galleryStartIndex{ 0 }
  , m_images{ }
  , m_cursTex{ nullptr }
  , m_imageModeTex{ nullptr }
  , m_fullScreen{ false }
  , m_shouldQuit{ false }
{
}


////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer()
{
  for (auto &tex : m_images)  
    SDL_DestroyTexture(tex);

  if (m_cursTex != nullptr)     
    SDL_DestroyTexture(m_cursTex);
  
  if (m_imageModeTex != nullptr) 
    SDL_DestroyTexture(m_imageModeTex);

  if (m_renderer != nullptr) 
    SDL_DestroyRenderer(m_renderer);
  
  if (m_window != nullptr) 
    SDL_DestroyWindow(m_window);

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

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); // anti-aliasing
  SDL_ShowCursor(0); // don't show mouse arrow.

  SDL_Texture *curtex{ nullptr };
  curtex = IMG_LoadTexture(renderer, cursorTexturePath.c_str());

  if (curtex == nullptr) {
    std::cerr << "Could not load image texture: " << SDL_GetError() << std::endl;
    return -1;
  }

  m_window = window;
  m_renderer = renderer;
  m_cursTex = curtex;

  

  return 0;
}


////////////////////////////////////////////////////////////////////////////
void 
Renderer::loop()
{
  while (!m_shouldQuit) {

    // pop events from the SDL event queue.
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      printEvent(&event);
      onEvent(event);
    } // while(SDL_Poll...

    // TODO: Check mouse coordinates from Kinect class
    SDL_RenderClear(m_renderer);

    switch (m_mode) {
    case DisplayMode::Gallery:
      renderGalleryMode();
      break;
    case DisplayMode::Image:
      renderImageViewMode();
      break;
    }

    renderCursorTexture();

    SDL_RenderPresent(m_renderer);
    
  } // while(!m_shouldQuit)

  std::cout << "Exiting render loop..." << "\n";
}

void 
Renderer::onEvent(const SDL_Event &event)
{
  if (event.type == SDL_MOUSEMOTION) {
    onMouseMotionEvent(event.motion);
  } // if SDL_MOUSEMOTION

  else if (event.type == SDL_WINDOWEVENT) {
    onWindowEvent(event.window);
  }

  else if (event.type == SDL_KEYDOWN) {
    onKeyDown(event.key);
  }

  else if (event.type == SDL_MOUSEBUTTONUP) {
    onMouseButtonUp(event.button);
  }

  else if (event.type == SDL_QUIT) {
    m_shouldQuit = true;
  } 
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::onMouseButtonUp(const SDL_MouseButtonEvent& button)
{
  if (button.button == SDL_BUTTON_LEFT) {
    if (m_mode == DisplayMode::Gallery) {
      int idx{ button.x / (m_winDims.x / NUM_IMAGES_TO_DRAW) };
      m_imageModeTex = m_images[idx];
      m_mode = DisplayMode::Image;
      std::cout << "Switch to Image View (image#: " << idx << ")\n";
    }
  } else if (button.button == SDL_BUTTON_RIGHT) {
    if (m_mode == DisplayMode::Image) {
      m_mode = DisplayMode::Gallery;
      std::cout << "Switch to Gallery View" << "\n";
    }
  }
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::onKeyDown(const SDL_KeyboardEvent& key)
{
  switch (key.keysym.sym) {

  case SDLK_ESCAPE:
    m_shouldQuit = true;
    break;

  case SDLK_f:
    toggleFullScreen();
    break;
  } // switch
}


void
Renderer::onWindowEvent(const SDL_WindowEvent& window)
{
  switch (window.type) {
  case SDL_WINDOWEVENT_RESIZED:
    m_winDims.x = window.data1;
    m_winDims.y = window.data2;
    m_cursDims.x = static_cast<int>(m_winDims.y * 0.11f);
    m_cursDims.y = static_cast<int>(m_winDims.y * 0.11f);
  }
}


void
Renderer::onMouseMotionEvent(const SDL_MouseMotionEvent& motion)
{
  m_cursPos.x = motion.x;
  m_cursPos.y = motion.y;
}



////////////////////////////////////////////////////////////////////////////
void
Renderer::renderGalleryMode()
{
  renderImageTextures();
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderImageViewMode()
{
  int texWidth, texHeight;
  SDL_QueryTexture(m_imageModeTex, nullptr, nullptr, &texWidth, &texHeight);
  float aspect_ratio{ texWidth / static_cast<float>(texHeight) };

  SDL_Rect dest;
  dest.h = m_winDims.y;
  // TODO: width scaling is not quite correct, could be wider than window width.
  dest.w = static_cast<int>(m_winDims.y * aspect_ratio);   // aspect_ratio = w / h --> w = h * aspect_ratio
  dest.x = std::max<int>(m_winDims.x - dest.w, 0) / 2;
  dest.y = 0;
  

  SDL_RenderCopy(m_renderer, m_imageModeTex, nullptr, &dest);
}

void
Renderer::renderCursorTexture()
{
  SDL_Rect dest;
  dest.x = m_cursPos.x - (m_cursDims.x / 2);
  dest.y = m_cursPos.y - (m_cursDims.y / 2);
  dest.w = m_cursDims.x;
  dest.h = m_cursDims.y;

  SDL_RenderCopy(m_renderer, m_cursTex, nullptr, &dest);
}


////////////////////////////////////////////////////////////////////////////
void
Renderer::renderImageTextures()
{
  const int imgWidth{ m_winDims.x / NUM_IMAGES_TO_DRAW };
  const int halfWinY{ m_winDims.y / 2 };
  
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
    dest.h = static_cast<int>(imgWidth / aspect_ratio);  // aspect_ratio = w / h --> h = w / aspect_ratio
    dest.x = xpos;
    dest.y = halfWinY - (dest.h / 2);                    // center image vertically

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


void
Renderer::toggleFullScreen()
{
  if (!m_fullScreen) {
    int success{ SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) };
    
    if (success == 0) {
      m_fullScreen = true;
    }
    else {
      std::cerr << "Switch to fullscreen failed: " << SDL_GetError() << "\n";
    }
  }
  else {
    int success{ SDL_SetWindowFullscreen(m_window, 0) };
    
    if (success == 0) {
      m_fullScreen = false;
    }
    else {
      std::cerr << "Switch to windowed mode failed: " << SDL_GetError() << "\n";
    }
  }
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

void 
Renderer::printEvent(const SDL_Event * event)
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

