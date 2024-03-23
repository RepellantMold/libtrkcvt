cmake_minimum_required(VERSION 3.0)

project(S3M2STM)
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(s3m2stm src/main.c src/sample.c src/pattern.c)
target_include_directories(s3m2stm PRIVATE include)

