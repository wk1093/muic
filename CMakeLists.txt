cmake_minimum_required(VERSION 3.26)
project(muic)
# download a single-header library
include(FetchContent)
FetchContent_Declare(
        mus
        GIT_REPOSITORY https://github.com/Muukid/mus.git
        GIT_TAG        main
)
# can't use FetchContent_MakeAvailable because mus is not a CMake project, it is a single header
FetchContent_GetProperties(mus)
if(NOT mus_POPULATED)
    FetchContent_Populate(mus)
    add_library(mus INTERFACE)
    target_include_directories(mus INTERFACE ${mus_SOURCE_DIR})
endif()


add_executable(muic main.c
        muic.h)
target_link_libraries(muic mus)
