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

find_package(OpenMP)
if(OpenMP_C_FOUND)
    target_link_libraries(fractus_core PUBLIC OpenMP::OpenMP_C)
endif()

add_executable(fractus_x64
    app/main.c
    app/app.c
    app/commons.c
    app/files.c
    app/config.c
    app/fractal.c
)

target_include_directories(fractus_x64 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fractus_x64 PRIVATE
    fractus_core
)

if(MINGW)
    get_filename_component(MINGW_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)
    add_custom_command(TARGET fractus_x64 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_BIN_DIR}/libgomp-1.dll"
            "${MINGW_BIN_DIR}/libwinpthread-1.dll"
            "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
            $<TARGET_FILE_DIR:fractus_x64>
    )
endif()



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
