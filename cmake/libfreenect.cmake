find_package(libusb-1.0 CONFIG QUIET)
if(NOT libusb-1.0_FOUND)
    # fetch and build it ourselves
    include(FetchContent)
    FetchContent_Declare(libusb
        GIT_REPOSITORY "https://github.com/libusb/libusb-cmake.git"
        GIT_TAG "v1.0.29-0"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(libusb)
endif()

# libusb naming is a mess across distros
if(NOT TARGET libusb::usb-1.0)
    if(TARGET usb-1.0)
        add_library(libusb::usb-1.0 ALIAS usb-1.0)
    elseif(TARGET libusb-1.0)
        add_library(libusb::usb-1.0 ALIAS libusb-1.0)
    else()
        message(FATAL_ERROR "Could not resolve a libusb target")
    endif()
endif()

find_package(libfreenect CONFIG QUIET)
if(NOT libfreenect_FOUND)
    # fetch and build it ourselves
    include(FetchContent)

    # need to force off otherwise it builds cpp examples too (???)
    set(BUILD_CPP ON CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_FAKENECT OFF CACHE BOOL "" FORCE)
    set(BUILD_C_SYNC OFF CACHE BOOL "" FORCE)
    set(BUILD_CV OFF CACHE BOOL "" FORCE)
    set(BUILD_AS3_SERVER OFF CACHE BOOL "" FORCE)
    set(BUILD_PYTHON OFF CACHE BOOL "" FORCE)
    set(BUILD_PYTHON2 OFF CACHE BOOL "" FORCE)
    set(BUILD_PYTHON3 OFF CACHE BOOL "" FORCE)
    set(BUILD_OPENNI2_DRIVER OFF CACHE BOOL "" FORCE)
    set(BUILD_CPACK_DEB OFF CACHE BOOL "" FORCE)
    set(BUILD_CPACK_RPM OFF CACHE BOOL "" FORCE)
    set(BUILD_CPACK_TGZ OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(libfreenect
        GIT_REPOSITORY "https://github.com/OpenKinect/libfreenect.git"
        GIT_TAG "v0.7.5"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(libfreenect)

    if(TARGET freenect)
        target_link_libraries(freenect libusb::usb-1.0)
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${libfreenect_SOURCE_DIR}/include"
        "${libfreenect_SOURCE_DIR}/wrappers/cpp")
elseif(DEFINED FREENECT_INCLUDE_DIRS)
    target_include_directories(${PROJECT_NAME} PRIVATE ${FREENECT_INCLUDE_DIRS})
endif()

if(DEFINED FREENECT_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FREENECT_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE freenect)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE libusb::usb-1.0)