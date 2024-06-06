cmake_minimum_required(VERSION 3.10)
project(atto)

file(GLOB_RECURSE sources
     src/*.cpp
     src/*.h*
     src/lib/*.cpp
     src/lib/*.h*)

add_executable(atto ${sources})

target_compile_options(atto PRIVATE -Werror -Wall -Wextra)
