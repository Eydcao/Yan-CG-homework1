cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
include_directories(EIGEN3_INCLUDE_DIR)

include_directories(${CMAKE_SOURCE_DIR}/
      src/include/
      )

add_executable(Rasterizer
  src/main.cpp
  src/rasterizer.cpp
  src/Triangle.cpp
  )

target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})