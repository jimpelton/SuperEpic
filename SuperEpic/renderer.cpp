#include "renderer.h"
#include "KinectSensor.h"

#include <SDL_image.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

namespace {
const int DEFAULT_WIN_WIDTH{640};
const int DEFAULT_WIN_HEIGHT{480};
const int DEFAULT_WIN_X{SDL_WINDOWPOS_UNDEFINED};
const int DEFAULT_WIN_Y{SDL_WINDOWPOS_UNDEFINED};

const float CURSOR_SCALE{0.11f};
const float DEFAULT_CURSOR_SPEED{1.0f};

const int NUM_IMAGES_TO_DRAW{5};

const char *DEFAULT_CURSOR_TEXTURE_PATH{"../res/crosshair.png"};

} // namespace

#define TEX_FROM_GAL_IDX(_idx)                                                 \
  m_images[(m_galleryStartIndex + (_idx)) % m_images.size()]

#define SCREEN_TO_GAL_IDX(_screen_coords)                                      \
  (_screen_coords) / (m_winDims.x / NUM_IMAGES_TO_DRAW)

////////////////////////////////////////////////////////////////////////////
Renderer::Renderer()
    : Renderer(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, DEFAULT_WIN_X,
               DEFAULT_WIN_Y) {}

////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(int winWidth, int winHeight, int winX, int winY)
    : m_window{nullptr}, m_renderer{nullptr}, m_winDims{winWidth, winHeight},
      m_winPos{winX, winY}, m_cursPos{winWidth / 2, winHeight / 2},
      m_cursDims{int(winHeight * CURSOR_SCALE), int(winHeight * CURSOR_SCALE)},
      m_cursorSpeed{DEFAULT_CURSOR_SPEED}, m_mode{DisplayMode::Gallery},
      m_galleryStartIndex{0}, m_images{}, m_cursTex{nullptr},
      m_imageModeTex{nullptr}, m_fullScreen{false}, m_clickCount{0},
      m_selected{false}, m_srcImageRect{0, 0, 0, 0},
      m_destWindowRect{0, 0, 0, 0}, m_imageScreenRatio{0},
      m_windowWidthLeastIncrement{0}, m_windowHeightLeastIncrement{0},
      m_imageHeightLeastIncrement{0}, m_imageWidthLeastIncrement{0},
      m_shouldQuit{false} {}

////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer() {
  for (auto &tex : m_images)
    SDL_DestroyTexture(tex);

  if (m_cursTex != nullptr)
    SDL_DestroyTexture(m_cursTex);

  if (m_renderer != nullptr)
    SDL_DestroyRenderer(m_renderer);

  if (m_window != nullptr)
    SDL_DestroyWindow(m_window);

  SDL_Quit();
}

////////////////////////////////////////////////////////////////////////////
void Renderer::loadImages(const std::vector<std::string> &images) {
  for (auto &file : images) {
    loadSingleTexture(file);
  }
}

////////////////////////////////////////////////////////////////////////////
int Renderer::init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return -1;
  }

  // Set log messages
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

  SDL_Window *window{nullptr};
  window = SDL_CreateWindow("SuperEpic",      // window title
                            m_winPos.x,       // initial x position
                            m_winPos.y,       // initial y position
                            m_winDims.x,      // width, in pixels
                            m_winDims.y,      // height, in pixels
                            SDL_WINDOW_OPENGL // flags
                            );

  if (window == nullptr) {
    std::cerr << "Coult not create window: " << SDL_GetError() << "\n";
    return -1;
  }

  SDL_Renderer *renderer{nullptr};
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);

  if (renderer == nullptr) {
    std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << "\n";
    return -1;
  }

  SDL_Texture *curtex{nullptr};
  curtex = IMG_LoadTexture(renderer, DEFAULT_CURSOR_TEXTURE_PATH);

  if (curtex == nullptr) {
    std::cerr << "Could not load image texture: " << SDL_GetError()
              << std::endl;
    return -1;
  }

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); // anti-aliasing
  SDL_ShowCursor(0);                                 // don't show mouse arrow.

  m_window = window;
  m_renderer = renderer;
  m_cursTex = curtex;

  return 0;
}

////////////////////////////////////////////////////////////////////////////
void Renderer::loop() {
  while (!m_shouldQuit) {

    // Pop events from the SDL event queue.
    // When we start using the kinect to control, this may get replaced, or
    // this is probably were the kinect gesture events will get checked.
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      printEvent(&event);
      onEvent(event);
    }

    if (m_useKinectForCursorPos) {
      KinectSensor::mapHandToCursor(KinectSensor::handCoords, m_winDims.x,
                                    m_winDims.y,
                                    reinterpret_cast<int *>(&m_cursPos));
    }

    SDL_RenderClear(m_renderer);

    switch (m_mode) {
    case DisplayMode::Gallery:
      renderGalleryMode();
      break;
    case DisplayMode::FromGalleryToImage:
      renderTransactionMode();
      break;
    case DisplayMode::Image:
      renderImageViewMode();
      break;
    }

    renderCursorTexture();

    SDL_RenderPresent(m_renderer);

  } // while(!m_shouldQuit)

  std::cout << "Exiting render loop..."
            << "\n";
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onEvent(const SDL_Event &event) {
  if (event.type == SDL_MOUSEMOTION) {
    onMouseMotionEvent(event.motion);
  }

  else if (event.type == SDL_WINDOWEVENT) {
    onWindowEvent(event.window);
  }

  else if (event.type == SDL_KEYDOWN) {
    onKeyDown(event.key);
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    onMouseButtonUp(event.button);
  } else if (event.type == SDL_MOUSEWHEEL) {
    onMouseWheelEvent(event.wheel);
  } else if (event.type == SDL_QUIT) {
    m_shouldQuit = true;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onMouseButtonUp(const SDL_MouseButtonEvent &button) {
  if (button.button == SDL_BUTTON_LEFT) {
    if (m_mode == DisplayMode::Gallery) {
      if (m_cursorImageHoverIndex == m_previousImageHoverIndex) {
        m_clickCount++;
      }
      if (m_clickCount == 1) {
        m_selected = true;
      } else if (m_clickCount == 2) {
        m_clickCount = 0;
        m_selected = false;

        int idx = SCREEN_TO_GAL_IDX(button.x);
        m_imageModeTex = TEX_FROM_GAL_IDX(idx);

        SDL_QueryTexture(m_imageModeTex, nullptr, nullptr, &m_srcImageRect.w,
                         &m_srcImageRect.h);
        m_srcImageRect.x = m_srcImageRect.y = 0;
        m_destWindowRect.x = (m_winDims.x / 5) * 2;
        m_destWindowRect.y = (m_winDims.y / 5) * 2;
        m_destWindowRect.h = m_winDims.y / 5;
        m_destWindowRect.w = m_winDims.x / 5;
        m_imageScreenRatio = 0;
        findLeastIncrement(m_destWindowRect.w, m_destWindowRect.h, true);
        findLeastIncrement(m_srcImageRect.w, m_srcImageRect.h, false);
        smoothIncrement();

        m_mode = DisplayMode::FromGalleryToImage;
        std::cout << "Switch to Image View (image#: " << idx << ")\n";
      }
    }
  } else if (button.button == SDL_BUTTON_RIGHT) {
    if (m_mode == DisplayMode::Image) {
      m_mode = DisplayMode::Gallery;
      std::cout << "Switch to Gallery View"
                << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onKeyDown(const SDL_KeyboardEvent &key) {
  switch (key.keysym.sym) {
  case SDLK_ESCAPE:
    if (m_mode == DisplayMode::Image) {
      m_mode = DisplayMode::Gallery;
    } else {
      m_shouldQuit = true;
    }
    break;
  case SDLK_LEFT:
    // scroll images left one index, wraps if start index is < 0.
    if (m_mode == DisplayMode::Gallery) {
      int i{m_galleryStartIndex - 1};
      m_galleryStartIndex = i < 0 ? static_cast<int>(m_images.size()) - 1 : i;
    }
    break;
  case SDLK_RIGHT:
    // scroll images right one index, wraps if start index > m_images.size()-1
    if (m_mode == DisplayMode::Gallery) {
      m_galleryStartIndex = (m_galleryStartIndex + 1) % m_images.size();
    }
    break;
  case SDLK_f:
    toggleFullScreen();
    break;
  case SDLK_k:
    m_useKinectForCursorPos = !m_useKinectForCursorPos;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onWindowEvent(const SDL_WindowEvent &window) {
  switch (window.event) {

  case SDL_WINDOWEVENT_SIZE_CHANGED:
    m_winDims.x = window.data1;
    m_winDims.y = window.data2;
    m_cursDims.x = static_cast<int>(m_winDims.y * 0.11f);
    m_cursDims.y = static_cast<int>(m_winDims.y * 0.11f);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onMouseMotionEvent(const SDL_MouseMotionEvent &motion) {
  m_cursPos.x = motion.x;
  m_cursPos.y = motion.y;
  m_previousImageHoverIndex = m_cursorImageHoverIndex;
  m_cursorImageHoverIndex = SCREEN_TO_GAL_IDX(
      motion.x); // motion.x / (m_winDims.x / NUM_IMAGES_TO_DRAW);
  if (m_previousImageHoverIndex != m_cursorImageHoverIndex) {
    m_clickCount = 0;
    m_selected = false;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onMouseWheelEvent(const SDL_MouseWheelEvent &event) {
  if (m_mode == DisplayMode::Image) {
    int scrollDeg = event.y;
    if (event.direction == SDL_MOUSEWHEEL_FLIPPED) {
      scrollDeg *= -1;
    }
    if (scrollDeg < 0) { // scroll backward - zoom out
      if (m_imageScreenRatio <= 0) {
        m_destWindowRect.h -= m_windowHeightLeastIncrement * 2;
        m_destWindowRect.w -= m_windowWidthLeastIncrement * 2;
        m_destWindowRect.y += m_windowHeightLeastIncrement;
        m_destWindowRect.x += m_windowWidthLeastIncrement;
        if (m_destWindowRect.w <= (m_winDims.x / 5)) {
          m_mode = DisplayMode::Gallery;
          std::cout << "Switch to Gallery View"
                    << "\n";
        }
      } else {
        m_srcImageRect.h += m_imageHeightLeastIncrement * 2;
        m_srcImageRect.w += m_imageWidthLeastIncrement * 2;
        m_srcImageRect.y -= m_imageHeightLeastIncrement;
        m_srcImageRect.x -= m_imageWidthLeastIncrement;
      }
      m_imageScreenRatio--;
    } else if (scrollDeg > 0) { // scroll forward - zoom in
      if (m_imageScreenRatio >= 0) {
        m_srcImageRect.h -= m_imageHeightLeastIncrement * 2;
        m_srcImageRect.w -= m_imageWidthLeastIncrement * 2;
        m_srcImageRect.y += m_imageHeightLeastIncrement;
        m_srcImageRect.x += m_imageWidthLeastIncrement;
      } else {
        m_destWindowRect.h += m_windowHeightLeastIncrement * 2;
        m_destWindowRect.w += m_windowWidthLeastIncrement * 2;
        m_destWindowRect.y -= m_windowHeightLeastIncrement;
        m_destWindowRect.x -= m_windowWidthLeastIncrement;
      }
      m_imageScreenRatio++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderGalleryMode() const { renderImageTextures(); }

////////////////////////////////////////////////////////////////////////////
void Renderer::renderTransactionMode() {
  if (m_destWindowRect.x >= 0 + m_windowWidthLeastIncrement &&
      m_destWindowRect.y >= 0 + m_windowHeightLeastIncrement &&
      m_destWindowRect.w <= m_winDims.x - m_windowWidthLeastIncrement * 2 &&
      m_destWindowRect.h <= m_winDims.y - m_windowHeightLeastIncrement * 2) {
    m_destWindowRect.x -= m_windowWidthLeastIncrement;
    m_destWindowRect.y -= m_windowHeightLeastIncrement;
    m_destWindowRect.w += m_windowWidthLeastIncrement * 2;
    m_destWindowRect.h += m_windowHeightLeastIncrement * 2;
  } else {
    m_mode = DisplayMode::Image;
    m_destWindowRect.x = m_destWindowRect.y = 0;
    m_destWindowRect.w = m_winDims.x;
    m_destWindowRect.h = m_winDims.y;
  }
  SDL_RenderCopy(m_renderer, m_imageModeTex, &m_srcImageRect,
                 &m_destWindowRect);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderImageViewMode() const {
  SDL_RenderCopy(m_renderer, m_imageModeTex, &m_srcImageRect,
                 &m_destWindowRect);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderCursorTexture() const {
  SDL_Rect dest;
  dest.x = m_cursPos.x - (m_cursDims.x / 2);
  dest.y = m_cursPos.y - (m_cursDims.y / 2);
  dest.w = m_cursDims.x;
  dest.h = m_cursDims.y;

  SDL_RenderCopy(m_renderer, m_cursTex, nullptr, &dest);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderImageTextures() const {
  const int imgWidth{m_winDims.x / NUM_IMAGES_TO_DRAW};
  const int halfWinY{m_winDims.y / 2};

  int xpos{0};

  int idx{0};
  while (idx < NUM_IMAGES_TO_DRAW) {
    SDL_Texture *tex{TEX_FROM_GAL_IDX(idx)};
    int texWidth, texHeight;
    SDL_QueryTexture(tex, nullptr, nullptr, &texWidth, &texHeight);
    float aspect_ratio{texWidth / static_cast<float>(texHeight)};

    SDL_Rect dest;
    dest.w = imgWidth;
    dest.h = static_cast<int>(
        imgWidth /
        aspect_ratio); // aspect_ratio = w / h --> h = w / aspect_ratio
    dest.x = xpos;
    dest.y = halfWinY - (dest.h / 2); // center image vertically

    SDL_RenderCopy(m_renderer, tex, nullptr, &dest);

    if (idx == m_cursorImageHoverIndex && m_selected) {
      renderImageSelectionRectangle(dest);
    }

    idx += 1;
    xpos += imgWidth;
  }
}

////////////////////////////////////////////////////////////////////////////
// void
// Renderer::renderSingleTexture(SDL_Texture* tex, int x, int y, int w, int h)
// const
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
void Renderer::renderImageSelectionRectangle(const SDL_Rect &dest) const {
  // save the clear color so it can be restored after DrawRect()
  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 0);
  SDL_RenderDrawRect(m_renderer, &dest);
  SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::toggleFullScreen() {
  if (!m_fullScreen) {
    int success{
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP)};

    if (success == 0) {
      m_fullScreen = true;
    } else {
      std::cerr << "Switch to fullscreen failed: " << SDL_GetError() << "\n";
    }
  } else {
    int success{SDL_SetWindowFullscreen(m_window, 0)};

    if (success == 0) {
      m_fullScreen = false;
    } else {
      std::cerr << "Switch to windowed mode failed: " << SDL_GetError() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::loadSingleTexture(const std::string &path) {
  SDL_Texture *tex{IMG_LoadTexture(m_renderer, path.c_str())};

  if (tex == nullptr) {
    std::cerr << "Could not load image texture: " << SDL_GetError()
              << std::endl;
    return;
  }

  std::cout << "Loaded image: " << path << "\n";
  m_images.push_back(tex);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::printEvent(const SDL_Event *event) const {
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
      printf("Window %d moved to %d,%d\n", event->window.windowID,
             event->window.data1, event->window.data2);
      break;
    case SDL_WINDOWEVENT_RESIZED:
      printf("Window %d resized to %dx%d\n", event->window.windowID,
             event->window.data1, event->window.data2);
      break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      printf("Window %d size changed to %dx%d\n", event->window.windowID,
             event->window.data1, event->window.data2);
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
      printf("Mouse entered window %d\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_LEAVE:
      printf("Mouse left window %d\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
      printf("Window %d gained keyboard focus\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
      printf("Window %d lost keyboard focus\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_CLOSE:
      printf("Window %d closed\n", event->window.windowID);
      break;
    default:
      printf("Window %d got unknown event %d\n", event->window.windowID,
             event->window.event);
      break;
    }
  }
}

void Renderer::findLeastIncrement(int width, int height, bool isWindow) {
  int a = width, b = height, c;
  if (a < b) {
    c = b % a;
    while (c != 0) {
      b = a;
      a = c;
      c = b % a;
    }
  } else {
    c = a % b;
    while (c != 0) {
      a = b;
      b = c;
      c = a % b;
    }
  }
  if (isWindow) {
    m_windowWidthLeastIncrement = width / b;
    m_windowHeightLeastIncrement = height / b;
  } else {
    m_imageWidthLeastIncrement = width / b;
    m_imageHeightLeastIncrement = height / b;
  }
}

void Renderer::smoothIncrement() {
  if (m_windowHeightLeastIncrement > m_imageHeightLeastIncrement) {
    m_imageWidthLeastIncrement *=
        m_windowHeightLeastIncrement / m_imageHeightLeastIncrement;
    m_imageHeightLeastIncrement *=
        m_windowHeightLeastIncrement / m_imageHeightLeastIncrement;
  } else {
    m_windowWidthLeastIncrement *=
        m_imageHeightLeastIncrement / m_windowHeightLeastIncrement;
    m_windowHeightLeastIncrement *=
        m_imageHeightLeastIncrement / m_windowHeightLeastIncrement;
  }
}