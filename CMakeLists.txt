cmake_minimum_required(VERSION 3.15)
project(PyPreSplash VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(pypresplash 
    src/splash_screen.cpp
    src/bridge.cpp
)

target_include_directories(pypresplash PRIVATE src/include)
target_compile_definitions(pypresplash PRIVATE UNICODE _UNICODE)
target_compile_definitions(pypresplash PRIVATE NOMINMAX)

target_link_libraries(pypresplash PRIVATE gdiplus user32 gdi32)

install(TARGETS pypresplash DESTINATION .)
