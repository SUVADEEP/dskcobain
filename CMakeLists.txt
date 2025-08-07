cmake_minimum_required(VERSION 3.10)
project(kcobain C CXX)

# Project configuration
set(LOGGER_TAG "${PROJECT_NAME}" CACHE STRING "Logger tag for the application")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Android NDK support
if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # Android NDK specific settings
    set(CMAKE_FIND_ROOT_PATH ${ANDROID_NDK})
    set(CMAKE_SYSTEM_NAME Android)
    set(CMAKE_SYSTEM_VERSION ${ANDROID_NATIVE_API_LEVEL})
    # Set Android platform and ABI
    set(ANDROID_PLATFORM android-33)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a armeabi-v7a)
    # Force C++11 for Android
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
    # Add Core Audio framework for macOS
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
    # Add ALSA for Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /std:c++11")
endif()

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

# Define project constants for the application
add_definitions(-DLOGGER_TAG="${LOGGER_TAG}")
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DKCobain_VERSION="1.0.0")

# Copy header files to build directory (for development)
file(COPY ${CMAKE_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR})

# Create core audio library (audio_rb_controller only)
add_library(kcobain_core STATIC
    src/utils/logger.cpp
    src/miniaudio_impl.cpp
    src/core/audio_rb_controller.cpp
)



# Create USB audio library (separate from core)
add_library(kcobain_usb STATIC
    src/core/usb_audio_producer.cpp
    src/core/usb_audio_consumer.cpp
    src/core/usb_audio_orchestrator.cpp
)

# Link core library to USB library
target_link_libraries(kcobain_usb kcobain_core)



# Create main executable (depends on USB library)
add_executable(kcobain 
    src/main.cpp
)

# Link libraries to main executable
target_link_libraries(kcobain kcobain_usb)

# Link libraries based on platform
if(APPLE)
    target_link_libraries(kcobain_core ${COREAUDIO_FRAMEWORK} ${AUDIOTOOLBOX_FRAMEWORK})
    target_link_libraries(kcobain_usb ${COREAUDIO_FRAMEWORK} ${AUDIOTOOLBOX_FRAMEWORK})

    target_link_libraries(kcobain ${COREAUDIO_FRAMEWORK} ${AUDIOTOOLBOX_FRAMEWORK})
elseif(UNIX)
    target_link_libraries(kcobain_core ${ALSA_LIBRARIES})
    target_include_directories(kcobain_core PRIVATE ${ALSA_INCLUDE_DIRS})
    target_compile_options(kcobain_core PRIVATE ${ALSA_CFLAGS_OTHER})
    
    target_link_libraries(kcobain_usb ${ALSA_LIBRARIES})
    target_include_directories(kcobain_usb PRIVATE ${ALSA_INCLUDE_DIRS})
    target_compile_options(kcobain_usb PRIVATE ${ALSA_CFLAGS_OTHER})
    

    
    target_link_libraries(kcobain ${ALSA_LIBRARIES})
    target_include_directories(kcobain PRIVATE ${ALSA_INCLUDE_DIRS})
    target_compile_options(kcobain PRIVATE ${ALSA_CFLAGS_OTHER})
elseif(WIN32)
    # Windows doesn't need additional libraries for miniaudio
endif()

# Add math library (for sinf, etc.) - only link to the main executable
target_link_libraries(kcobain m)

# Set output directory
set_target_properties(kcobain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install headers (for library usage)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/kcobain
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

# Install libraries
install(TARGETS kcobain_core kcobain_usb
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

# Install executable
install(TARGETS kcobain
        RUNTIME DESTINATION bin)
