add_executable(dummytest
        src/javaparsertest.cpp
        src/javaservicetest.cpp)

target_link_libraries(javatest ${GTEST_BOTH_LIBRARIES} pthread)

# Add a test to the project to be run by ctest
add_test(allJavaTest javatest)