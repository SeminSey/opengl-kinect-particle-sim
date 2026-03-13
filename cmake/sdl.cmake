# try finding system package first
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
    # fetch and build it ourselves
    include(FetchContent)
    FetchContent_Declare(sdl
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        GIT_TAG "release-3.2.24"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(sdl)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)