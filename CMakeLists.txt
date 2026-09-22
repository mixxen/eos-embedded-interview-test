cmake_minimum_required(VERSION 3.16)
project(eos_embedded_interview LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

enable_testing()
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

function(add_exercise_target target implementation candidate_test_source label)
    add_executable(${target}
        ${implementation}
        ${candidate_test_source}
        tests/fake_uart.c
        tests/test_telemetry.c)
    target_include_directories(${target} PRIVATE include tests)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if(ENABLE_SANITIZERS)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
            target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer -g)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
            if(CMAKE_C_COMPILER_ID MATCHES "Clang")
                target_compile_options(${target} PRIVATE
                    -fsanitize-address-use-after-return=always)
            endif()
        else()
            message(FATAL_ERROR "This sanitizer option requires GCC or Clang, not MSVC")
        endif()
    endif()
    foreach(group packet send candidate)
        add_test(NAME ${label}.${group} COMMAND ${target} ${group})
        set_tests_properties(${label}.${group} PROPERTIES LABELS ${label} TIMEOUT 5)
    endforeach()
endfunction()

add_exercise_target(telemetry_tests src/telemetry.c tests/test_candidate.c candidate)
