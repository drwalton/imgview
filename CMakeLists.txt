cmake_minimum_required(VERSION 3.6)

project(imgview LANGUAGES CXX)

add_definitions(/DGOOGLE_GLOG_DLL_DECL=)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SDL REQUIRED)

include_directories(include)
include_directories(.)
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})

set(IMGVIEW_LIBRARIES
	${SDL_LIBRARY}
	${SDL_MAIN_LIBRARY}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}

	optimized
		${OPENCV_LIBRARIES}
	debug
		${OPENCV_world_LIBRARY_DEBUG}
)

add_executable(imgview WIN32
	ImgviewApp.cpp
	ImgviewApp.hpp

	SdlGlContext.cpp
	SdlGlContext.hpp

	ShaderProgram.cpp
	ShaderProgram.hpp

	imgview.cpp
	alphanum.hpp
)

target_link_libraries(imgview ${IMGVIEW_LIBRARIES})

target_compile_features(imgview PRIVATE cxx_std_20)

message("Final Library List: ${IMGVIEW_LIBRARIES}")
