cmake_minimum_required(VERSION 3.6)

project(imgview LANGUAGES CXX)

if(WIN32)
#Disables Boost's auto-linking, which often causes problems for me on Windows.
add_definitions( -DBOOST_ALL_NO_LIB )

add_definitions(/DGOOGLE_GLOG_DLL_DECL=)

#Needs to be defined for some libraries to link correctly (e.g.
# program_options).
add_definitions( -DBOOST_ALL_DYN_LINK  )
endif(WIN32)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

#Turn on modern C++ in Clang.
if(APPLE)
	set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif(APPLE)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(Boost COMPONENTS Filesystem System Program_Options Locale REQUIRED)

include_directories(include)
include_directories(.)
include_directories(${GLUE_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIRS})
include_directories(${SDL_ttf_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})

set(IMGVIEW_LIBRARIES
	${SDL_LIBRARY}
	${SDL_MAIN_LIBRARY}
	${SDL_ttf_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}

	optimized
		${glue_LIBRARIES_RELEASE}
	debug
		${glue_LIBRARIES_DEBUG}

	optimized
		${OPENCV_LIBRARIES}
	debug
		${OPENCV_DEBUG_LIBRARIES}

	${Boost_LIBRARIES}
)

add_executable(imgview WIN32
	ImgviewApp.cpp
	ImgviewApp.hpp

	SdlGlContext.cpp
	SdlGlContext.hpp

	ShaderProgram.cpp
	ShaderProgram.hpp

	imgview.cpp
)

target_link_libraries(imgview ${IMGVIEW_LIBRARIES})

message("Linking boost libraries: ${Boost_LIBRARIES}")
message("Final Library List: ${IMGVIEW_LIBRARIES}")
