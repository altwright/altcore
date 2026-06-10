cmake_minimum_required(VERSION 3.30)
project(altcore C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ALTCORE_SHARED_LIB "Build as a shared library" OFF)

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
        maths.h
        draw/pixels.h
        draw/window.h
        draw/framebuffer.h
        draw/renderer.h
        draw/ui.h
        draw/fonts.h
        ecs.h
)

set(SOURCE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

foreach(HEADER_FILEPATH ${HEADER_FILEPATHS})
    get_filename_component(DEST_DIR ${HEADER_FILEPATH} DIRECTORY)
    file(COPY ${SOURCE_INCLUDE_DIR}/${HEADER_FILEPATH} DESTINATION ${GENERATED_INCLUDE_DIR}/${PROJECT_NAME}/${DEST_DIR})
endforeach()

set(SOURCE_FILEPATHS
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
    maths.c
    ecs.c
    draw/window.c
    draw/framebuffer.c
    draw/renderer.c
    draw/pixels.c
    draw/ui.c
    draw/ui_impl.c
    draw/fonts.c
    draw/cmds/clear.c
    draw/cmds/draw_rect.c
    draw/cmds/draw_text.c
    draw/cmds/present.c
    draw/cmds/scissor.c
)

if (ALTCORE_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED
        ${SOURCE_FILEPATHS}
    )

    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)

    set(CGLM_SHARED ON)
    set(CGLM_STATIC OFF)
else()
    add_library(${PROJECT_NAME} STATIC
        ${SOURCE_FILEPATHS}
    )

    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)

    set(CGLM_SHARED OFF)
    set(CGLM_STATIC ON)
endif()

add_subdirectory(libs/SDL3-3.4.4 EXCLUDE_FROM_ALL)

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

target_compile_options(${PROJECT_NAME} PUBLIC -Wmissing-braces)