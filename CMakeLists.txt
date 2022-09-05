set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.16)

include(cmake/vcpkg.cmake)

project("Crispy Doom" VERSION 5.7.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include(cmake/toolchain.cmake)

# Autotools variables
set(top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})
set(top_builddir ${CMAKE_CURRENT_BINARY_DIR})

# AC_INIT variables
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "crispy-doom")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/fabiangreffrath/crispy-doom/issues")

string(REGEX REPLACE " Doom$" "" PACKAGE_SHORTNAME "${PACKAGE_NAME}")
set(PACKAGE_COPYRIGHT "Copyright (C) 1993-2020")
set(PACKAGE_LICENSE "GNU General Public License, version 2")

# Any settings that should apply to all targets in this directory and all
# subdirectories should go here.  Use judiciously.
if(MSVC)
    add_definitions("/D_CRT_SECURE_NO_WARNINGS" "/D_CRT_SECURE_NO_DEPRECATE"
                    "/D_CRT_NONSTDC_NO_DEPRECATE")
else()
    add_compile_options("-Wall"
                        "-Wredundant-decls")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(sdl2-net CONFIG REQUIRED)
find_package(SampleRate CONFIG REQUIRED)
find_package(PNG)

set(HAVE_LIBSAMPLERATE TRUE)
set(HAVE_LIBPNG TRUE)

include(CheckSymbolExists)
include(CheckIncludeFile)
check_symbol_exists(strcasecmp "strings.h" HAVE_DECL_STRCASECMP)
check_symbol_exists(strncasecmp "strings.h" HAVE_DECL_STRNCASECMP)
#check_include_file("dirent.h" HAVE_DIRENT_H)

set(HAVE_DIRENT_H True)


string(CONCAT WINDOWS_RC_VERSION "${PROJECT_VERSION_MAJOR}, "
    "${PROJECT_VERSION_MINOR}, ${PROJECT_VERSION_PATCH}, 0")

# Without a hyphen. This is used for the bash-completion scripts.
string(TOLOWER "${PACKAGE_SHORTNAME}" PROGRAM_SPREFIX)

# With a hyphen, used almost everywhere else.
set(PROGRAM_PREFIX "${PROGRAM_SPREFIX}-")

configure_file(src/resource.rc.in src/resource.rc)
configure_file(src/setup-res.rc.in src/setup-res.rc)
configure_file(src/setup/setup-manifest.xml.in src/setup/setup-manifest.xml)

message(STATUS "${CMAKE_PROJECT_NAME}: Gathering DOOM_COMPILE_DEFINITIONS")

set(DOOM_COMPILE_DEFINITIONS PACKAGE_TARNAME="${PACKAGE_TARNAME}" PACKAGE_STRING="${PACKAGE_STRING}" PACKAGE_NAME="${PACKAGE_NAME}" PROGRAM_PREFIX="${PROGRAM_PREFIX}" PACKAGE_VERSION="${PACKAGE_VERSION}")

if(HAVE_LIBSAMPLERATE)
    list(APPEND DOOM_COMPILE_DEFINITIONS HAVE_LIBSAMPLERATE)
endif()

if(PNG_FOUND)
    list(APPEND DOOM_COMPILE_DEFINITIONS HAVE_LIBPNG)
endif()

if(HAVE_DIRENT_H)
    list(APPEND DOOM_COMPILE_DEFINITIONS HAVE_DIRENT_H)
endif()

if(HAVE_DECL_STRCASECMP)
    list(APPEND DOOM_COMPILE_DEFINITIONS HAVE_DECL_STRCASECMP)
endif()

if(HAVE_DECL_STRNCASECMP)
    list(APPEND DOOM_COMPILE_DEFINITIONS HAVE_DECL_STRNCASECMP)
endif()

message(STATUS "${CMAKE_PROJECT_NAME}: Found DOOM_COMPILE_DEFINITIONS : ${DOOM_COMPILE_DEFINITIONS}")

enable_testing()
include_directories(utils)

foreach(SUBDIR textscreen midiproc opl pcsound src test)
    add_subdirectory("${SUBDIR}")
endforeach()
