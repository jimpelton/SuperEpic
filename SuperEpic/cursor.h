#ifndef epic_cursor_h__
#define epic_cursor_h__

#include "image.h"

class Cursor
{
public:
  Cursor() 
    : m_img{ nullptr } 
  { }
  
  ~Cursor()
  {
    if (m_img)
      delete m_img;
  }

  /// \brief Sets the image of the cursor so that it is 
  ///        centered over the mouse coordinates.
  void setPos(int mouseX, int mouseY) const
  {
    m_img->setPos(mouseX - m_img->getWidth()/2,
                  mouseY - m_img->getHeight()/2);
  }
  

  void setSize(int w, int h) const
  {
    m_img->setSize(w, h);
  }

  
  void setImage(Image *img)
  {
    m_img = img;
  }
  

  const Image* getCursorImage() const
  {
    return m_img;
  }

  void draw()
  {
    m_img->draw();
  }

private:
  Image *m_img;
};


#endif
