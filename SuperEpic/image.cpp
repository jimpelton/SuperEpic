#include "image.h"
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {}

///////////////////////////////////////////////////////////////////////////////
Image *Image::load(const std::string &file) {
  SDL_Surface *surf{IMG_Load(file.c_str())};
  SDL_Texture *tex{SDL_CreateTextureFromSurface(sdl_renderer(), surf)};
  if (tex == nullptr)
    return nullptr;

  Image *image{new Image()};

  SDL_QueryTexture(tex, nullptr, nullptr, &image->m_bbox.w, &image->m_bbox.h);
  image->m_bbox.x = 0;
  image->m_bbox.y = 0;
  image->m_texture = tex;

  image->m_src = image->m_bbox;
  image->m_texDims.x = image->m_bbox.w;
  image->m_texDims.y = image->m_bbox.h;

  // this is just to init the m_scaleFactor variable to something that is not
  // zero.
  image->maximize();

  return image;
}

///////////////////////////////////////////////////////////////////////////////
Image::Image()
    : m_texture{nullptr}, m_bbox{0, 0, 0, 0}, m_src{0, 0, 0, 0},
      m_texDims{0, 0}, m_scaleFactor{0} {}

///////////////////////////////////////////////////////////////////////////////
Image::~Image() {
  if (m_texture != nullptr) {
    SDL_DestroyTexture(m_texture);
  }
}

void Image::draw() {
  SDL_RenderCopy(sdl_renderer(), m_texture, &m_src, &m_bbox);
}

///////////////////////////////////////////////////////////////////////////////
void Image::scale(float s) {
  if (s < 0.0f) {
    s = 0.0f;
  }
  int ww, wh, x_max = m_texDims.x * s, y_max = m_texDims.y * s;
  SDL_GetWindowSize(sdl_window(), &ww, &wh);
  SDL_Point c = getCenter();

  m_bbox.w = static_cast<int>(s * m_texDims.x);
  m_bbox.h = static_cast<int>(s * m_texDims.y);
  m_bbox.x = c.x - m_bbox.w / 2;
  m_bbox.y = c.y - m_bbox.h / 2;

  if (m_scaleFactor > s) {
    if (s <= m_baseScaleFactor || ww >= x_max || wh >= y_max) { // center image
      setPos((ww - m_bbox.w) / 2, (wh - m_bbox.h) / 2);
    } else { // align boarder
      int dx = 0, dy = 0;
      if (m_bbox.x > 0) {
        dx = 0 - m_bbox.x;
      } else if (m_bbox.x + x_max < ww) {
        dx = ww - m_bbox.x - x_max;
      }
      if (m_bbox.y > 0) {
        dy = 0 - m_bbox.y;
      } else if (m_bbox.y + y_max < wh) {
        dy = wh - m_bbox.y - y_max;
      }
      translateBy(dx, dy);
    }
  }

  m_scaleFactor = s;
}

///////////////////////////////////////////////////////////////////////////////
void Image::maximize() {
  int ww, wh;
  SDL_GetWindowSize(sdl_window(), &ww, &wh);

  // min(max_size_possible / source_image_size)
  float scale_factor{
      std::min<float>(ww / float(m_texDims.x), wh / float(m_texDims.y))};

  scale(scale_factor);
  setPos((ww - m_bbox.w) / 2, (wh - m_bbox.h) / 2);

  // render entire texture
  m_src.x = 0;
  m_src.y = 0;
  m_src.w = m_texDims.x;
  m_src.h = m_texDims.y;

  m_scaleFactor = scale_factor;
}

///////////////////////////////////////////////////////////////////////////////
void Image::panBy(const SDL_Point &delta) { panBy(delta.x, delta.y); }

///////////////////////////////////////////////////////////////////////////////
void Image::panBy(int dx, int dy) {
  if (m_scaleFactor > m_baseScaleFactor) {
    int ww, wh, x_max = m_texDims.x * m_scaleFactor,
                y_max = m_texDims.y * m_scaleFactor;
    SDL_GetWindowSize(sdl_window(), &ww, &wh);
    if (m_bbox.x + dx > 0) {
      dx = 0;
    } else if (m_bbox.x + x_max + dx < ww) {
      dx = 0;
    }
    if (m_bbox.y + dy > 0) {
      dy = 0;
    } else if (m_bbox.y + y_max + dy < wh) {
      dy = 0;
    }
    translateBy(dx, dy);
  }
}

///////////////////////////////////////////////////////////////////////////////
void Image::translateBy(const SDL_Point &delta) {
  translateBy(delta.x, delta.y);
}

void Image::translateBy(int dx, int dy) {
  m_bbox.x += dx;
  m_bbox.y += dy;
}

///////////////////////////////////////////////////////////////////////////////
void Image::setSize(const SDL_Point &p) { setSize(p.x, p.y); }

///////////////////////////////////////////////////////////////////////////////
void Image::setSize(int w, int h) {
  m_bbox.w = w;
  m_bbox.h = h;
}

///////////////////////////////////////////////////////////////////////////////
void Image::setPos(const SDL_Point &p) { setPos(p.x, p.y); }

///////////////////////////////////////////////////////////////////////////////
void Image::setPos(int x, int y) {
  m_bbox.x = x;
  m_bbox.y = y;
}

///////////////////////////////////////////////////////////////////////////////
SDL_Point Image::getPos() const { return {m_bbox.x, m_bbox.y}; }

///////////////////////////////////////////////////////////////////////////////
SDL_Point Image::getCenter() const {
  return {m_bbox.x + m_bbox.w / 2, m_bbox.y + m_bbox.h / 2};
}

///////////////////////////////////////////////////////////////////////////////
const SDL_Rect &Image::getBounds() const { return m_bbox; }

///////////////////////////////////////////////////////////////////////////////
void Image::setBounds(const SDL_Rect &r) { m_bbox = r; }

///////////////////////////////////////////////////////////////////////////////
SDL_Texture *Image::getTexture() const { return m_texture; }

///////////////////////////////////////////////////////////////////////////////
int Image::getWidth() const { return m_bbox.w; }

///////////////////////////////////////////////////////////////////////////////
int Image::getHeight() const { return m_bbox.h; }

///////////////////////////////////////////////////////////////////////////////
int Image::getTexWidth() const { return m_texDims.x; }

///////////////////////////////////////////////////////////////////////////////
int Image::getTexHeight() const { return m_texDims.y; }

///////////////////////////////////////////////////////////////////////////////
float Image::getScaleFactor() const { return m_scaleFactor; }

void Image::setBaseScaleFactor(float baseScaleFactor) {
  m_baseScaleFactor = baseScaleFactor;
}

float Image::getBaseScaleFactor() const { return m_baseScaleFactor; }
