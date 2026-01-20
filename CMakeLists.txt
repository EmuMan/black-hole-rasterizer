cmake_minimum_required(VERSION 3.16)
include("${CMAKE_SOURCE_DIR}/FindGfxLibs.cmake")

project(BlackHoleRasterizer)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/ext/*/*.cpp" "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/ext/*/*.h" "${CMAKE_SOURCE_DIR}/ext/glad/*/*.h")
file(GLOB_RECURSE GLSL "${CMAKE_SOURCE_DIR}/resources/*.glsl")

include_directories("ext")
include_directories("ext/glad/include")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

findGLFW3(${CMAKE_PROJECT_NAME})
findGLM(${CMAKE_PROJECT_NAME})

if(NOT WIN32)
  message(STATUS "Adding GCC style compiler flags")
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE "-Wall" "-pedantic" "-Werror=return-type")

  if(APPLE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
  else()
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "GL" "dl")
  endif()
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "opengl32.lib")
endif()
