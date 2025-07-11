cmake_minimum_required(VERSION 3.31)
project(LEDMatrixSimulator)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
set(MAGICKCORE_QUANTUM_DEPTH 8)
set(MAGICKCORE_HDRI_ENABLE 0)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
if(NOT ImageMagick_FOUND)
    message(FATAL_ERROR "ImageMagick not found. Please install it or set the ImageMagick_DIR variable.")
endif()
include_directories(${ImageMagick_INCLUDE_DIRS})
add_definitions(-DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
add_executable(LEDMatrixSimulator main.cpp)
target_link_libraries(LEDMatrixSimulator PRIVATE sfml-graphics sfml-window sfml-system)
target_link_libraries(LEDMatrixSimulator PRIVATE ${ImageMagick_LIBRARIES})