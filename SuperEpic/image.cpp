#include "image.h"
#include <SDL_image.h>
#include <algorithm>
#include <iostream>
#include <cmath>

namespace {
}


///////////////////////////////////////////////////////////////////////////////
Image*
Image::load(const std::string &file)
{
  SDL_Surface* surf{ IMG_Load(file.c_str()) };
  SDL_Texture* tex{ SDL_CreateTextureFromSurface(sdl_renderer(), surf) };
  if (tex == nullptr)
    return nullptr;

  Image *image{ new Image() };

  SDL_QueryTexture(tex, nullptr, nullptr, &image->m_bbox.w, &image->m_bbox.h);
  image->m_bbox.x = 0;
  image->m_bbox.y = 0;
  image->m_texture = tex;

  image->m_src = image->m_bbox;
  image->m_texDims.x = image->m_bbox.w;
  image->m_texDims.y = image->m_bbox.h;

  return image;
}

///////////////////////////////////////////////////////////////////////////////
Image::Image()
  : m_texture{ nullptr }
  , m_bbox{ 0, 0, 0, 0 }
  , m_src{ 0, 0, 0, 0 }
  , m_texDims{ 0, 0 }
  , m_zoomFact{ 1 }
{ }


///////////////////////////////////////////////////////////////////////////////
Image::~Image()
{
  if (m_texture != nullptr) {
    SDL_DestroyTexture(m_texture);
  }
}

void 
Image::draw()
{
  SDL_RenderCopy(sdl_renderer(), m_texture, &m_src, &m_bbox);
}


///////////////////////////////////////////////////////////////////////////////
//void
//Image::scale(const SDL_Point &p)
//{
//  scale(p.x, p.y);
//}


///////////////////////////////////////////////////////////////////////////////
void
Image::scale(float s)
{
  int ww, wh;
  SDL_GetWindowSize(sdl_window(), &ww, &wh);

//  m_bbox.w = std::min<int>(ww,
//                           static_cast<int>(m_bbox.w * s));
//  m_bbox.h = std::min<int>(wh,
//                           static_cast<int>(m_bbox.h * s));

  float aspect_ratio{ std::min<float>(ww / float(m_texDims.x),
                                      wh / float(m_texDims.y)) };

  m_bbox.w = static_cast<int>(s * m_bbox.w * aspect_ratio);
  m_bbox.h = static_cast<int>(s * m_bbox.h * aspect_ratio);
  m_bbox.x = (ww - m_bbox.w) / 2;
  m_bbox.y = (wh - m_bbox.h) / 2;
}


///////////////////////////////////////////////////////////////////////////////
void 
Image::maximize()
{

  int texWidth, texHeight;
  SDL_QueryTexture(m_texture, nullptr, nullptr, &texWidth, &texHeight);
  
  int ww, wh;
  SDL_GetWindowSize(sdl_window(), &ww, &wh);

  float aspect_ratio{ std::min<float>(ww / float(m_texDims.x), 
                                      wh / float(m_texDims.y) ) };
  
  m_bbox.w = static_cast<int>(texWidth * aspect_ratio);
  m_bbox.h = static_cast<int>(texHeight * aspect_ratio);
  m_bbox.x = (ww - m_bbox.w) / 2;
  m_bbox.y = (wh - m_bbox.h) / 2;

  // render entire texture
  m_src.x = 0;
  m_src.y = 0;
  m_src.w = texWidth;
  m_src.h = texHeight;

}


///////////////////////////////////////////////////////////////////////////////
void
Image::zoom(float f)
{
  m_zoomFact += f;
  //TODO: adjust cropping rectangle (m_src)
  int ww, wh;
  SDL_GetWindowSize(sdl_window(), &ww, &wh);

  float aspect_ratio{ std::min<float>(ww / float(m_texDims.x),
                                      wh / float(m_texDims.y) )};

  m_src.w += static_cast<int>(f * aspect_ratio * 5.0f);
  m_src.h += static_cast<int>(f * aspect_ratio * 5.0f);

  m_src.x = (m_texDims.x - m_src.w) / 2;
  m_src.y = (m_texDims.y - m_src.h) / 2;

  if (m_src.w >= m_texDims.x){
    m_src.w = m_texDims.x;
    m_src.x = 0;
    m_bbox.w += static_cast<int>(f * aspect_ratio * 5.0f);
    m_bbox.x = (ww - m_bbox.w) / 2;
  }

  if (m_src.h >= m_texDims.y) {
    m_src.h = m_texDims.y;
    m_src.y = 0;
    m_bbox.h += static_cast<int>(f * aspect_ratio * 5.0f);
    m_bbox.y = (wh - m_bbox.h) / 2;
  }

  std::cout << "m_bbox: " << m_bbox.x << " " << m_bbox.y << " "
            << m_bbox.w << " " << m_bbox.h << std::endl;
  std::cout << "m_src: " << m_src.x << " " << m_src.y << " "
            << m_src.w << " " << m_src.h << std::endl;

}


///////////////////////////////////////////////////////////////////////////////
void
Image::panBy(const SDL_Point& delta)
{
  panBy(delta.x, delta.y);
}


///////////////////////////////////////////////////////////////////////////////
void
Image::panBy(int dx, int dy)
{
  if (m_zoomFact <= 0) {
    return;
  }
  //TODO: adjust location of src bounding rectangle
}


///////////////////////////////////////////////////////////////////////////////
void
Image::translateBy(const SDL_Point &delta)
{
  m_bbox.x += delta.x;
  m_bbox.y += delta.y;
}


///////////////////////////////////////////////////////////////////////////////
void
Image::setSize(const SDL_Point& p)
{
  setSize(p.x, p.y);
}


///////////////////////////////////////////////////////////////////////////////
void 
Image::setSize(int w, int h)
{
  m_bbox.w = w;
  m_bbox.h = h;
}


///////////////////////////////////////////////////////////////////////////////
void
Image::setPos(const SDL_Point &p)
{
  setPos(p.x, p.y);
}


///////////////////////////////////////////////////////////////////////////////
void
Image::setPos(int x, int y)
{
  m_bbox.x = x;
  m_bbox.y = y;
}


///////////////////////////////////////////////////////////////////////////////
SDL_Point
Image::getPos() const
{
  return{ m_bbox.x, m_bbox.y };
}


///////////////////////////////////////////////////////////////////////////////
SDL_Point
Image::getCenter() const
{
  return { m_bbox.x + m_bbox.w / 2, 
           m_bbox.y + m_bbox.h / 2 };
}


///////////////////////////////////////////////////////////////////////////////
const SDL_Rect&
Image::getBounds() const
{
  return m_bbox;
}


///////////////////////////////////////////////////////////////////////////////
void
Image::setBounds(const SDL_Rect& r)
{
  m_bbox = r;
}


///////////////////////////////////////////////////////////////////////////////
SDL_Texture*
Image::getTexture() const
{
  return m_texture;
}


///////////////////////////////////////////////////////////////////////////////
int 
Image::getWidth() const
{
  return m_bbox.w;
}


///////////////////////////////////////////////////////////////////////////////
int 
Image::getHeight() const
{
  return m_bbox.h;
}


///////////////////////////////////////////////////////////////////////////////
int 
Image::getTexWidth() const
{
  return m_texDims.x;
}


///////////////////////////////////////////////////////////////////////////////
int 
Image::getTexHeight() const
{
  return m_texDims.y;
}
