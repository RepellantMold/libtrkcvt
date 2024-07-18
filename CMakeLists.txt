cmake_minimum_required(VERSION 3.0)

project(libtrkcvt)
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)

add_definitions(-DLOGLEVEL=3)

include_directories(include)
include_directories(extern)

add_library( libtrkcvt
                src/effects.c
                src/file.c
                src/crc.c
                src/valid.c
                src/song_header.c
                src/orders.c
                src/sample_header.c
                src/parapnt.c
                src/pattern.c
                src/sample.c)
