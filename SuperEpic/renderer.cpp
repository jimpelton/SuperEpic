
#include "renderer.h"

#ifdef WIN32
#include "KinectSensor.h"
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

namespace {
const int DEFAULT_WIN_WIDTH{640};
const int DEFAULT_WIN_HEIGHT{480};
const int DEFAULT_WIN_X{SDL_WINDOWPOS_UNDEFINED};
const int DEFAULT_WIN_Y{SDL_WINDOWPOS_UNDEFINED};

const float DEFAULT_CURSOR_SCALE{0.11f};
const float DEFAULT_CURSOR_SPEED{1.0f};

const int NUM_IMAGES_TO_DRAW{5};

const char *DEFAULT_CURSOR_TEXTURE_PATH{"../res/crosshair.png"};

} // namespace

#define IMG_FROM_GAL_IDX(_idx)                                                 \
  m_images[(m_galleryStartIndex + (_idx)) % m_images.size()]

/// \brief Convert screen coords to a Gallery View index
#define SCREEN_COORD_TO_GAL_IDX(_screen_coords)                                \
  (_screen_coords) / (m_winDims.x / NUM_IMAGES_TO_DRAW)

////////////////////////////////////////////////////////////////////////////
Renderer::Renderer()
    : Renderer(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, DEFAULT_WIN_X,
               DEFAULT_WIN_Y) {}

////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(int winWidth, int winHeight, int winX, int winY)
    : m_window{nullptr}, m_renderer{nullptr}, m_winDims{winWidth, winHeight},
      m_winPos{winX, winY}, m_cursorSpeed{DEFAULT_CURSOR_SPEED},
      m_cursor{new Cursor()}, m_mode{DisplayMode::Gallery},
      m_galleryStartIndex{0}, m_images{}, m_imageModeImage{nullptr},
      m_fullScreen{false}, m_shouldQuit{false}, m_useKinectForCursorPos{false}

      ,
      m_clickCount{0}, m_selected{false} //  , m_srcImageRect{ 0, 0, 0, 0 }
//  , m_destWindowRect{ 0, 0, 0, 0 }
//  , m_imageScreenRatio{ 0 }
//  , m_windowHeightLeastIncrement{ 0 }
//  , m_windowWidthLeastIncrement{ 0 }
//  , m_imageHeightLeastIncrement{ 0 }
//  , m_imageWidthLeastIncrement{ 0 }
{}

////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer() {
  for (auto img : m_images)
    delete img;

  if (m_cursor != nullptr)
    delete m_cursor;

  if (m_renderer != nullptr)
    SDL_DestroyRenderer(m_renderer);

  if (m_window != nullptr)
    SDL_DestroyWindow(m_window);

  SDL_Quit();
}

////////////////////////////////////////////////////////////////////////////
void Renderer::loadImages(const std::vector<std::string> &images) {
  for (auto &file : images) {
    Image *img{Image::load(file)};

    if (!img) {
      std::cerr << "Could not load image texture: " << SDL_GetError()
                << std::endl;
      return;
    }

    std::cout << "Loaded image: " << file << "\n";
    m_images.push_back(img);
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

  m_window = SDL_CreateWindow("SuperEpic",      // sdl_window title
                              m_winPos.x,       // initial x position
                              m_winPos.y,       // initial y position
                              m_winDims.x,      // width, in pixels
                              m_winDims.y,      // height, in pixels
                              SDL_WINDOW_OPENGL // flags
                              );

  if (m_window == nullptr) {
    std::cerr << "Coult not create sdl_window: " << SDL_GetError() << "\n";
    return -1;
  }

  Image::sdl_window(m_window);

  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED |
                                                    SDL_RENDERER_PRESENTVSYNC);

  if (m_renderer == nullptr) {
    std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << "\n";
    return -1;
  }

  Image::sdl_renderer(m_renderer);

  Image *cursImg = Image::load(DEFAULT_CURSOR_TEXTURE_PATH);
  if (cursImg == nullptr) {
    std::cerr << "Could not load image texture: " << SDL_GetError()
              << std::endl;
    return -1;
  }
  m_cursor->setImage(cursImg);
  m_cursor->setSize(static_cast<int>(m_winDims.x * DEFAULT_CURSOR_SCALE),
                    static_cast<int>(m_winDims.x * DEFAULT_CURSOR_SCALE));

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); // anti-aliasing
  SDL_ShowCursor(0);                                 // don't show mouse arrow.

  return 0;
}

////////////////////////////////////////////////////////////////////////////
void Renderer::loop() {
  float since = 0.0f; // seconds since last loop iteration
  float then = 0.0f;
  while (!m_shouldQuit) {
    since = SDL_GetTicks() * 1e-3f - then; // seconds

    // Pop events from the SDL event queue.
    // When we start using the kinect to control, this may get replaced, or
    // this is probably were the kinect gesture events will get checked.
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      printEvent(&event);
      onEvent(event);
    }
#ifdef WIN32
    if (m_useKinectForCursorPos) {
      SDL_Point pos;
      KinectSensor::mapHandToCursor(KinectSensor::handCoords, m_winDims.x,
                                    m_winDims.y, reinterpret_cast<int *>(&pos));
      m_cursor->setPos(pos.x, pos.y);
    }
#endif

    SDL_RenderClear(m_renderer);

    switch (m_mode) {
    case DisplayMode::Gallery:
      renderGalleryMode();
      break;
    case DisplayMode::FromGalleryToImage:
      renderTransitionMode(since, m_targetScale);
      break;
    case DisplayMode::Image:
      renderImageViewMode();
      break;
    }

    renderCursorTexture();

    SDL_RenderPresent(m_renderer);
    then = since;
  } // while(!m_shouldQuit)

  std::cout << "Exiting render loop\n";
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onEvent(const SDL_Event &event) {
  if (event.type == SDL_MOUSEMOTION) {
    onMouseMotionEvent(event.motion);
  } else if (event.type == SDL_WINDOWEVENT) {
    onWindowEvent(event.window);
  } else if (event.type == SDL_KEYDOWN) {
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
      if (m_currentImageHoverIndex == m_previousImageHoverIndex) {
        m_clickCount++;
      }
      if (m_clickCount == 1) { // image selected under cursor
        m_selected = true;
      } else if (m_clickCount == 2) { // switch to image view mode.
        // m_mode = DisplayMode::Image;
        m_mode = DisplayMode::FromGalleryToImage;
        m_clickCount = 0;
        m_selected = false;

        int idx = SCREEN_COORD_TO_GAL_IDX(button.x);
        m_imageModeImage = IMG_FROM_GAL_IDX(idx);
        m_imageModeImage->maximize();
        m_targetScale = m_imageModeImage->getScaleFactor();
        m_imageModeImage->scale(0.0f);
        // m_imageModeImage->maximize();
        //        SDL_QueryTexture(m_imageModeImage->getTexture(), nullptr,
        //        nullptr,
        //                         &m_srcImageRect.w,
        //                         &m_srcImageRect.h);

        //        m_srcImageRect.x = m_srcImageRect.y = 0;
        //        m_destWindowRect.x = (m_winDims.x / 5) * 2;
        //        m_destWindowRect.y = (m_winDims.y / 5) * 2;
        //        m_destWindowRect.h = m_winDims.y / 5;
        //        m_destWindowRect.w = m_winDims.x / 5;
        //        m_imageScreenRatio = 0;
        //        findLeastIncrement(m_destWindowRect.w, m_destWindowRect.h,
        //        true);
        //        findLeastIncrement(m_srcImageRect.w, m_srcImageRect.h, false);
        //        smoothIncrement();

        //        m_mode = DisplayMode::FromGalleryToImage;
        //        std::cout << "Switch to Image View (image#: " << idx << ")\n";
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
      m_mode = DisplayMode::Gallery; // go back to gallery.
    } else {
      m_shouldQuit = true; // if in gallery, then quit.
    }
    break;
  case SDLK_LEFT:
    // scroll images left one index, wraps if start index is < 0.
    if (m_mode == DisplayMode::Gallery) {
      m_galleryStartIndex = m_galleryStartIndex - 1 < 0
                                ? static_cast<int>(m_images.size()) - 1
                                : m_galleryStartIndex - 1;
    } else if (m_mode == DisplayMode::Image) {
      SDL_Point delta{10, 0};
      m_imageModeImage->panBy(delta);
    }
    break;
  case SDLK_RIGHT:
    // scroll images right one index, wraps if start index > m_images.size()-1
    if (m_mode == DisplayMode::Gallery) {
      m_galleryStartIndex =
          static_cast<int>((m_galleryStartIndex + 1) % m_images.size());
    } else if (m_mode == DisplayMode::Image) {
      SDL_Point delta{-10, 0};
      m_imageModeImage->panBy(delta);
    }
    break;
  case SDLK_UP:
    if (m_mode == DisplayMode::Image) {
      SDL_Point delta{0, 10};
      m_imageModeImage->panBy(delta);
    }
    break;
  case SDLK_DOWN:
    if (m_mode == DisplayMode::Image) {
      SDL_Point delta{0, -10};
      m_imageModeImage->panBy(delta);
    }
    break;
  case SDLK_f:
    toggleFullScreen();
    break;
  case SDLK_k:
    m_useKinectForCursorPos = !m_useKinectForCursorPos;
    break;
  case SDLK_z:
    if (m_mode == DisplayMode::Image) {
      float currentScaleFactor = m_imageModeImage->getScaleFactor() - 0.01f;
      m_imageModeImage->scale(currentScaleFactor);
    }
    break;
  case SDLK_a:
    if (m_mode == DisplayMode::Image) {
      float currentScaleFactor = m_imageModeImage->getScaleFactor() + 0.01f;
      m_imageModeImage->scale(currentScaleFactor);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onWindowEvent(const SDL_WindowEvent &window) {
  switch (window.event) {

  case SDL_WINDOWEVENT_SIZE_CHANGED:
    m_winDims.x = window.data1;
    m_winDims.y = window.data2;

    m_cursor->setSize(static_cast<int>(m_winDims.y * DEFAULT_CURSOR_SCALE),
                      static_cast<int>(m_winDims.y * DEFAULT_CURSOR_SCALE));

    if (m_mode == DisplayMode::Image) {
      m_imageModeImage->maximize();
    }

    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onMouseMotionEvent(const SDL_MouseMotionEvent &motion) {
  m_cursor->setPos(motion.x, motion.y);
  m_previousImageHoverIndex = m_currentImageHoverIndex;
  m_currentImageHoverIndex = SCREEN_COORD_TO_GAL_IDX(motion.x);
  if (m_previousImageHoverIndex != m_currentImageHoverIndex) {
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
    //    m_imageModeImage->scale(1 + (scrollDeg * 0.5));
    //    if (scrollDeg < 0) { // scroll backward - zoom out
    //      if (m_imageScreenRatio <= 0) {
    //        m_destWindowRect.h -= m_windowHeightLeastIncrement * 2;
    //        m_destWindowRect.w -= m_windowWidthLeastIncrement * 2;
    //        m_destWindowRect.y += m_windowHeightLeastIncrement;
    //        m_destWindowRect.x += m_windowWidthLeastIncrement;
    //        if (m_destWindowRect.w <= (m_winDims.x / 5)) {
    //          m_mode = DisplayMode::Gallery;
    //          std::cout << "Switch to Gallery View"
    //              << "\n";
    //        }
    //      } else {
    //        m_srcImageRect.h += m_imageHeightLeastIncrement * 2;
    //        m_srcImageRect.w += m_imageWidthLeastIncrement * 2;
    //        m_srcImageRect.y -= m_imageHeightLeastIncrement;
    //        m_srcImageRect.x -= m_imageWidthLeastIncrement;
    //      }
    //      m_imageScreenRatio--;
    //    } else if (scrollDeg > 0) { // scroll forward - zoom in
    //      if (m_imageScreenRatio >= 0) {
    //        m_srcImageRect.h -= m_imageHeightLeastIncrement * 2;
    //        m_srcImageRect.w -= m_imageWidthLeastIncrement * 2;
    //        m_srcImageRect.y += m_imageHeightLeastIncrement;
    //        m_srcImageRect.x += m_imageWidthLeastIncrement;
    //      } else {
    //        m_destWindowRect.h += m_windowHeightLeastIncrement * 2;
    //        m_destWindowRect.w += m_windowWidthLeastIncrement * 2;
    //        m_destWindowRect.y -= m_windowHeightLeastIncrement;
    //        m_destWindowRect.x -= m_windowWidthLeastIncrement;
    //      }
    //      m_imageScreenRatio++;
    //    }
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderGalleryMode() { renderImageTextures(); }

////////////////////////////////////////////////////////////////////////////
void Renderer::renderTransitionMode(float secondsSinceLastUpdate,
                                    float targetScale) {
  float zoom_speed = 0.001f;

  float scale = m_imageModeImage->getScaleFactor() +
                (zoom_speed * secondsSinceLastUpdate);
  if (scale > targetScale) {
    m_imageModeImage->scale(targetScale);
    m_imageModeImage->setBaseScaleFactor(targetScale);
    m_mode = DisplayMode::Image;
    return;
  }

  m_imageModeImage->scale(scale);
  m_imageModeImage->draw();
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderImageViewMode() const { m_imageModeImage->draw(); }

////////////////////////////////////////////////////////////////////////////
void Renderer::renderCursorTexture() const { m_cursor->draw(); }

////////////////////////////////////////////////////////////////////////////
void Renderer::renderImageTextures() {

  const int imgWidth{m_winDims.x / NUM_IMAGES_TO_DRAW};
  const int halfWinY{m_winDims.y / 2};

  int imgXPos{0};

  int idx{0};
  for (int idx{0}; idx < NUM_IMAGES_TO_DRAW; ++idx, imgXPos += imgWidth) {
    Image *img = IMG_FROM_GAL_IDX(idx);

    updateImageForGalleryView(img, imgXPos, imgWidth);

    img->draw();

    if (idx == m_currentImageHoverIndex && m_selected) {
      renderImageSelectionRectangle(img->getBounds());
    }
  }
}

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
void Renderer::updateImageForGalleryView(Image *img, int imgXPos,
                                         int imgWidth) {

  float aspect_ratio{img->getTexWidth() /
                     static_cast<float>(img->getTexHeight())};

  SDL_Rect dest;
  dest.w = imgWidth;
  // aspect_ratio = w / h --> h = w / aspect_ratio
  dest.h = static_cast<int>(imgWidth / aspect_ratio);
  dest.x = imgXPos;
  dest.y = (m_winDims.y / 2) - (dest.h / 2); // center image vertically

  img->setBounds(dest);
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
      printf("Mouse entered sdl_window %d\n", event->window.windowID);
      break;
    case SDL_WINDOWEVENT_LEAVE:
      printf("Mouse left sdl_window %d\n", event->window.windowID);
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

// void
// Renderer::findLeastIncrement(int width, int height, bool isWindow)
//{
//  int a = width, b = height, c;
//  if (a < b) {
//    c = b % a;
//    while (c != 0) {
//      b = a;
//      a = c;
//      c = b % a;
//    }
//  } else {
//    c = a % b;
//    while (c != 0) {
//      a = b;
//      b = c;
//      c = a % b;
//    }
//  }
//  if (isWindow) {
//    m_windowWidthLeastIncrement = width / b;
//    m_windowHeightLeastIncrement = height / b;
//  } else {
//    m_imageWidthLeastIncrement = width / b;
//    m_imageHeightLeastIncrement = height / b;
//  }
//}

// void
// Renderer::smoothIncrement()
//{
//  if (m_windowHeightLeastIncrement > m_imageHeightLeastIncrement) {
//    m_imageWidthLeastIncrement *=
//        m_windowHeightLeastIncrement / m_imageHeightLeastIncrement;
//    m_imageHeightLeastIncrement *=
//        m_windowHeightLeastIncrement / m_imageHeightLeastIncrement;
//  } else {
//    m_windowWidthLeastIncrement *=
//        m_imageHeightLeastIncrement / m_windowHeightLeastIncrement;
//    m_windowHeightLeastIncrement *=
//        m_imageHeightLeastIncrement / m_windowHeightLeastIncrement;
//  }
//}
