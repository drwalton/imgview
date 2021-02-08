#include "SdlGlContext.hpp"
#include <string>
#include <stdexcept>
#include <iostream>

SdlGlContext::SdlGlContext(int initX, int initY, int initWidth, int initHeight)
{

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		throw std::runtime_error("Failed to initialize SDL");
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	Uint32 winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	window_ = SDL_CreateWindow("Imgview", initX, initY, initWidth, initHeight, winFlags);

	context_ = SDL_GL_CreateContext(window_);

	renderer_ = SDL_CreateRenderer(window_, -1, 0);

	if (SDL_GL_MakeCurrent(window_, context_)) {
		throw std::runtime_error("Unable to make SDL GL context current");
	}
	
	glewExperimental = GL_TRUE;
	GLenum r = glewInit();
	if (r != GLEW_OK) {
		throw std::runtime_error("Unable to initialize GLEW");
	}
	glGetError();

	
	const GLubyte* buf;
	buf = glGetString(GL_VERSION);
	std::cout << buf << std::endl;
}

SdlGlContext::~SdlGlContext() throw()
{
	SDL_DestroyRenderer(renderer_);
	SDL_DestroyWindow(window_);
}

SDL_Window *SdlGlContext::window()
{
	return window_;
}
