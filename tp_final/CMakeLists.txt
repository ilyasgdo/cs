cmake_minimum_required(VERSION 3.10)
project(tp_final)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(tp_final tp_final.cpp common/GLShader.cpp)

target_include_directories(tp_final PRIVATE common)

target_link_libraries(tp_final PRIVATE glfw OpenGL::GL)
