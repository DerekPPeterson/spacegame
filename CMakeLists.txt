cmake_minimum_required(VERSION 3.3.0)
project(SPACEGAME C CXX)

include_directories(SYSTEM include src)
file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB TEST_SOURCES "test/*.cpp" "test/.*c")

set(CMAKE_C_FLAGS "-Wall -Wextra -O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++17")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")

set(LIBS bfd glfw3 GL X11 pthread Xrandr Xi dl assimp stdc++fs curlpp curl pistache)

add_library(spacegamelib STATIC ${SOURCES})

add_executable(app src/main.cxx)
target_link_libraries(app pthread spacegamelib ${LIBS} )

add_executable(textgame src/textgame.cxx )
target_link_libraries(textgame spacegamelib ${LIBS})

add_executable(server src/server.cxx )
target_link_libraries(server spacegamelib ${LIBS})

add_executable(tests test/tests.cxx ${TEST_SOURCES})
target_link_libraries(tests spacegamelib ${LIBS})


#add_executable(hello_triangle src/hello_triangle.cpp src/glad.c )
#target_link_libraries(hello_triangle glfw3 GL X11 pthread Xrandr Xi dl)
