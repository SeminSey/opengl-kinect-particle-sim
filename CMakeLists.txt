cmake_minimum_required(VERSION 3.22)
project(opengl-kinect-particle-sim LANGUAGES CXX)

# use C++23 and set executable output directory to bin/
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/")

# create executable
add_executable(${PROJECT_NAME} "src/main.cpp")
target_include_directories(${PROJECT_NAME} PRIVATE "include/")
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/pch.hpp")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

# collect all asset paths
file(GLOB_RECURSE ASSET_FILES
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" # base path
    CONFIGURE_DEPENDS "assets/*")          # globbing expression
# copy only recently modified or missing asset files next to the executable
foreach(ASSET_FILE ${ASSET_FILES})
    configure_file(${ASSET_FILE} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSET_FILE}" COPYONLY)
endforeach()

# add dependencies
include("cmake/opengl.cmake")
include("cmake/sdl.cmake")
include("cmake/glm.cmake")
include("cmake/libfreenect.cmake")
