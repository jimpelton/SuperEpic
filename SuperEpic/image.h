#ifndef epic_image_h__
#define epic_image_h__

#include <SDL.h>
#include <string>


class Image
{

public:
  // These two functions are such a bad idea (for at least two reasons)
  static SDL_Renderer* 
  sdl_renderer(SDL_Renderer *ren = nullptr)
  {
    static SDL_Renderer * renderer = ren; 
    return renderer;
  }
  
  static SDL_Window* 
  sdl_window(SDL_Window *win = nullptr)
  {
    static SDL_Window* window = win;  
    return window;
  }

  /// \brief Create an image from the image at imgFilePath.
  /// \return nullptr if failure, otherwise a valid Image.
  static Image* load(const std::string &imgFilePath);

  virtual ~Image();
  
  void draw();

  /// \brief Translate the image to given destination.
  /// \param p Where the upper-left corner of this Image should be.
  //void scale(const SDL_Point &p);
  void scale(float s);
  

  /// \brief Make this image as large as possible for the sdl_window size.
  void maximize();

  /// \brief Zoom by a factor of provided value.
  void zoom(float);

  /// \brief Pan the source rectangle around.
  void panBy(const SDL_Point &delta);
  void panBy(int dx, int dy);


  /// \brief Translate the image by the provided delta.
  /// \param delta How much to move the upper-left corner of this Image.
  void translateBy(const SDL_Point& delta);

  /// \brief Set the size of this image.
  void setSize(const SDL_Point &p);
  void setSize(int w, int h);
  

  /// \brief Set the position of upper left corner.
  void setPos(const SDL_Point &p);
  void setPos(int x, int y);
  

  /// \brief Get the upper left corner
  SDL_Point getPos() const;


  /// \brief Compute and return center coordinate.
  SDL_Point getCenter() const;

  /// \brief Get the bounding rectangle for this image.
  const SDL_Rect& getBounds() const;
  void setBounds(const SDL_Rect &r);


  /// \brief Get the texture from SDL that is this image.
  SDL_Texture* getTexture() const;

  /// \brief
  int getWidth() const;
  
  /// \brief
  int getHeight() const;

  /// \brief
  int getTexWidth() const;

  /// \brief
  int getTexHeight() const;


private:
  Image();

  SDL_Texture *m_texture;
  SDL_Rect m_bbox; ///< The bounding box for this image
  SDL_Rect m_src;  ///< The cropping rectangle for this image.
  SDL_Point m_texDims;

  float m_zoomFact;


};

#endif  // ! epic_image_h__