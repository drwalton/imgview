set(HBREW_CV_PATH
		/usr/local/Cellar/opencv3/3.2.0)

find_path(OPENCV_INCLUDE_DIRS
	NAMES
		opencv2/opencv.hpp
	PATHS
		/usr/include
		/opt/local/include
		/usr/local/include
		${HBREW_CV_PATH}/include
		"C:/local/opencv/opencv/build/install/include"
		"F:/local/opencv/opencv/build/install/include"
		"G:/local/opencv/opencv/build/install/include"
		"C:/local/opencv/opencv/build/include"
		"F:/local/opencv/opencv/build/include"
		"G:/local/opencv/opencv/build/include"
		$ENV{WIN_LOCAL_DIR}/opencv/opencv-3.0.0/build/include
		$ENV{WIN_LOCAL_DIR}/opencv/opencv-3.1.0/build/include
		$ENV{WIN_LOCAL_DIR}/opencv/opencv-3.2.0/build/include
		"C:/local/opencv/opencv-3.0.0/build/include"
		"C:/local/opencv/opencv-3.1.0/build/include"
		"G:/local/opencv/opencv-3.1.0/build/include"
		"F:/local/opencv/opencv/vsbuild/install/include"
		"F:/local/opencv/opencv/vsbuild/install/x64/vc14/lib/"
	NO_DEFAULT_PATH
)

if(NOT ${OPENCV_INCLUDE_DIRS} EQUAL OPENCV_INCLUDE_DIRS-NOTFOUND)
	set(OPENCV_FOUND TRUE)
endif(NOT ${OPENCV_INCLUDE_DIRS} EQUAL OPENCV_INCLUDE_DIRS-NOTFOUND)

set(LIB_PATHS
		/usr/lib
		/usr/local/lib
		/opt/local/lib
		${HBREW_CV_PATH}/lib
		$ENV{WIN_LOCAL_DIR}/opencv/opencv/vsbuild/install/x64/vc14/lib
		$ENV{WIN_LOCAL_DIR}/opencv/opencv/build/lib/Release
		$ENV{WIN_LOCAL_DIR}/opencv/opencv/build/lib/Debug
		$ENV{WIN_LOCAL_DIR}/opencv/opencv-3.1.0/build/x64/vc12/lib
		$ENV{WIN_LOCAL_DIR}/opencv/opencv-3.2.0/build/x64/vc12/lib
		"F:/local/opencv/opencv/vsbuild/install/x64/vc14/lib"
		"C:/local/opencv/opencv/build/install/x64/vc14/lib"
		"F:/local/opencv/opencv/build/install/x64/vc14/lib"
		"G:/local/opencv/opencv/build/install/x64/vc14/lib"
		"C:/local/opencv/opencv/build/lib/Release"
		"C:/local/opencv/opencv/build/lib/Debug"
		"F:/local/opencv/opencv/build/lib/Release"
		"F:/local/opencv/opencv/build/lib/Debug"
		"G:/local/opencv/opencv/build/lib/Release"
		"G:/local/opencv/opencv/build/lib/Debug"
		"C:/local/opencv/opencv-3.0.0/build/x64/vc12/lib"
		"C:/local/opencv/opencv-3.1.0/build/x64/vc12/lib"
		"G:/local/opencv/opencv-3.1.0/build/x64/vc12/lib"
)


function(FIND_OPENCV_LIB LIBNAME)
	find_library(OPENCV_${LIBNAME}_LIBRARY
		NAMES
			opencv_${LIBNAME}
			opencv_${LIBNAME}331
			opencv_${LIBNAME}330
			opencv_${LIBNAME}320
			opencv_${LIBNAME}430
		PATHS
			${LIB_PATHS}
		NO_DEFAULT_PATH
	)

if(WIN32)
	find_library(OPENCV_${LIBNAME}_LIBRARY_DEBUG
		NAMES
			opencv_${LIBNAME}d
			opencv_${LIBNAME}331d
			opencv_${LIBNAME}330d
			opencv_${LIBNAME}320d
			opencv_${LIBNAME}430d
		PATHS
			${LIB_PATHS}
		NO_DEFAULT_PATH
	)
else(WIN32)
	set(OPENCV_${LIBNAME}_LIBRARY_DEBUG ${OPENCV_${LIBNAME}_LIBRARY})
endif(WIN32)


set(OPENCV_LIBRARIES_RELEASE ${OPENCV_LIBRARIES} ${OPENCV_${LIBNAME}_LIBRARY} PARENT_SCOPE)
set(OPENCV_LIBRARIES_DEBUG ${OPENCV_LIBRARIES_DEBUG} ${OPENCV_${LIBNAME}_LIBRARY_DEBUG} PARENT_SCOPE)

set(OPENCV_LIBRARIES ${OPENCV_LIBRARIES} optimized ${OPENCV_${LIBNAME}_LIBRARY} debug ${OPENCV_${LIBNAME}_LIBRARY_DEBUG} PARENT_SCOPE)
endfunction()


#FIND_OPENCV_LIB(calib3d)
#FIND_OPENCV_LIB(core)
#FIND_OPENCV_LIB(features2d)
#FIND_OPENCV_LIB(flann)
#FIND_OPENCV_LIB(highgui)
#FIND_OPENCV_LIB(imgproc)
#FIND_OPENCV_LIB(photo)
#FIND_OPENCV_LIB(video)
#FIND_OPENCV_LIB(videoio)
#FIND_OPENCV_LIB(imgcodecs)
#FIND_OPENCV_LIB(ximgproc)
FIND_OPENCV_LIB(world)
message("OPENCV ${OPENCV_LIBRARIES}")

