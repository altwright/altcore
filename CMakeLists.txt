cmake_minimum_required(VERSION 3.30)
set(CMAKE_C_COMPILER gcc)
project(altcore C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(HEADER_FILEPATHS
        types.h
        arenas.h
        strings.h
        defer.h
        memory.h
        hashmap.h
        debug.h
        init.h
        worker.h
        events.h
        barrier.h
        fs.h
        draw/pixels.h
        draw/window.h
        draw/framebuffer.h
        draw/renderer.h
        draw/ui.h
        draw/fonts.h
)

set(SOURCE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

foreach(HEADER_FILEPATH ${HEADER_FILEPATHS})
    get_filename_component(DEST_DIR ${HEADER_FILEPATH} DIRECTORY)
    file(COPY ${SOURCE_INCLUDE_DIR}/${HEADER_FILEPATH} DESTINATION ${GENERATED_INCLUDE_DIR}/${PROJECT_NAME}/${DEST_DIR})
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
        worker.c
        events.c
        barrier.c
        fs.c
        draw/window.c
        draw/framebuffer.c
        draw/renderer.c
        draw/pixels.c
        draw/ui.c
        draw/ui_impl.c
        draw/fonts.c
        draw/fonts_impl.c
        draw/cmds/clear.c
        draw/cmds/present.c
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(libs/SDL3-3.4.4 EXCLUDE_FROM_ALL)

set(CGLM_SHARED OFF)
set(CGLM_STATIC ON)
add_subdirectory(libs/cglm-0.9.6 EXCLUDE_FROM_ALL)

add_subdirectory(libs/cwalk-1.2.9 EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3-static
        cglm
        cwalk
)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${GENERATED_INCLUDE_DIR}>
)

target_include_directories(${PROJECT_NAME} PUBLIC
        libs/clay-0.14
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wmissing-braces -Werror)