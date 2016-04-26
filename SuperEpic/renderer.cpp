#include "renderer.h"
#include <ctime>

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

const int NUM_IMAGES_TO_DRAW{6};

// const char *DEFAULT_CURSOR_TEXTURE_PATH{"../res/open_hand.png"};
// const char *DEFAULT_CURSOR_RING_TEXTURE_PATH{
// "../res/circle_section_white.png" };
const float DEFAULT_CURSOR_RING_CYCLE_SECONDS{1.0f};
const int DEFAULT_WILLING_TO_QUIT{60};
} // namespace

bool Renderer::m_shouldQuit = false;

////////////////////////////////////////////////////////////////////////////
Renderer::Renderer()
    : Renderer(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, DEFAULT_WIN_X,
               DEFAULT_WIN_Y) {}

////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(int winWidth, int winHeight, int winX, int winY)
    : m_window{nullptr}, m_renderer{nullptr}, m_winDims{winWidth, winHeight},
      m_winPos{winX, winY}, m_cursorSpeed{DEFAULT_CURSOR_SPEED},
      m_cursor{nullptr}, m_mode{DisplayMode::Gallery}, m_galleryStartIndex{0},
      m_images{}, m_imageModeImage{nullptr}, m_fullScreen{false},
      m_useKinectForCursorPos{false}, m_imageStartingPos{0}, m_clickCount{0},
      m_selected{false}, m_willingToQuit{0} //  , m_srcImageRect{ 0, 0, 0, 0 }
//  , m_destWindowRect{ 0, 0, 0, 0 }
//  , m_imageScreenRatio{ 0 }
//  , m_windowHeightLeastIncrement{ 0 }
//  , m_windowWidthLeastIncrement{ 0 }
//  , m_imageHeightLeastIncrement{ 0 }
//  , m_imageWidthLeastIncrement{ 0 }
{}

////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer() {
  for (auto img : m_images) {
    delete img;
  }

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
  for (auto file : images) {
    Image *texture{Image::load(file)};
    if (!texture) {
      std::cerr << "Could not load image texture: " << SDL_GetError()
                << std::endl;
      return;
    }
    std::cout << "Loaded image: " << file << "\n";
    m_images.push_back(texture);
    Image *thumb = new Image(*texture);
    m_thumbs.push_back(thumb);
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
                                                    SDL_RENDERER_PRESENTVSYNC |
                                                    SDL_RENDERER_TARGETTEXTURE);

  if (m_renderer == nullptr) {
    std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << "\n";
    return -1;
  }

  Image::sdl_renderer(m_renderer);

  m_cursor = new Cursor();
  m_cursor->init(static_cast<int>(m_winDims.x * DEFAULT_CURSOR_SCALE),
                 static_cast<int>(m_winDims.x * DEFAULT_CURSOR_SCALE));
  //  Image *cursImg = Image::load(DEFAULT_CURSOR_TEXTURE_PATH);
  //  if (cursImg == nullptr) {
  //    std::cerr << "Could not load image texture: " << SDL_GetError()
  //              << std::endl;
  //    return -1;
  //  }

  m_cursor->setRingTime(DEFAULT_CURSOR_RING_CYCLE_SECONDS);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); // anti-aliasing
  SDL_ShowCursor(0);                                 // don't show mouse arrow.
  toggleFullScreen();
  return 0;
}

////////////////////////////////////////////////////////////////////////////
void Renderer::loop() {
  float since = 0.0f; // seconds since last loop iteration
  float then = 0.0f;
  while (!m_shouldQuit) {
    float now = SDL_GetTicks() * 1e-3f;
    since = now - then; // seconds

// Pop events from the SDL event queue.
// When we start using the kinect to control, this may get replaced, or
// this is probably were the kinect gesture events will get checked.

#ifdef WIN32
    if (m_useKinectForCursorPos) {
      onGesture();
      SDL_Event event;
      while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_k) {
          m_useKinectForCursorPos = !m_useKinectForCursorPos;
        }
      }
    } else {
      SDL_Event event;
      while (SDL_PollEvent(&event) != 0) {
        printEvent(&event);
        onEvent(event);
      }
    }
#else
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      printEvent(&event);
      onEvent(event);
    }
#endif

    std::cout << KinectSensor::getGestureType() << std::endl;

    SDL_RenderClear(m_renderer);

    switch (m_mode) {
    case DisplayMode::Gallery:
      renderGalleryMode();
      break;
    case DisplayMode::FromGalleryToImage:
      renderTransitionMode(since);
      break;
    case DisplayMode::Image:
      renderImageViewMode();
      break;
    }

    m_cursor->update(since);
    renderCursorTexture();

    SDL_RenderPresent(m_renderer);
    then = now;
  } // while(!m_shouldQuit)

  std::cout << "Exiting render loop\n";
}

////////////////////////////////////////////////////////////////////////////
int Renderer::getGalleryIndexFromCoord(int screen_coords) const {
  return (screen_coords - m_imageStartingPos) / (m_winDims.x / 5);
}

Image *Renderer::getImageFromGalleryIndex(int index) const {
  return m_images[(m_galleryStartIndex + (index)) % m_images.size()];
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
      } else if (m_clickCount ==
                 2) { // switch to GalleryToImage transition mode.
        prepareForGalleryToImageTransition();
      }
    }
  } else if (button.button == SDL_BUTTON_RIGHT) {
    if (m_mode == DisplayMode::Image) {
      prepareForGalleryViewMode();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onKeyDown(const SDL_KeyboardEvent &key) {

  switch (key.keysym.sym) {
  case SDLK_ESCAPE:
    if (m_mode == DisplayMode::Image) {
      prepareForGalleryViewMode(); // go back to Gallery View.
    } else if (m_mode == DisplayMode::Gallery) {
      m_shouldQuit = true; // If in gallery, then quit.
    }
    break;
  case SDLK_LEFT:
    if (m_mode == DisplayMode::Gallery) {
      shiftCandidates(-10);
    } else if (m_mode == DisplayMode::Image) {
      SDL_Point delta{10, 0};
      m_imageModeImage->panBy(delta);
    }
    break;
  case SDLK_RIGHT: // scroll images left
    if (m_mode == DisplayMode::Gallery) {
      shiftCandidates(10);
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
      if (currentScaleFactor <= m_imageModeImage->getBaseScaleFactor() / 5)
        prepareForGalleryViewMode();
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
  m_currentImageHoverIndex = getGalleryIndexFromCoord(motion.x);
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
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onGesture() {
  std::string gesture = KinectSensor::getGestureType();
  if (gesture == "NO_GESTURE") {
    m_cursor->setMode(Cursor::Mode::Normal);
    onNoGesture();
  } else if (gesture == "SELECT") {
    m_cursor->setMode(Cursor::Mode::Selected);
    onSelect();
  } else if (gesture == "PANNING") {
    m_cursor->setMode(Cursor::Mode::PanningGallery);
    onPanning();
  } else if (gesture == "ZOOM_IN") {
    m_cursor->setMode(Cursor::Mode::Selected);
    onZoom(1);
  } else if (gesture == "ZOOM_OUT") {
    m_cursor->setMode(Cursor::Mode::Selected);
    onZoom(-1);
  } else if (gesture == "SELECTION_PROGRESS") {
    m_cursor->setMode(Cursor::Mode::Selecting);
    onSelectionProgress();
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onNoGesture() {
  SDL_Point pos;
  KinectSensor::mapHandToCursor(KinectSensor::handCoords, m_winDims.x,
                                m_winDims.y, reinterpret_cast<int *>(&pos));

  m_cursor->setPos(pos.x, pos.y);
  m_previousImageHoverIndex = m_currentImageHoverIndex;
  m_currentImageHoverIndex = getGalleryIndexFromCoord(pos.x);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::onSelect() {
  m_currentImageSelectIndex = m_currentImageHoverIndex;
  m_selected = true;
}

void Renderer::onPanning() {
  if (m_mode == DisplayMode::Gallery) {
    shiftCandidates(KinectSensor::panning_delta_x * 30);
  } else if (m_mode == DisplayMode::Image) {
    SDL_Point delta{KinectSensor::panning_delta_x * 30,
                    KinectSensor::panning_delta_y * 30};
    m_imageModeImage->panBy(delta);
  }
}

void Renderer::onZoom(int factor) {
  if (m_mode == DisplayMode::Gallery) {
    if (factor < 0) {
      m_willingToQuit -= factor;
      if (m_willingToQuit >= DEFAULT_WILLING_TO_QUIT) {
        m_shouldQuit = true;
      }
    } else if (m_selected) {
      m_willingToQuit = 0;
      prepareForGalleryToImageTransition();
    } else {
      m_willingToQuit = 0;
    }

  } else if (m_mode == DisplayMode::Image) {
    float currentScaleFactor =
        m_imageModeImage->getScaleFactor() + 0.01f * factor;
    m_imageModeImage->scale(currentScaleFactor);
    if (currentScaleFactor <= m_imageModeImage->getBaseScaleFactor() / 5) {
      prepareForGalleryViewMode();
    }
  }
}

void Renderer::onSelectionProgress() {
  if (std::time(nullptr) - KinectSensor::timer > 0.5) {
    m_selected = false;
  }
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderGalleryMode() {
  renderImageTextures();
  renderThumbsTexture();
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderTransitionMode(float secondsSinceLastUpdate) {
  float zoom_speed = 0.3f;

  float scale = m_imageModeImage->getScaleFactor() +
                (zoom_speed * secondsSinceLastUpdate);

  // Keep scaling until the target scale has been reached.
  if (scale > m_targetScale) {
    // done animating so go to Image view mode.
    prepareForImageViewMode();
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
  const int imgWidth{m_winDims.x / 5};

  int imgXPos = m_imageStartingPos;

  for (int i = 0; i < NUM_IMAGES_TO_DRAW; ++i, imgXPos += imgWidth) {
    Image *img = getImageFromGalleryIndex(i);

    updateImageForGalleryView(img, imgXPos, imgWidth);

    img->draw();

    if (m_selected &&
        (!m_useKinectForCursorPos && i == m_currentImageHoverIndex ||
         m_useKinectForCursorPos && i == m_currentImageSelectIndex)) {
      renderRectangle(img->getBounds(), 3, 255, 0, 0);
    }
  }
}

void Renderer::renderThumbsTexture() {
  const int thumbWidth{m_winDims.x / (5 * static_cast<int>(m_thumbs.size()))};
  const int imgWidth{m_winDims.x / 5};
  int thumbXpos = 2 * m_winDims.x / 5;
  SDL_Rect thumbBox;
  thumbBox.h = 0;
  thumbBox.x = thumbXpos;
  thumbBox.w = 5 * thumbWidth;
  for (auto thumb : m_thumbs) {
    updateThumbForGalleryView(thumb, thumbXpos, thumbWidth);
    thumbXpos += thumbWidth;
    thumb->draw();
    if (thumbBox.h < thumb->getBounds().h)
      thumbBox.h = thumb->getBounds().h;
  }
  thumbBox.h *= 1.2;
  thumbBox.y = (m_winDims.y * 4 / 5) - (thumbBox.h / 2);
  thumbBox.x += thumbWidth * m_galleryStartIndex -
                (m_imageStartingPos * thumbWidth / imgWidth);
  renderRectangle(thumbBox, 2, 0, 255, 255);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::renderRectangle(const SDL_Rect &dest, int thickness, Uint8 R,
                               Uint8 G, Uint8 B) const {
  // save the clear color so it can be restored after DrawRect()
  Uint8 r, g, b, a;
  SDL_Rect targetRect(dest);
  SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(m_renderer, R, G, B, 0);
  for (int i = 0; i < thickness; i++) {
    SDL_RenderDrawRect(m_renderer, &targetRect);
    targetRect.x++;
    targetRect.y++;
    targetRect.w -= 2;
    targetRect.h -= 2;
  }
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

void Renderer::updateThumbForGalleryView(Image *thumb, int thumbXPos,
                                         int thumbWidth) {
  float aspect_ratio{thumb->getTexWidth() /
                     static_cast<float>(thumb->getTexHeight())};

  SDL_Rect dest;
  dest.w = thumbWidth;
  // aspect_ratio = w / h --> h = w / aspect_ratio
  dest.h = static_cast<int>(thumbWidth / aspect_ratio);
  dest.x = thumbXPos;
  dest.y = (m_winDims.y * 4 / 5) - (dest.h / 2);

  thumb->setBounds(dest);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::prepareForGalleryToImageTransition() {
  m_mode = DisplayMode::FromGalleryToImage;
  m_clickCount = 0;
  m_selected = false;
  m_willingToQuit = 0;
  int idx = m_useKinectForCursorPos ? m_currentImageSelectIndex
                                    : m_currentImageHoverIndex;
  m_imageModeImage = getImageFromGalleryIndex(idx);
  m_imageModeImage->maximize();
  m_targetScale = m_imageModeImage->getScaleFactor();
  m_imageModeImage->scale(0.0f);
}

////////////////////////////////////////////////////////////////////////////
void Renderer::prepareForImageViewMode() {
  m_mode = KinectSensor::mode = DisplayMode::Image;
  m_imageModeImage->scale(m_targetScale);
  m_imageModeImage->setBaseScaleFactor(m_targetScale);
}

void Renderer::prepareForGalleryViewMode() {
  m_mode = KinectSensor::mode = DisplayMode::Gallery;
  std::cout << "Switch to Gallery View"
            << "\n";
  m_willingToQuit = 0;
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

////////////////////////////////////////////////////////////////////////////
void Renderer::shiftCandidates(int dx) {
  const int imgWidth{m_winDims.x / 5};
  if (dx < 0) { // shift to left to bring up new candidate from right
    if (m_galleryStartIndex + 5 < m_images.size()) {
      if (m_imageStartingPos + dx + imgWidth < 0) {
        m_galleryStartIndex++;
        m_imageStartingPos = 0;
      } else {
        m_imageStartingPos += dx;
      }
    }
  } else { // shift to right to bring up new candidate from left
    if (!(m_galleryStartIndex == 0 && m_imageStartingPos == 0)) {
      if (m_imageStartingPos == 0) {
        m_galleryStartIndex--;
        m_imageStartingPos = dx - imgWidth;
      } else {
        m_imageStartingPos += dx;
        if (m_imageStartingPos > 0)
          m_imageStartingPos = 0;
      }
    }
  }
}