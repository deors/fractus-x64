include(${PROJECT_SOURCE_DIR}/cmake/FractusSDL2.cmake)

fractus_resolve_sdl2(FRACTUS_SDL2_TARGET)

add_library(fractus_core
    core/core.c
    core/fractal.c
    core/formats.c
)

target_include_directories(fractus_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(fractus_ui
    ui/font.c
    ui/ui.c
)

target_include_directories(fractus_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(fractus_platform
    platform/framebuffer.c
    platform/graphics.c
    platform/platform.c
)

target_include_directories(fractus_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fractus_platform PUBLIC
    ${FRACTUS_SDL2_TARGET}
)

target_link_libraries(fractus_ui PUBLIC
    fractus_platform
)

target_link_libraries(fractus_core PUBLIC
    fractus_ui
    fractus_platform
)

add_executable(fractus_x64
    app/main.c
    app/app.c
)

target_include_directories(fractus_x64 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fractus_x64 PRIVATE
    fractus_core
)

foreach(target fractus_core fractus_ui fractus_platform fractus_x64)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    if(FRACTUS_WARNINGS_AS_ERRORS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /WX)
        else()
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endforeach()
