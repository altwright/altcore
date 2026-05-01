cmake_minimum_required(VERSION 3.30)
project(altcore C)

set(CMAKE_C_STANDARD 23)

set(HEADER_FILES
        types.h
        arenas.h
        strings.h
        defer.h
        memory.h
        hashmap.h
        debug.h
        init.h
)

set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

foreach(HEADER ${HEADER_FILES})
    get_filename_component(HEADER_NAME ${HEADER} NAME)
    configure_file(${HEADER} ${GENERATED_INCLUDE_DIR}/${PROJECT_NAME}/${HEADER_NAME} COPYONLY)
endforeach()

add_library(${PROJECT_NAME} STATIC
        types.c
        arenas.c
        strings.c
        defer.c
        memory.c
        hashmap.c
        debug.c
        init.c
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(libs/SDL3-3.4.4 EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3-static
)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${GENERATED_INCLUDE_DIR}>
)