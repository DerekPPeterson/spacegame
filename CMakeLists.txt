cmake_minimum_required(VERSION 3.0.0)
project(learnopengl)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(app src/glad.c ${SOURCES})
target_link_libraries(app glfw3 GL X11 pthread Xrandr Xi dl)
