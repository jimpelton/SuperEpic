#include "image.h"
#include <SDL_image.h>
#include <algorithm>


///////////////////////////////////////////////////////////////////////////////
Image*
Image::load(const std::string &file)
{
  SDL_Texture* tex{ IMG_LoadTexture(sdl_renderer(), file.c_str()) };
  if (tex == nullptr)
    return nullptr;

  Image *image{ new Image() };

  SDL_QueryTexture(tex, nullptr, nullptr, &image->m_bbox.w, &image->m_bbox.h);
  image->m_bbox.x = 0;
  image->m_bbox.y = 0;
  image->m_texture = tex;

  return image;
}

///////////////////////////////////////////////////////////////////////////////
Image::Image()
  : m_texture{ nullptr }
  , m_bbox{ 0, 0, 0, 0 }
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
  SDL_RenderCopy(sdl_renderer(), m_texture, nullptr, &m_bbox);
}


///////////////////////////////////////////////////////////////////////////////
void
Image::scale(const SDL_Point &p)
{
  scale(p.x, p.y);
}


///////////////////////////////////////////////////////////////////////////////
void
Image::scale(float dw, float dh)
{
  m_bbox.w = static_cast<int>(m_bbox.w * dw);
  m_bbox.h = static_cast<int>(m_bbox.h * dh);
}


///////////////////////////////////////////////////////////////////////////////
void 
Image::maximize()
{

  int texWidth, texHeight;
  SDL_QueryTexture(m_texture, nullptr, nullptr, &texWidth, &texHeight);
  
  int ww, wh;
  SDL_GetWindowSize(sdl_window(), &ww, &wh);

  float aspect_ratio{ std::min<float>(ww / float(texWidth), 
                                      wh/float(texHeight) ) };
  
  m_bbox.w = texWidth * aspect_ratio;
  m_bbox.h = texHeight * aspect_ratio;
  m_bbox.x = (ww - m_bbox.w) / 2;
  m_bbox.y = (wh - m_bbox.h) / 2;

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
