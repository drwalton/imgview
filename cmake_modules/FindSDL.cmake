find_path(SDL_INCLUDE_DIRS
	NAMES
		SDL.h
	PATHS
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.4/include
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.5/include
		C:/local/SDL/SDL2-2.0.5/include
		"${SDL_DIR}/include"
		/opt/local/include/SDL2
		/usr/local/include/SDL2
)

find_library(SDL_LIBRARY
	NAMES
		SDL2
	PATHS
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.4/lib/x64
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.5/lib/x64
		C:/local/SDL/SDL2-2.0.5/lib/x64
		"${SDL_DIR}/lib/x64"
		/Library/Frameworks
)

find_library(SDL_MAIN_LIBRARY
	NAMES
		SDL2Main
	PATHS
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.4/lib/x64
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.5/lib/x64
		C:/local/SDL/SDL2-2.0.5/lib/x64
		"${SDL_DIR}/lib/x64"
		/Library/Frameworks
)

set(SDL_LIBRARIES
	${SDL_LIBRARY}
	${SDL_MAIN_LIBRARY})

message("SDL2 Library: ${SDL_LIBRARIES}")
