cmake_minimum_required(VERSION 3.0)


project(S3M2STM)
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(s3m2stm src/main.c)
# https://stackoverflow.com/a/50882216
if(MSVC)
  target_compile_options(s3m2stm PRIVATE /W4 /WX)
else()
  target_compile_options(s3m2stm PRIVATE -Wall -Wextra -Wpedantic -Werror -march=native)
endif()