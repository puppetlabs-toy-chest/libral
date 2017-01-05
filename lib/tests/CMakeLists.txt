# Setup compiling the test executable. C++ compile flags are inherited from
# the parent directory.
include_directories(${LEATHERMAN_CATCH_INCLUDE})

set(TEST_CASES ${PROJECT_NAME}.cc)

add_executable(libral_test $<TARGET_OBJECTS:libprojectsrc> ${TEST_CASES} main.cc)
target_link_libraries(libral_test
    ${LEATHERMAN_LIBRARIES}
    ${YAMLCPP_LIBRARIES}
    ${Boost_LIBRARIES}
    ${AUGEAS_LIBRARIES}
)

add_test(NAME "unit_tests" COMMAND libral_test)
