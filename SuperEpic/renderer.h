#ifndef epic_renderer_h__
#define epic_renderer_h__

#include "cursor.h"
#include "image.h"

#include <SDL.h>

#include <string>
#include <vector>

class Renderer {
public:
  enum class DisplayMode { Gallery, Image, FromGalleryToImage };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Create a sdl_renderer with 640x480 width and height,
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
  /// \brief Initialize SDL, open the sdl_window, create an SDL_Renderer.
  ///
  /// \return < 0 on failure, otherwise 0.
  ////////////////////////////////////////////////////////////////////////////
  int init();

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Executes the main rendering loop which does not return until the
  ///        SDL sdl_window is closed.
  ///
  /// \note  This needs to be called from the same thread that called init().
  ////////////////////////////////////////////////////////////////////////////
  void loop();

  void cursorSpeed(float s) { m_cursorSpeed = s; }
  float cursorSpeed() const { return m_cursorSpeed; }
  static bool m_shouldQuit;            ///< If the main loop should exit.

private:
  /// \brief Handle SDL events! :)
  void onEvent(const SDL_Event &event);
  void onMouseButtonUp(const SDL_MouseButtonEvent &event);
  void onKeyDown(const SDL_KeyboardEvent &event);
  void onWindowEvent(const SDL_WindowEvent &event);
  void onMouseMotionEvent(const SDL_MouseMotionEvent &event);
  void onMouseWheelEvent(const SDL_MouseWheelEvent &event);
  /// \brief Handle gestures
  void onGesture();
  void onNoGesture();
  void onSwapCandidate();
  void onSelect();
  void onPanning();
  void onZoom();
  void onSelectionProgress();
  /// \brief Update renderer state for making the transition to Image mode.
  void prepareForGalleryToImageTransition();
  /// \brief Update renderer state for displaying Image view mode (after completing transition).
  void prepareForImageViewMode();
  /// \brief Update renderer state for displaying the Gallery mode.
  void prepareForGalleryViewMode();
  /// \brief Render 5 images (thumbnails) in gallery mode.
  void renderGalleryMode();
  /// \brief Render transition from gallery mode to image mode
  void renderTransitionMode(float dt);
  /// \brief Render the image pointed to by m_imageModeImage;
  void renderImageViewMode() const;
  /// \brief Render the texture for the cursor
  void renderCursorTexture() const;
  /// \brief Renders five of the textures in m_images.
  void renderImageTextures();
  /// \brief Renders all thumbs
  void renderThumbsTexture();
  /// \brief Render a rectangle around the texture under the cursor.
  void renderRectangle(const SDL_Rect &dest, Uint8 R, Uint8 G, Uint8 B) const;
  /// \brief Update img's position and size for the gallery view mode.
  void updateImageForGalleryView(Image *img, int imgXPos, int imgWidth);
  /// \brief Update thumb's position and size for the gallery view mode.
  void updateThumbForGalleryView(Image *thumb, int thumbXPos, int thumbWidth);
  /// \brief Toggle between windowed and fullscreen modes.
  void toggleFullScreen();
  /// \brief Print info for only SDL_WindowEvents.
  void printEvent(const SDL_Event *) const;
  /// \brief Shift Candidates
  void shiftCandidates(int dx);
  /// \brief Convert screen coords to a Gallery View index
  int getGalleryIndexFromCoord(int screen_coords) const;
  /// \brief Convert Gallery View index to Image
  Image *getImageFromGalleryIndex(int index) const;

private:
  SDL_Window *m_window;
  SDL_Renderer *m_renderer;

  SDL_Point m_winDims; ///< The current sdl_window dimensions
  SDL_Point m_winPos;  ///< The current sdl_window position

  float m_cursorSpeed; ///< Scale the speed of the cursor.
  Cursor *m_cursor;
  int m_currentImageHoverIndex; ///< The image index that the cursor is hovering
                                /// over.
  int m_previousImageHoverIndex; ///< The image index that the cursor was
                                 /// hovering over.

  DisplayMode m_mode; ///< Gallery view, or image view
  int m_galleryStartIndex; ///< The index within the gallery to start at.

  std::vector<Image *> m_images; ///< Textures currently loaded into memory.
  std::vector<Image *> m_thumbs; ///< Textures for thumbs
  Image *m_imageModeImage;       ///< The image to display in image view mode.

  /// The scaling factor that the gallery to image transition should stop at.
  float m_targetScale;

  bool m_fullScreen;            ///< If in fullscreen or not.
  bool m_useKinectForCursorPos; ///< If the kinect sensor should override mouse
                                /// for cursor position.

  int m_clickCount; ///< Number of clicks clicked.
  bool m_selected;  ///< If user choose a candidate image.

  int m_imageStartingPos;
};

#endif // ! epic_renderer_h__
