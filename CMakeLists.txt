cmake_minimum_required(VERSION 3.0)


project(SCREAMVERTER)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(screamverter src/main.c)
# https://stackoverflow.com/a/50882216
if(MSVC)
  target_compile_options(screamverter PRIVATE /W4)
else()
  target_compile_options(screamverter PRIVATE -Wall -Wextra -Wpedantic -Werror -march=native)
endif()