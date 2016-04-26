//
// Created by Jim Pelton on 4/21/16.
//

#include "cursor.h"
#include <iostream>
#include <iterator>

// unsigned long long printSDLError(unsigned long long ec, const char* func,
// unsigned long long line)
//{
//    if (ec != 0) {
//        std::cerr << func << ":" << line << ":: " << SDL_GetError() <<
//        std::endl;
//    }
//    return ec;
//}

namespace {
const std::string resourcePath{"../res/"};
}

Cursor::Cursor()
    : m_mode{Mode::Normal}, m_img{nullptr}, m_circleSection{nullptr},
      m_target{nullptr}, m_animate{false}, m_alreadyAnimating{false},
      m_angle{0.0f}, m_da{1.0f} {}

Cursor::~Cursor() {
  //  for (int i = 0; i < images.size(); ++i)
  //    if (images[i] != nullptr) {
  //      delete images[i];
  //    }
}

void Cursor::init(int w, int h) {
  m_circleSection = Image::load(resourcePath + "circle_section_white.png");

  Image *handClosed = Image::load(resourcePath + "HandCursorClosed.png");
  Image *handOpened = Image::load(resourcePath + "HandCursorOpened.png");
  Image *exit = Image::load(resourcePath + "exit.png");

  images[ordinal(Mode::Normal)] = handOpened;
  images[ordinal(Mode::PanningGallery)] = handClosed;
  images[ordinal(Mode::PanningImage)] = handClosed;
  images[ordinal(Mode::Selecting)] = handClosed;
  images[ordinal(Mode::Selected)] = handClosed;
  images[ordinal(Mode::Exit)] = exit;

  for (int i = 0; i < images.size(); ++i) {
    images[i]->setSize(w, h);
  }

  setImage(images[ordinal(Mode::Normal)]);
  m_mode = Mode::Normal;

  createRingTexture();
}

void Cursor::setPos(int mouseX, int mouseY) {
  SDL_Point img_pos = {mouseX - m_img->getWidth() / 2,
                       mouseY - m_img->getHeight() / 2};

  m_img->setPos(img_pos);
  updateRingBounds();
  m_circleSection->setBounds(m_ringBounds);
}

void Cursor::setSize(int w, int h) {
  m_img->setSize(w, h);
  updateRingBounds();
  m_circleSection->setBounds(m_ringBounds);
}

void Cursor::setMode(Cursor::Mode m) {
  // m_mode = m;
  if (m_mode == m) {
    return;
  }
  m_mode = m;

  switch (m) {
  case Mode::PanningGallery:
    setImage(images[ordinal(Mode::PanningImage)]);
    setAnimate(false);
    break;

  case Mode::PanningImage:
    setImage(images[ordinal(Mode::PanningImage)]);
    break;

  case Mode::Selecting:
    setImage(images[ordinal(Mode::Selecting)]);
    setAnimate(true);
    break;

  case Mode::Selected:
    setImage(images[ordinal(Mode::Selected)]);
    setAnimate(false);
    break;

  case Mode::Normal:
  default:
    setImage(images[ordinal(Mode::Normal)]);
    setAnimate(false);
    break;
  }
}

void Cursor::setImage(Image *img) {
  // new image gets the position of the old image if we have one
  if (m_img) {
    img->setPos(m_img->getPos());
  }

  m_img = img;
  updateRingBounds();
  m_circleSection->setBounds(m_ringBounds);

  // clearRingTexture();
}

void Cursor::draw() {
  if (m_animate) {
    SDL_Renderer *r{Image::sdl_renderer()};
    SDL_SetRenderTarget(r, m_target);
    SDL_SetTextureBlendMode(m_target, SDL_BLENDMODE_BLEND);

    // render ring rotated around m_targetBounds center.
    SDL_RenderCopyEx(r, m_circleSection->getTexture(), nullptr, nullptr,
                     m_angle, nullptr, SDL_FLIP_NONE);

    // reset render target
    SDL_SetRenderTarget(r, nullptr);

    // render ring texture to screen at the position of the cursor image.
    SDL_RenderCopy(r, m_target, nullptr, &m_ringBounds);
  }

  // draw cursor image to screen
  m_img->draw();
}

void Cursor::update(float dt) {

  if (m_animate) {
    m_angle += (m_da * dt);
    if (m_angle > 365.0f) {
      setAnimate(false);
    }
  }
}

void Cursor::setAnimate(bool a) {
  if (a == false) {
    m_angle = 0;
    clearRingTexture();
  }

  m_animate = a;
}

void Cursor::setRingTime(float sec) { m_da = 360.0f / sec; }

void Cursor::updateRingBounds() {
  m_ringBounds = {m_img->getPos().x - 50, m_img->getPos().y - 50,
                  m_img->getWidth() + 100, m_img->getHeight() + 100};
}

void Cursor::createRingTexture() {
  if (m_target) {
    SDL_DestroyTexture(m_target);
  }

  m_target = SDL_CreateTexture(Image::sdl_renderer(), SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_TARGET, m_ringBounds.w,
                               m_ringBounds.h);

  if (m_target == nullptr) {
    std::cout << __func__ << ":" << __LINE__ << "::" << SDL_GetError()
              << std::endl;
    return;
  }
}

void Cursor::clearRingTexture() {
  SDL_Renderer *r{Image::sdl_renderer()};
  // Clear out the render target for the cursor.
  SDL_SetRenderTarget(r, m_target);
  SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0x0);
  SDL_RenderClear(r);
  SDL_SetRenderTarget(r, nullptr);
}
