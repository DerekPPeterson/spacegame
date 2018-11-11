cmake_minimum_required(VERSION 3.0.0)
project(learnopengl)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
#file(GLOB SOURCES "src/*.cpp")

add_executable(app src/glad.c src/app.cpp src/shader.cpp src/mesh.cpp)
target_link_libraries(app glfw3 GL X11 pthread Xrandr Xi dl assimp)

#add_executable(hello_triangle src/hello_triangle.cpp src/glad.c )
#target_link_libraries(hello_triangle glfw3 GL X11 pthread Xrandr Xi dl)
