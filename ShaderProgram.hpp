#ifndef SHADERPROGRAM_HPP_INCLUDED
#define SHADERPROGRAM_HPP_INCLUDED

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <GL/glew.h>
#include <string>

class ShaderProgram final
{
public:
  ShaderProgram();
  ~ShaderProgram() throw();
	
  GLuint program();
	
  void setFrag2Tex(float x, float y);
  void setOffset(float x, float y);
	
  void use();
  void bindVertexArray();
  void unbindVertexArray();
private:
  GLuint program_;
  GLint frag2TexLoc_, offsetLoc_;
	GLuint vertBuffer_, vao_;
  GLuint compileShader(const std::string &source, GLenum type);
};

#endif
