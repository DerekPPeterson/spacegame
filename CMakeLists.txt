cmake_minimum_required(VERSION 3.3.0)
project(learnopengl)

set (CMAKE_CXX_STANDARD 17)

include_directories(include)
file(GLOB SOURCES "src/*.cpp" "src/*.c")

add_executable(app ${SOURCES})
target_link_libraries(app glfw3 GL X11 pthread Xrandr Xi dl assimp bfd)
target_compile_options(app PUBLIC -Wall -O3)

#add_executable(hello_triangle src/hello_triangle.cpp src/glad.c )
#target_link_libraries(hello_triangle glfw3 GL X11 pthread Xrandr Xi dl)
