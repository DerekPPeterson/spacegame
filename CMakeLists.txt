cmake_minimum_required(VERSION 3.3.0)

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CPP_COMPILER clang-cl)

project(SPACEGAME C CXX)

include_directories(SYSTEM include src)
link_directories(SYSTEM lib)

file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB TEST_SOURCES "test/*.cpp" "test/.*c")

add_compile_definitions(NOMINMAX)
#add_compile_definitions(CURL_STATICLIB)

set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "/EHsc /W0 /std:c++latest")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")

set(LIBS bfd glfw3 GL X11 pthread Xrandr Xi dl assimp stdc++fs curlpp curl pistache)

#add_library(curl STATIC IMPORTED )
#set_target_properties(curl PROPERTIES IMPORTED_LOCATION "lib/libcurl.a")

add_library(spacegamelib STATIC ${SOURCES})
target_link_libraries(spacegamelib )

add_executable(app src/main.cxx )
target_link_libraries(app glfw3 libcurl-d_imp libcurlpp assimp-vc140-mt zlibstaticd spacegamelib )

#target_link_libraries(app pthread spacegamelib ${LIBS} )

#add_executable(textgame src/textgame.cxx src/backward.cpp)
#target_link_libraries(textgame bfd dl pthread spacegamelib ${LIBS})

#add_executable(server src/server.cxx )
#target_link_libraries(server spacegamelib ${LIBS})

#add_executable(tests test/tests.cxx ${TEST_SOURCES})
#target_link_libraries(tests spacegamelib ${LIBS})

#add_executable(startgame test/startgame.cxx)

