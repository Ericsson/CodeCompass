add_executable(dummytest
  src/dummyparsertest.cpp
  src/dummyservicetest.cpp)

target_link_libraries(dummytest ${GTEST_BOTH_LIBRARIES} pthread)

# Add a test to the project to be run by ctest
add_test(allDummyTest dummytest)