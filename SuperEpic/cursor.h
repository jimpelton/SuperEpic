#ifndef epic_cursor_h__
#define epic_cursor_h__

#include "image.h"
#include <array>

class Cursor {
public:
  enum class Mode : int {
    Normal,
    PanningGallery,
    PanningImage,
    Selecting,
    Selected,
    Exit
  };

  template <typename T = int> T ordinal(Mode m) { return static_cast<T>(m); }

  Cursor();

  ~Cursor();

  void init(int w, int h);

  /// \brief Set cursor position centered around mouseX and mouseY.
  void setPos(int mouseX, int mouseY);

  void setSize(int w, int h);

  void setMode(Cursor::Mode);

  void draw();

  void update(float dt);

  /// \brief Set the time in seconds for the ring to complete one rotation.
  void setRingTime(float seconds);

private:
  /// \brief Set the bounds of the ring so they match up with the cursor image.
  void updateRingBounds();
  void createRingTexture();
  void clearRingTexture();
  void setImage(Image *img);

  /// \brief Start of stop the animated ring. If false, the ring is reset.
  void setAnimate(bool);

  Cursor::Mode m_mode;
  static const int IMAGES_LENGTH = 6;
  std::array<Image *, IMAGES_LENGTH> images;
  Image *m_img;           ///< Current cursor image.
  Image *m_circleSection; ///< Section of circle.
  SDL_Texture *m_target;  ///< SDL target texture.
  SDL_Rect m_ringBounds;  ///< Bounds for target texture.
  bool m_animate;         ///< True if should animate.
  bool m_alreadyAnimating;
  float m_angle; ///< Current angle of texture rotation.
  float m_da;    ///< Number of degrees per second.
};

#endif
