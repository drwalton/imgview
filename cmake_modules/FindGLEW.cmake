find_path(GLEW_INCLUDE_DIRS
	NAMES
		GL/glew.h
	PATHS
		C:/local/glew/glew-2.1.0/include
)

find_library(GLEW_LIBRARY
	NAMES
		glew32
	PATHS
		C:/local/glew/glew-2.1.0/lib/Release/x64
)


set(GLEW_LIBRARIES ${GLEW_LIBRARY})
