cmake_minimum_required(VERSION 3.10)
project(robo VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    set(WARN_EXT /W2)
else()
    set(THREAD_LIB pthread)
    set(THREAD_OPTIONS -pthread)
endif()

# Warning level setup #######################################################

# GCC/Clang common warnings
set(GCC_CLANG_WARNINGS -Werror -Wall -Wextra -Wnon-virtual-dtor -pedantic
-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function)

set(MSVC_WARNINGS /W2)

# GCC specific warnings
set(GCC_WARNINGS
        -Wduplicated-branches
        -Wduplicated-cond
        -Wlogical-op
        -Wcast-align
        -Wno-missing-field-initializers)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(WARNINGS ${WARNINGS} ${GCC_WARNINGS} ${GCC_CLANG_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(WARNINGS ${WARNINGS} ${CLANG_WARNINGS} ${GCC_CLANG_WARNINGS})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(WARNINGS ${WARNINGS} ${MSVC_WARNINGS})
endif()


add_subdirectory(ansi)
add_subdirectory(../mrb mrb)
add_subdirectory(../pix pix)

add_executable(robo src/main.cpp)
target_link_libraries(robo PRIVATE pix mrb::mrb ansi)
