cmake_minimum_required(VERSION 3.15)
project(PyPreSplash VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

include_directories(src/include)
set(SOURCES 
    src/bridge.cpp
)


if(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
    add_compile_definitions(NOMINMAX)
    list(APPEND SOURCES 
        src/platforms/win/splash_screen.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND SOURCES 
        src/platforms/linux/splash_screen.cpp
    )
endif()

pybind11_add_module(pypresplash ${SOURCES})

if(WIN32)
    target_link_libraries(pypresplash PRIVATE gdiplus user32 gdi32)
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11_CAIRO REQUIRED x11 cairo)
    target_include_directories(pypresplash PRIVATE ${X11_CAIRO_INCLUDE_DIRS})
    target_link_libraries(pypresplash PRIVATE ${X11_CAIRO_LIBRARIES})
    target_compile_options(pypresplash PRIVATE ${X11_CAIRO_CFLAGS_OTHER})
endif()


install(TARGETS pypresplash DESTINATION .)
