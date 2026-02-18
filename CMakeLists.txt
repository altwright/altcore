cmake_minimum_required(VERSION 4.1)
project(altcore C)

set(CMAKE_C_STANDARD 23)

add_library(altcore STATIC
        types.c
        types.h
        arenas.c
        arenas.h
        strings.c
        strings.h)
