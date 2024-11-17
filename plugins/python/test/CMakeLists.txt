if (NOT FUNCTIONAL_TESTING_ENABLED)
  fancy_message("Skipping generation of test project pythontest." "yellow" TRUE)
  return()
endif()

fancy_message("Generating test project for pythontest." "blue" TRUE)

include_directories(
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

add_executable(pythonparsertest
  src/pythontest.cpp
  src/pythonparsertest.cpp)

target_link_libraries(pythonparsertest
  ${GTEST_BOTH_LIBRARIES}
  pthread
  util
  model
  pythonmodel)

# Clean up the build folder and the output of the test in a `make clean`.
set_property(DIRECTORY APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/workdir")

# CTest runs the `pythonparsertest` binary with two arguments:
#   1. Shell script which parses the Python project
#   2. Test database connection string
add_test(NAME pythonparser COMMAND pythonparsertest
    "echo \"Test database used: ${TEST_DB}\" && \
    ${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
       --database \"${TEST_DB}\" \
       --name pythonparsertest \
       --input ${CMAKE_CURRENT_SOURCE_DIR}/sources/ \
       --workspace ${CMAKE_CURRENT_BINARY_DIR}/workdir/ \
       --force"
    "${TEST_DB}")
