cmake_minimum_required(VERSION 3.17)
project(flea)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_SOURCE_DIR})

option(STATIC_BUILD "enable static build" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include(FetchContent)
include(ExternalProject)

FetchContent_Declare(
        ELFIO
        GIT_REPOSITORY https://github.com/serge1/ELFIO
        GIT_TAG Release_3.9
)

FetchContent_MakeAvailable(ELFIO)

FetchContent_Declare(
        zero
        GIT_REPOSITORY https://github.com/Hackerl/zero
        GIT_TAG master
)

FetchContent_MakeAvailable(zero)

set(LIBUNWIND_BINARY ${CMAKE_BINARY_DIR}/libunwind-binary)
set(LIBUNWIND_INCLUDE_DIR ${LIBUNWIND_BINARY}/include)
set(LIBUNWIND_LIBRARY_DIR ${LIBUNWIND_BINARY}/lib)

file(MAKE_DIRECTORY ${LIBUNWIND_INCLUDE_DIR})

ExternalProject_Add(
        libunwind
        GIT_REPOSITORY https://github.com/libunwind/libunwind
        GIT_TAG v1.6.2
        SOURCE_DIR ${CMAKE_BINARY_DIR}/libunwind-src
        BUILD_IN_SOURCE ON
        UPDATE_DISCONNECTED ON
        CONFIGURE_COMMAND autoreconf -i && ./configure --prefix=${LIBUNWIND_BINARY} --enable-static=yes --enable-shared=no
)

add_library(libunwind-base STATIC IMPORTED)
add_dependencies(libunwind-base libunwind)

target_include_directories(libunwind-base INTERFACE ${LIBUNWIND_INCLUDE_DIR})
set_target_properties(libunwind-base PROPERTIES IMPORTED_LOCATION ${LIBUNWIND_LIBRARY_DIR}/libunwind.a)

add_library(libunwind-generic STATIC IMPORTED)

set_target_properties(libunwind-generic PROPERTIES IMPORTED_LOCATION ${LIBUNWIND_LIBRARY_DIR}/libunwind-generic.a)
target_link_libraries(libunwind-generic INTERFACE libunwind-base)

add_library(libunwind-ptrace STATIC IMPORTED)

set_target_properties(libunwind-ptrace PROPERTIES IMPORTED_LOCATION ${LIBUNWIND_LIBRARY_DIR}/libunwind-ptrace.a)
target_link_libraries(libunwind-ptrace INTERFACE libunwind-generic)

add_executable(flea main.cpp ptrace/tracee.cpp ptrace/executor.cpp inject/injector.cpp)
target_link_libraries(flea zero elfio libunwind-ptrace $<$<BOOL:${STATIC_BUILD}>:-static-pie>)