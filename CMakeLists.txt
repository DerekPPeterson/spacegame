cmake_minimum_required(VERSION 3.3.0)
project(learnopengl)

include_directories(SYSTEM include src)
file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB LOGIC_SOURCES "src/logic.cpp" "src/backward.cpp")
file(GLOB TEST_SOURCES "src/*.cpp" "src/*.c" "test/*.cpp" "test/.*c")

set(CMAKE_C_FLAGS "-Wall -Wmost -Werror -O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++17")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")

add_executable(app src/main.cxx ${SOURCES})
target_link_libraries(app glfw3 GL X11 pthread Xrandr Xi dl assimp bfd stdc++fs curlpp curl)

add_executable(textgame src/textgame.cxx ${LOGIC_SOURCES})
target_link_libraries(textgame glfw3 GL X11 pthread Xrandr Xi dl assimp bfd stdc++fs curlpp curl)

add_executable(server src/server.cxx ${LOGIC_SOURCES})
target_link_libraries(server glfw3 GL X11 pthread Xrandr Xi dl assimp bfd stdc++fs pistache curlpp curl)

add_executable(tests test/tests.cxx ${TEST_SOURCES})
target_link_libraries(tests glfw3 GL X11 pthread Xrandr Xi dl assimp bfd stdc++fs pistache curlpp curl)


#add_executable(hello_triangle src/hello_triangle.cpp src/glad.c )
#target_link_libraries(hello_triangle glfw3 GL X11 pthread Xrandr Xi dl)
