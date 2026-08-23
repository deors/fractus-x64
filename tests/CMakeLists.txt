set(TEST_TARGETS
    test_biomorphs
    test_fractals
    test_formats
    test_font
)

foreach(test_name ${TEST_TARGETS})
    add_executable(${test_name}
        ${test_name}.c
    )

    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
    )

    target_link_libraries(${test_name} PRIVATE
        fractus_core
    )

    if(MSVC)
        target_compile_options(${test_name} PRIVATE /W4)
    else()
        target_compile_options(${test_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
