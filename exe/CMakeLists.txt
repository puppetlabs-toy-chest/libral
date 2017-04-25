# Set C++ compile flags based on project settings.
set(CMAKE_CXX_FLAGS ${${PROJECT_NAME_UPPER}_CXX_FLAGS})

# Set prefix for logging messages.
leatherman_logging_namespace("puppetlabs.ralsh.command")

# Setup compiling the executable.
include_directories(
    ../lib/inc
    ../mruby/core/include
    ${PROJECT_BINARY_DIR}/inc
    ${Boost_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${LEATHERMAN_INCLUDE_DIRS}
)

add_executable(ralsh ralsh.cc)
target_link_libraries(ralsh libral)

# This is pretty hinky: we manually add -ldl because libmruby needs it;
# that should really be done with some cmake magic
add_executable(ral-check ral-check.cc)
target_link_libraries(ral-check
  libral
  ${PROJECT_BINARY_DIR}/mruby/host/lib/libmruby.a dl)

link_directories(
  ${Boost_LIBRARY_DIRS}
  ${YAMLCPP_LIBRARY_DIRS})

leatherman_install(ralsh)

# Tests for the executable. These don't verify behavior, simply that the executable runs
# without crashing or generating an error. Useful, but should be enhanced with test scripts
# and/or acceptance tests.
add_test(NAME "smoke_test" COMMAND ralsh)
add_test(NAME "help_test" COMMAND ralsh --help)
add_test(NAME "version_test" COMMAND ralsh --version)
