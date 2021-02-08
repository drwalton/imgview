#ifndef SDLGLCONTEXT_HPP_INCLUDED
#define SDLGLCONTEXT_HPP_INCLUDED

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <GL/glew.h>

class SdlGlContext final
{
public:
	SdlGlContext(int initX, int initY, int initWidth, int initHeight);
	~SdlGlContext() throw();

	SDL_Window *window();
private:
	SDL_Renderer *renderer_;
	SDL_Window *window_;
	SDL_GLContext context_;
};

#endif
