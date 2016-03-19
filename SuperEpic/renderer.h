#ifndef epic_renderer_h__
#define epic_renderer_h__

#include <SDL.h>

#include <vector>
#include <string>

class Renderer
{
public:
  enum class DisplayMode
  {
    Gallery, Image
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Create a renderer with 640x480 width and height, 
  ///        and unspecified position.
  ////////////////////////////////////////////////////////////////////////////
  Renderer();
  Renderer(int winWidth, int winHeight, int winX, int winY);
  ~Renderer();


  ////////////////////////////////////////////////////////////////////////////
  /// \brief Load the images in \c filePaths.
  ////////////////////////////////////////////////////////////////////////////
  void loadImages(const std::vector<std::string> &filePaths);


  ////////////////////////////////////////////////////////////////////////////
  /// \brief Initialize SDL, open the window, create an SDL_Renderer.
  ///
  /// \return < 0 on failure, otherwise 0.
  ////////////////////////////////////////////////////////////////////////////
  int init();


  ////////////////////////////////////////////////////////////////////////////
  /// \brief Executes the main rendering loop which does not return until the 
  ///        SDL window is closed.
  ///
  /// \note  This needs to be called from the same thread that called init().
  ////////////////////////////////////////////////////////////////////////////
  void loop();


private:
  void onButtonUp(const SDL_MouseButtonEvent &event);
  void renderGalleryMode();
  void renderImageViewMode();

  /// \brief Render the texture for the cursor
  void renderCursorTexture();

  /// \brief Renders five of the textures in m_images.
  void renderImageTextures();
//  void renderSingleTexture(SDL_Texture *tex, int x, int y, int w, int h) const;
  
  void loadSingleTexture(const std::string &filePath);


  void printEvent(const SDL_Event*);

private:
  SDL_Window *m_window;
  SDL_Renderer *m_renderer;

  SDL_Point m_winDims;         ///< The current window dimensions
  SDL_Point m_winPos;          ///< The current window position
  SDL_Point m_cursPos;         ///< The current position of cursor/mouse/hand
  SDL_Point m_cursDims;        ///< The current cursor dimensions

  DisplayMode m_mode;            ///< Gallery view, or image view
  //RenderStrategy *m_rendStrat  ///< The method to render current DisplayMode (gallery or image view)
  size_t m_galleryStartIndex;    ///< The index within the gallery to start at.
  
  std::vector<SDL_Texture*> m_images;  ///< Textures currently loaded into memory.
  SDL_Texture *m_cursTex;              ///< The texture for the cursor.
  SDL_Texture *m_imageModeTex;         ///< The image to display in image view mode.
};


#endif // ! epic_renderer_h__
