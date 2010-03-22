# The name of our project is "xyz"
# root source directory = ${XYZ_DIR}
# root binary directory = ${XYZ_DIR}

cmake_minimum_required (VERSION 2.6)
project (xyz)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_NAME "xyz")
SET(CPACK_SYSTEM_NAME "amd64")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "glutg3 (>= 3.7), glutg3-dev (>= 3.7)")

set (CMAKE_VERBOSE_MAKEFILE OFF)
if(NOT MSVC)
  set (CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra -Wpointer-arith -fno-default-inline -fno-implicit-inline-templates")
endif(NOT MSVC)

# add_subdirectory (gl)

option (_UNITTEST "UnitTest" OFF)

if (_UNITTEST)
    set_property( SOURCE main.cpp
       APPEND PROPERTY COMPILE_DEFINITIONS _UNITTEST
    )
endif (_UNITTEST)

set ( SRC_FILES
aio_reader
aio_writer
flow
sensors/mesa
gl/glcamera
gl/glstdlib
gl/main
gl/view
main
)

set ( SRC 
${SRC_FILES}
)

set ( HEADERS
/usr/include
)

find_package(GLUT)
find_package(OpenGL)
find_package ( OpenCV REQUIRED )
 
set(PROGRAMS oglfirst pointers)
set(CORELIBS ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${OPENCV_LIBRARIES} mesasr m rt)
 
include_directories (${HEADERS})
add_executable (xyz ${SRC})
target_link_libraries (xyz ${CORELIBS} )

