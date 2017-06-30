# Set C++ compile flags based on project settings.
set(CMAKE_CXX_FLAGS ${${PROJECT_NAME_UPPER}_CXX_FLAGS})

# Set prefix for logging messages.
leatherman_logging_namespace("puppetlabs.gowrapper")

# Setup compiling the executable.
include_directories(
    ../lib/inc
    ${PROJECT_BINARY_DIR}/inc
    ${Boost_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${LEATHERMAN_INCLUDE_DIRS}
)

add_executable(wrapper wrapper.cpp)
target_link_libraries(wrapper libral)

link_directories(
  ${Boost_LIBRARY_DIRS}
  ${YAMLCPP_LIBRARY_DIRS})

leatherman_install(wrapper)

# Tests for the executable. These don't verify behavior, simply that the executable runs
# without crashing or generating an error. Useful, but should be enhanced with test scripts
# and/or acceptance tests.
add_test(NAME "smoke_test" COMMAND wrapper)
