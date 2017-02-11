add_test(NAME header_test COMMAND header_test)
add_executable(header_test header_test.cpp)
target_link_libraries(header_test "boost_unit_test_framework")
