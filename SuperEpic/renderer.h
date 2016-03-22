#ifndef epic_renderer_h__
#define epic_renderer_h__

#include <SDL.h>

#include <string>
#include <vector>

class Renderer {
public:
  enum class DisplayMode { Gallery, Image };

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

  void cursorSpeed(float s) { m_cursorSpeed = s; }
  float cursorSpeed() const { return m_cursorSpeed; }

private:
  /// \brief Handle SDL events! :)
  void onEvent(const SDL_Event &event);
  void onMouseButtonUp(const SDL_MouseButtonEvent &event);
  void onKeyDown(const SDL_KeyboardEvent &event);
  void onWindowEvent(const SDL_WindowEvent &event);
  void onMouseMotionEvent(const SDL_MouseMotionEvent &event);
  void onMouseWheelEvent(const SDL_MouseWheelEvent &event);
  /// \brief Render 5 images in gallery mode.
  void renderGalleryMode() const;
  /// \brief Render the image pointed to by m_imageModeTex;
  void renderImageViewMode() const;
  /// \brief Render the texture for the cursor
  void renderCursorTexture() const;
  /// \brief Renders five of the textures in m_images.
  void renderImageTextures() const;
  /// \brief Render a rectangle around the texture under the cursor.
  void renderImageSelectionRectangle(const SDL_Rect &) const;
  //  void renderSingleTexture(SDL_Texture *tex, int x, int y, int w, int h)
  //  const;
  /// \brief Toggle between windowed and fullscreen modes.
  void toggleFullScreen();
  /// \brief Load image into gpu memory and push onto m_images if successful.
  void loadSingleTexture(const std::string &filePath);
  /// \brief Print info for only SDL_WindowEvents.
  void printEvent(const SDL_Event *) const;

private:
  SDL_Window *m_window;
  SDL_Renderer *m_renderer;

  SDL_Point m_winDims; ///< The current window dimensions
  SDL_Point m_winPos;  ///< The current window position

  SDL_Point m_cursPos;         ///< The current position of cursor/mouse/hand
  SDL_Point m_cursDims;        ///< The current cursor dimensions
  float m_cursorSpeed;         ///< Scale the speed of the cursor.
  int m_cursorImageHoverIndex; ///< The image index that the cursor is hovering
                               /// over.

  DisplayMode m_mode; ///< Gallery view, or image view
  // RenderStrategy *m_rendStrat  ///< The method to render current DisplayMode
  // (gallery or image view)
  int m_galleryStartIndex; ///< The index within the gallery to start at.

  std::vector<SDL_Texture *>
      m_images;                ///< Textures currently loaded into memory.
  SDL_Texture *m_cursTex;      ///< The texture for the cursor.
  SDL_Texture *m_imageModeTex; ///< The image to display in image view mode.
  ///< Note: m_imageModeTex != m_images[m_cursorImageHoverIndex], fyi!

  bool m_fullScreen;            ///< If in fullscreen or not.
  bool m_shouldQuit;            ///< If the main loop should exit.
  bool m_useKinectForCursorPos; ///< If the kinect sensor should override mouse
                                ///for cursor position.

  int m_previousImageHoverIndex; ///< The image index that the cursor was
                                 /// hovering over.
  int m_clickCount;              ///< Number of clicks clicked.
  bool m_selected;               ///< If user choose a candidate image.

  SDL_Rect m_srcImageRect;   ///< Source image rectangle.
  SDL_Rect m_destWindowRect; ///< Destination window rectangle.
  int m_imageScreenRatio;    ///< Image screen ratio,
                             ///< 0 when image = screen
                             ///< >0 when image > screen (zoomed in)
                             ///< <0 when image < screen (zoomed out)

  int m_windowHeightLeastIncrement; ///< Window height min increment
  int m_windowWidthLeastIncrement;  ///< Window width min increment
  int m_imageHeightLeastIncrement;  ///< Image height min increment
  int m_imageWidthLeastIncrement;   ///< Image width min increment

  void findLeastIncrement(int width, int height,
                          bool isWindow); ///< Find least increment
  void smoothIncrement();
};

#endif // ! epic_renderer_h__
