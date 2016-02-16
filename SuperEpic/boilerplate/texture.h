#ifndef texture_h__
#define texture_h__


#include <string>
#include <cstdlib>

namespace hci {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Wraps up GL texture creation and binds it when asked.
/// \note If Texture is wrapping a 3D texture, then the texture unit
///   \c GL_TEXTURE0+static_cast<unsigned>(Sampler::Volume) is activated.
///
/// \note If a 1D texture is wrapped then the texture unit bound is
///   \c GL_TEXTURE0+static_cast<unsigned>(Sampler::Transfer) is activated.
///////////////////////////////////////////////////////////////////////////////
class Texture {
 public:

  enum class Target: unsigned int {
    Tex1D, Tex2D, Tex3D
  };


  enum class Format: unsigned int {
    RED, RG, RGB, RGBA
  };


  ///////////////////////////////////////////////////////////////////////////////
  // Constructors/Destructor
  ///////////////////////////////////////////////////////////////////////////////
  Texture(Target textureType);
  virtual ~Texture();


  ///////////////////////////////////////////////////////////////////////////////
  // Interface
  ///////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Binds this texture to the currently active texture unit.
  ///////////////////////////////////////////////////////////////////////////////
  void bind() const;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Activates texture unit \c unit and binds this texture.
  ///
  /// \note Difference between this and no-parameter version is that this calls
  ///       glActiveTexture() before the bind.
  ///////////////////////////////////////////////////////////////////////////////
  void bind(unsigned int unit) const;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create a 1D texture and hand \c img to opengl. Texture gen is
  ///        synchronous, so once this method returns img can be deallocated.
  ///
  /// \return The OpenGL integer for this texture, or 0 on failure.
  ///////////////////////////////////////////////////////////////////////////////
  unsigned int genGLTex1d(float *img, Format ity, Format ety, size_t w);

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create a 2D texture and hand \c img to opengl. Texture gen is
  ///        synchronous, so once this method returns img can be deallocated.
  ///
  /// \return The OpenGL integer for this texture, or 0 on failure.
  ///////////////////////////////////////////////////////////////////////////////
  unsigned int genGLTex2d(float *img, Format ity, Format ety,
                          size_t w, size_t h);

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create a 3D texture and hand \c img to opengl. Texture gen is
  ///        synchronous, so once this method returns img can be deallocated.
  ///
  /// \return The OpenGL integer for this texture, or 0 on failure.
  ///////////////////////////////////////////////////////////////////////////////
  unsigned int genGLTex3d(float *img, Format internal, Format external,
                          size_t w, size_t h, size_t d);


  unsigned int id() const { return m_id; }

//  unsigned int samplerLocation() const { return m_samplerUniform; }
//  void samplerLocation(unsigned int loc) { m_samplerUniform = loc; }

//  unsigned int textureUnit() const { return m_unit; }
//  void textureUnit(unsigned int unit) { m_unit = unit; }

  Target target() const { return m_type; }

  std::string to_string() const;

 private:
  ///////////////////////////////////////////////////////////////////////////////
  // Data members
  ///////////////////////////////////////////////////////////////////////////////
  unsigned int m_id;   ///< OpenGL id of the texture data.
//  unsigned int m_samplerUniform;  ///< OpenGL id of the texture sampler to use.
//  unsigned int m_unit; ///< Texture sampling unit.
  Target m_type;         ///< the gl target to bind to.
//    Sampler m_sampler; ///
};

std::ostream &operator<<(std::ostream &os, const Texture &t);

} // namespace hci



#endif // ! texture_h__
