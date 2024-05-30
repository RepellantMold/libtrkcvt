cmake_minimum_required(VERSION 3.0)


project(SCREAMVERTER)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(include)

add_executable( screamverter
                src/main.c
                src/log.c
                src/file.c
                src/crc.c
                src/header.c
                src/parapnt.c
                src/pattern.c
                src/sample.c
                src/tostm.c
                src/tostx.c)