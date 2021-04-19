#include "ShaderProgram.hpp"
#include <sstream>

const std::string vertShaderSource =
"#version 410\n"
"in vec2 vPos;\n"
"void main() {\n"
"	gl_Position = vec4(vPos, 0, 1);\n"
"}\n";
const std::string fragShaderSource =
"#version 410\n"
"uniform vec2 frag2Tex;\n"
"uniform vec2 offset;\n"
"uniform sampler2D texSampler;\n"
"uniform vec4 bgColor;\n"
"out vec4 color;\n"
"void main() {\n"
" vec2 tex = frag2Tex * (gl_FragCoord.xy + offset);\n"
" if(tex.x < 0.f || tex.y < 0.f ||\n"
"		tex.x > 1.f || tex.y > 1.f) {\n"
"		color = bgColor;\n"
"	} else {\n"
" 	color = texture(texSampler, tex);\n"
" }\n"
"}\n";

const float verts[] = {
	-1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f
};

ShaderProgram::ShaderProgram()
	:frag2TexLoc_(-1),
	offsetLoc_(-1),
	bgColorLoc_(-1)
{
	GLuint fragShader = 0, vertShader = 0;
	fragShader = compileShader(fragShaderSource, GL_FRAGMENT_SHADER);
	vertShader = compileShader(vertShaderSource, GL_VERTEX_SHADER);
	program_ = glCreateProgram();
	glAttachShader(program_, vertShader);
	glAttachShader(program_, fragShader);
	glLinkProgram(program_);
	GLint status;
	glGetProgramiv(program_, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) //Linking failure
	{
		GLint logLength = 0;
		glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* errorLog = new GLchar[logLength];
		glGetProgramInfoLog(program_, logLength, 0, errorLog);
		std::string errorString(errorLog);
		delete[] errorLog;
		throw std::runtime_error("Error encountered when linking shaders: " +
			errorString);
	}

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
	
	GLint loc = glGetUniformLocation(program_, "texSampler");
	if(loc == -1) {
		throw std::runtime_error("Unable to get uniform for texSampler");
	}
	glProgramUniform1i(program_, loc, 0);
	
	GLint
		vertAttrib = glGetAttribLocation(program_, "vPos"),
		texAttrib = glGetAttribLocation(program_, "vTex");
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	glGenBuffers(1, &vertBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer_);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertAttrib);
	glVertexAttribPointer(vertAttrib, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

ShaderProgram::~ShaderProgram() throw()
{
  glDeleteProgram(program_);
}

GLuint ShaderProgram::program()
{
	return program_;
}

void ShaderProgram::setFrag2Tex(float x, float y)
{
	if(frag2TexLoc_ == -1) {
		frag2TexLoc_ = glGetUniformLocation(program_, "frag2Tex");
	}
	glProgramUniform2f(program_, frag2TexLoc_, x, y);
}

void ShaderProgram::setOffset(float x, float y)
{
	if(offsetLoc_ == -1) {
		offsetLoc_ = glGetUniformLocation(program_, "offset");
	}
	glProgramUniform2f(program_, offsetLoc_, x, y);
}

void ShaderProgram::setBgColor(float r, float g, float b, float a)
{
	if(bgColorLoc_ == -1) {
		bgColorLoc_ = glGetUniformLocation(program_, "bgColor");
	}
	glProgramUniform4f(program_, bgColorLoc_, r, g, b, a);
}

void ShaderProgram::use()
{
	glUseProgram(program_);
}

void ShaderProgram::bindVertexArray()
{
	glBindVertexArray(vao_);
}

void ShaderProgram::unbindVertexArray()
{
	glBindVertexArray(0);
}

GLuint ShaderProgram::compileShader(const std::string &source, GLenum type)
{
  GLuint shader = glCreateShader(type);
	const char *cSource = source.c_str();
	glShaderSource(shader, 1, &cSource, 0);
	glCompileShader(shader);
	GLint status;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) //Compilation failure.
	{
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		GLchar* errorLog = new GLchar[logLength];
		glGetShaderInfoLog(shader, logLength, 0, errorLog);
		std::string errorString(errorLog);
		delete[] errorLog;
		std::stringstream err;
		err << "Failed to compile following shader source:\n"
			<< source << std::endl << "With error:\n"
			<< errorString;
		throw std::runtime_error(err.str());
	}
	return shader;
}
