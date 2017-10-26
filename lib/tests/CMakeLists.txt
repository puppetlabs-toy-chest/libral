# Setup compiling the test executable. C++ compile flags are inherited from
# the parent directory.
include_directories(${LEATHERMAN_CATCH_INCLUDE})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/fixtures.hpp.in"
               "${PROJECT_BINARY_DIR}/inc/fixtures.hpp")

set(TEST_CASES file.cc ${PROJECT_NAME}.cc json_provider.cc)

add_executable(libral_test $<TARGET_OBJECTS:libprojectsrc> ${TEST_CASES} fixtures.cc attr/spec.cc main.cc)
target_link_libraries(libral_test libral)

add_test(NAME "unit_tests" COMMAND libral_test)
