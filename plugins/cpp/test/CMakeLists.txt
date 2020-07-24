include_directories(
  ${PLUGIN_DIR}/model/include
  ${PLUGIN_DIR}/service/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_executable(cppservicetest
  src/cpptest.cpp
  src/servicehelper.cpp
  src/cpppropertiesservicetest.cpp
  src/cppreferenceservicetest.cpp)

add_executable(cppparsertest
  src/cpptest.cpp
  src/cppparsertest.cpp)

target_compile_options(cppservicetest PUBLIC -Wno-unknown-pragmas)
target_compile_options(cppparsertest PUBLIC -Wno-unknown-pragmas)

target_link_libraries(cppservicetest
  util
  model
  cppmodel
  cppservice
  ${Boost_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  pthread)

target_link_libraries(cppparsertest
  util
  model
  cppmodel
  ${Boost_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  pthread)

if (NOT FUNCTIONAL_TESTING_ENABLED)
  fancy_message("Skipping generation of test project cpptest." "yellow" TRUE)
else()
  # Clean up the build folder and the output of the test in a `make clean`.
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
      "${CMAKE_CURRENT_BINARY_DIR}/build"
      "${CMAKE_CURRENT_BINARY_DIR}/workdir")

  # Add test to the project to run by ctest.
  #
  # There are three arguments that are passed to the `cppservicetest` binary
  # here.
  #   The first sets up the build environment and builds the project.
  #   The second executes the parsing.
  #   Then we put the database string which will be used by the tests.
  add_test(NAME cppservice COMMAND cppservicetest
    "echo \"Test database used: ${TEST_DB}\" && \
       rm -rf ${CMAKE_CURRENT_BINARY_DIR}/build && \
       mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cd ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cmake ${CMAKE_CURRENT_SOURCE_DIR}/sources/service \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=on"
    "${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
       --database \"${TEST_DB}\" \
       --name cppservicetest \
       --input ${CMAKE_CURRENT_BINARY_DIR}/build/compile_commands.json \
       --workspace ${CMAKE_CURRENT_BINARY_DIR}/workdir/ \
       --force"
    "${TEST_DB}")

  add_test(NAME cppparser COMMAND cppparsertest
    "echo \"Test database used: ${TEST_DB}\" && \
       rm -rf ${CMAKE_CURRENT_BINARY_DIR}/build && \
       mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cd ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cmake ${CMAKE_CURRENT_SOURCE_DIR}/sources/parser \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=on"
    "${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
       --database \"${TEST_DB}\" \
       --name cppparsertest \
       --input ${CMAKE_CURRENT_BINARY_DIR}/build/compile_commands.json \
       --workspace ${CMAKE_CURRENT_BINARY_DIR}/workdir/ \
       --force"
    "${TEST_DB}")

  fancy_message("Generating test project for cppservicetest." "blue" TRUE)
endif()
