# Defines how cmake should behave, and the minimum version necessary to build.
cmake_minimum_required(VERSION 3.2.2)

# Project Setup - modify to match project naming
## Source code for a simple command-line executable for a dynamic library will be generated from the project name.
## The command-line and library names will be based off the project name.
project(libral VERSION 0.0.1)

# Whether to build a static executable. Only works with pl-build-tools for
# now since Fedora is missing augeas-static, icu-static, and yaml-cpp-static
option(LIBRAL_STATIC "Build a static executable")

# Build a static libral library
option(LIBRAL_LIBRAY_STATIC "Build a static library" OFF)

set(USE_STATIC_LIBS OFF)
if(LIBRAL_STATIC OR LIBRAL_LIBRAY_STATIC)
    set(USE_STATIC_LIBS ON)
endif()

# NB: this does not work on Mac due to FindAugeas.cmake logic being for linux
option(AUGEAS_STATIC "Use static libaugeas" OFF)

# The default data dir. Providers should be placed into
set(LIBRAL_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/libral/data" CACHE STRING "where to look for providers etc. by default")

# The default libexec dir. mruby will be installed there
set(LIBRAL_LIBEXEC_DIR "${CMAKE_INSTALL_PREFIX}/libexec/libral" CACHE STRING "where to install and look for mruby and other auxiliary executables")

string(MAKE_C_IDENTIFIER ${PROJECT_NAME} PROJECT_C_NAME)
string(TOUPPER ${PROJECT_C_NAME} PROJECT_NAME_UPPER)
string(TOLOWER ${PROJECT_C_NAME} PROJECT_NAME_LOWER)

# Common cmake setup
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Defaulting to a release build.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(YAMLCPP_STATIC "Use yaml-cpp's static libraries" ${USE_STATIC_LIBS})
find_package(YAMLCPP REQUIRED)

# Leatherman setup
SET(LEATHERMAN_COMPONENTS locale catch nowide logging execution json_container)
SET(BOOST_STATIC ${USE_STATIC_LIBS})
find_package(Leatherman REQUIRED COMPONENTS ${LEATHERMAN_COMPONENTS})

# Now that we have leatherman, we can pull in its options file, which
# we need for finding all our other libraries.
include(options)
## Pull in common cflags setting from leatherman. Don't override CMAKE_CXX_FLAGS at the project root to avoid impacting 3rd party code.
include(cflags)
set(LIBRAL_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS}")

check_cxx_compiler_flag("-Wsuggest-override" HAS_SUGGEST_OVERRIDE)
if (HAS_SUGGEST_OVERRIDE)
  set(LIBRAL_CXX_FLAGS "${LIBRAL_CXX_FLAGS} -Wsuggest-override")
endif(HAS_SUGGEST_OVERRIDE)

# On OS X, git_t is an unsigned int, though signed int on most other platforms
if (APPLE)
    list(APPEND LEATHERMAN_DEFINITIONS -DHAVE_UNSIGNED_GID_T)
endif()

add_definitions(${LEATHERMAN_DEFINITIONS})

## Pull in helper macros for working with leatherman libraries
include(leatherman)

# Add other dependencies
set(Boost_USE_STATIC_LIBS ${USE_STATIC_LIBS})
find_package(Boost 1.54 REQUIRED COMPONENTS program_options)

find_package(Augeas REQUIRED)
add_definitions(${AUGEAS_DEFINITIONS})

find_package(LibXml2 REQUIRED)
if ("${LIBXML2_INCLUDE_DIRS}" STREQUAL "")
    find_path(LIBXML2_INCLUDE_DIRS libxml/tree.h PATH_SUFFIXES libxml2)
    if ("${LIBXML2_INCLUDE_DIRS}" STREQUAL "LIBXML2_INCLUDE_DIRS-NOTFOUND")
        message(FATAL_ERROR "LibXml2 was not found")
    endif()
endif()

# Display a summary of the features
include(FeatureSummary)
feature_summary(WHAT ALL)

add_subdirectory(lib)
add_subdirectory(exe)
add_subdirectory(mruby)

configure_file("${PROJECT_SOURCE_DIR}/config.hpp.in"
               "${PROJECT_BINARY_DIR}/inc/config.hpp")
include_directories(${PROJECT_BINARY_DIR}/inc)

# Add cpplint and cppcheck targets
file(GLOB_RECURSE ALL_SOURCES lib/src/**/*.cc lib/inc/*.hpp exe/*.cc)
add_cpplint_files(${ALL_SOURCES})
enable_cpplint()

add_cppcheck_dirs("${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/exe")
enable_cppcheck()

# Add docygen target
# - first we can indicate the documentation build as an option and set it
# to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the
    # application
    add_custom_target(doc
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# This will be used to install all data:
file(GLOB_RECURSE ALL_LENSES "${CMAKE_SOURCE_DIR}/data/lenses/*.aug")
file(GLOB_RECURSE ALL_PROVIDERS "${CMAKE_SOURCE_DIR}/data/providers/*.prov")

install(FILES ${ALL_LENSES} DESTINATION "${LIBRAL_DATA_DIR}/lenses/")
install(PROGRAMS ${ALL_PROVIDERS} DESTINATION "${LIBRAL_DATA_DIR}/providers/")

# Packing a static tarball
add_custom_target("statpack")
add_dependencies("statpack" ralsh mruby)
add_custom_command(TARGET "statpack"
  COMMENT "Build a distributable minimal tarball"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND "${PROJECT_SOURCE_DIR}/contrib/statpack"
          "${PROJECT_BINARY_DIR}")
