
include(CTest)

project(test)

add_executable(global_test "${CMAKE_SOURCE_DIR}/test/tests.cpp")
target_link_libraries(global_test PRIVATE pretty)

add_test(NAME "Global Test" COMMAND global_test)
