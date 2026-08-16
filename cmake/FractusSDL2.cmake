include(FetchContent)

function(fractus_resolve_sdl2 out_target)
    set(sdl2_target "")

    find_package(SDL2 CONFIG QUIET)

    if(TARGET SDL2::SDL2)
        set(sdl2_target SDL2::SDL2)
    elseif(TARGET SDL2::SDL2-static)
        set(sdl2_target SDL2::SDL2-static)
    elseif(TARGET SDL2-static)
        set(sdl2_target SDL2-static)
    endif()

    if(NOT sdl2_target)
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
        set(SDL_TESTS OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            SDL2
            URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.11.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        )

        FetchContent_MakeAvailable(SDL2)

        if(TARGET SDL2::SDL2)
            set(sdl2_target SDL2::SDL2)
        elseif(TARGET SDL2::SDL2-static)
            set(sdl2_target SDL2::SDL2-static)
        elseif(TARGET SDL2-static)
            set(sdl2_target SDL2-static)
        endif()
    endif()

    if(NOT sdl2_target)
        message(FATAL_ERROR "Could not resolve an SDL2 target")
    endif()

    set(${out_target} "${sdl2_target}" PARENT_SCOPE)
endfunction()
