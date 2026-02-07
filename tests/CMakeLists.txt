add_executable(astrax_tests
    buffer_test.cpp
    command_test.cpp
)

target_link_libraries(astrax_tests PRIVATE
    astrax_core
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(astrax_tests)
