cmake_minimum_required(VERSION 3.10)
project(tp2)

# La norme C++ à utiliser
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Trouve automatiquement les dossiers d'installation de GLFW et OpenGL (plus besoin de brew --prefix)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Indique le fichier source à compiler
add_executable(tp2 tp2.cpp common/GLShader.cpp)

target_include_directories(tp2 PRIVATE common)

# Lie (link) notre exécutable avec GLFW et OpenGL
target_link_libraries(tp2 PRIVATE glfw OpenGL::GL)
