cmake_minimum_required(VERSION 3.30)
project(altcore C)

set(CMAKE_C_STANDARD 23)

add_library(altcore STATIC
        types.c
        types.h
        arenas.c
        arenas.h
        strings.c
        strings.h
        defer.c
        defer.h
        memory.c
        memory.h
        hashmap.c
        hashmap.h)
