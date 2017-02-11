add_test(NAME myclient_test COMMAND myclient_test)
add_executable(myclient_test myclient_test.cpp)
target_link_libraries(myclient_test "boost_unit_test_framework")
