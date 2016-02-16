#include "shader.h"
#include "gl_strings.h"
#include "ordinal.h"
#include "gl_log.h"

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <sstream>
#include <fstream>
#include <iostream>


namespace hci {

namespace {

const std::array<GLenum, 2> gl_target{ GL_VERTEX_SHADER,
                                       GL_FRAGMENT_SHADER };

} // namespace


//////////////////////////////////////////////////////////////////////////
//     Compiler  Implementation
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
bool Compiler::compile(Shader &shader, const char *code) {

  unsigned int shaderId = shader.id();

  gl_check(glShaderSource(shaderId, 1, &code, nullptr));

  gl_check(glCompileShader(shaderId));

  // Check for errors.
  GLint result{ GL_FALSE };
  int infoLogLength;

  gl_check(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
  gl_check(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));

  if (infoLogLength > 1) {
    std::vector<char> msg(infoLogLength + 1);
    gl_check(glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &msg[0]));
    gl_log("%s", &msg[0]);
  }

  return result == GL_TRUE;

}


//////////////////////////////////////////////////////////////////////////
//     Shader  Implementation
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
Shader::Shader(ShaderType t, const std::string &name)
    : m_type(t)
    , m_id(0) {
}

Shader::~Shader() {
  gl_log("Shader destructor not implemented :( Type: %s, GLid: %d", typeString(), m_id);
}


unsigned int Shader::create() {
  GLenum gl_type{ gl_target[ordinal<ShaderType>(m_type)] };
  GLuint shaderId = gl_check(glCreateShader(gl_type));

  gl_log("glCreateShader for type=%s returned id=%d",
         hci::gl_to_string(gl_type), shaderId);

  return m_id = shaderId;

}

///////////////////////////////////////////////////////////////////////////////
void Shader::loadFromFile(const std::string &filepath) {
  //GLuint shaderId = 0;
  std::ifstream file(filepath.c_str());
  if (!file.is_open()) {
    gl_log_err("Couldn't open %s", filepath.c_str());
    return;
  }
  std::stringstream shaderCode;
  shaderCode << file.rdbuf();

  std::string code{ shaderCode.str() };
  const char *ptrCode{ code.c_str() };
  file.close();

  Compiler::compile(*this, ptrCode); //compileShader(ptrCode);
}


///////////////////////////////////////////////////////////////////////////////
void Shader::loadFromString(const std::string &shaderString) {
  Compiler::compile(*this, shaderString.c_str());
}


///////////////////////////////////////////////////////////////////////////////
//unsigned int Shader::compileShader(const char *shader) {
//  return Compiler::compile(m_type, shader);
//}


///////////////////////////////////////////////////////////////////////////////
ShaderType Shader::type() const {
  return m_type;
}


///////////////////////////////////////////////////////////////////////////////
const char* Shader::typeString() const {
  return gl_to_string(gl_target[ordinal(m_type)]);
}

std::string Shader::to_string() const {
  return std::to_string(m_id) + ": " + typeString();
}


//////////////////////////////////////////////////////////////////////////
//   ShaderProgram Impl
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram()
  : ShaderProgram(nullptr, nullptr) { }


///////////////////////////////////////////////////////////////////////////////
ShaderProgram::ShaderProgram(Shader *vert, Shader *frag)
  : m_stages{ }
  , m_programId{ 0 }
  , m_params{ }
{
  if (vert) addStage(vert);
  if (frag) addStage(frag);
}


///////////////////////////////////////////////////////////////////////////////
ShaderProgram::~ShaderProgram() {
  //TODO: cleanup opengl shaders ... glDelete?
  gl_log_err("I might be a memory/resource leak"
    "(no cleanup in shader destructor).");
}



///////////////////////////////////////////////////////////////////////////////
//unsigned int ShaderProgram::addStage(const std::string &path, ShaderType ty) {
//  Shader *sh{ new Shader(ty) };
//  sh->create();
//  sh->loadFromFile(path);
//  return addStage(sh);
//}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::linkProgram() {
  if (!checkBuilt()) {
    // this ShaderProgram has already been built.
    return 0;
  }

  if (m_stages.empty()) {
    gl_log_err("When linking program, there were no shaders to link! Cannot "
                   "create shader program.");
    return 0;
  }

  if (m_programId != 0) {
    gl_log("Relinking shader program id=%d", m_programId);
  } else {
    // have GL make a program and bail out if no success.
    createNewProgram();

    if (m_programId == 0)
      return 0;

    gl_log("Linking shader program id=%d", m_programId);
  }

  gl_check(glLinkProgram(m_programId));

  // Check the program
  int InfoLogLength{ 0 };
  GLint result{ GL_FALSE };

  gl_check(glGetProgramiv(m_programId, GL_LINK_STATUS, &result));
  gl_check(glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &InfoLogLength));

  if (InfoLogLength > 1) {
    std::vector<char> programErrorMessage(InfoLogLength + 1);
    gl_check(glGetProgramInfoLog(m_programId,
                                 InfoLogLength,
                                 nullptr,
                                 &programErrorMessage[0]));
    gl_log("%s", &programErrorMessage[0]);
  }

  return m_programId;
}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::linkProgram(Shader *vert, Shader *frag) {

  addStage(vert);
  addStage(frag);
  return linkProgram();

}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::linkProgram(const std::string &vertPath,
                                        const std::string &fragPath) {

  Shader *vert{ new Shader(ShaderType::Vertex, vertPath.c_str()) };
  vert->create();
  vert->loadFromFile(vertPath);

  Shader *frag{ new Shader(ShaderType::Fragment, fragPath.c_str()) };
  frag->create();
  frag->loadFromFile(fragPath);

  return linkProgram(vert, frag);

}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, const glm::mat4 &val) {
  unsigned int loc = getUniformLocation(param);
//  gl_check(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val)));
  gl_check(glProgramUniformMatrix4fv(m_programId, loc, 1, GL_FALSE, glm::value_ptr(val)));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, const glm::vec4 &val) {
  unsigned int loc = getUniformLocation(param);

  gl_check(glProgramUniform4fv(m_programId, loc, 1, glm::value_ptr(val)));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, const glm::vec3 &val) {
  unsigned int loc = getUniformLocation(param);

  gl_check(glProgramUniform3fv(m_programId, loc, 1, glm::value_ptr(val)));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::setUniform(const char *param, float val) {
  unsigned int loc = getUniformLocation(param);
//  gl_check(glUniform1f(m_programId, loc, val));
  gl_check(glProgramUniform1f(m_programId, loc, val));
}


///////////////////////////////////////////////////////////////////////////////
//void ShaderProgram::setUniform(const std::string &param, const Texture &tex)
//{
//    unsigned int loc = getUniformLocation(param);
//    m_textures[loc] = &tex;
//}

void ShaderProgram::setUniform(const char *param, int val) {
  unsigned int loc = getUniformLocation(param);

  gl_check(glProgramUniform1i(m_programId, loc, val));
}

///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::getUniformLocation(const char *param) {
  int rval{ 0 };
  ParamTable::iterator found = m_params.find(param);
  if (found != m_params.end()) {
    rval = (*found).second;
  } else {
    rval = gl_check(glGetUniformLocation(m_programId, param));
    // put the uniform location in params table for faster lookup next time.
    m_params[param] = rval;
  }

  return static_cast<unsigned int>(rval);
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::bind() {
//    static const std::array<GLenum, 3> targets
//    {
//        GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D
//    };

  gl_check(glUseProgram(m_programId));

//    int i = 0;
  // pair is (glsl sampler location, Texture)
//    for (auto &pair : m_textures) {
//        gl_check(glActiveTexture(GL_TEXTURE0 + i));
//        const Texture &tex = *(pair.second);
//        GLenum target = targets.at(static_cast<int>(tex.type()));
//        gl_check(glBindTexture(target, tex.id()));
//        gl_check(glUniform1f(pair.first, tex.id()));
//
//        i += 1;
//    }

  //gl_check(glActiveTexture(GL_TEXTURE0));
}


///////////////////////////////////////////////////////////////////////////////
void ShaderProgram::unbind() {
  gl_check(glUseProgram(0));
}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::programId() const {
  return m_programId;
}


///////////////////////////////////////////////////////////////////////////////
//  ShaderProgram Private Members
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::addStage(Shader *stage) {
  m_stages.push_back(stage);
  unsigned int id{ stage->id() };
  gl_log("Added shader %s", stage->to_string().c_str());

  return id;
}


///////////////////////////////////////////////////////////////////////////////
unsigned int ShaderProgram::createNewProgram() {
  GLuint programId = gl_check(glCreateProgram());
  if (programId==0) {

    gl_log_err(
        "Unable to create shader program with glCreateProgram(). "
            "Returned id was 0.");

  } else {

    gl_log("Created program id: %d", programId);
    for (const Shader *stage : m_stages) {
      gl_check(glAttachShader(programId, stage->id()));
    }

  }

  return m_programId = programId;
}

///////////////////////////////////////////////////////////////////////////////
bool ShaderProgram::checkBuilt() {
  bool rval = true;
  auto shader = m_stages.begin();

  while (rval && shader != m_stages.end()) {
    rval = (*shader)->isBuilt();
    gl_log("Checking if shader %s is built: is built=%s",
           (*shader)->to_string().c_str(),
           rval ? "true" : "false");
    ++shader;
  }

  if (!rval && shader != m_stages.end()) {
    gl_log_err("While linking shader program %d, I found "
                   "that shader %s was not built, cannot link program.",
               m_programId,
               (*shader)->to_string().c_str());
  }

  return rval;
}

} //namespace hci

