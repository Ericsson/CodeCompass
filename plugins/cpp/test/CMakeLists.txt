include_directories(
  ${PLUGIN_DIR}/model/include
  ${PLUGIN_DIR}/service/include
  ${PROJECT_SOURCE_DIR}/service/language/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_executable(cpptest
  src/cppservicetest.cpp
  src/servicehelper.cpp
  src/cpppropertiesservicetest.cpp
  src/cppreferenceservicetest.cpp)

target_compile_options(cpptest PUBLIC -Wno-unknown-pragmas)

find_boost_libraries(
  filesystem
  log
  program_options
  system)

target_link_libraries(cpptest
  util
  cppservice
  ${Boost_LINK_LIBRARIES}
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
  # There are three arguments that are passed to the `cpptest` binary here.
  #   The first sets up the build environment and builds the project.
  #   The second executes the parsing.
  #   Then we put the database string which will be used by the tests.
  add_test(NAME cpp COMMAND cpptest
    "echo \"Test database used: ${TEST_DB}\" && \
       mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cd ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cmake ${CMAKE_CURRENT_SOURCE_DIR}/sources \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=on"
    "${CMAKE_INSTALL_PREFIX}/bin/keepalive \
       ${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
       --database \"${TEST_DB}\" \
       --name cpptest \
       --input ${CMAKE_CURRENT_BINARY_DIR}/build/compile_commands.json \
       --workspace ${CMAKE_CURRENT_BINARY_DIR}/workdir/ \
       --force"
    "${TEST_DB}")
  fancy_message("Generating test project for cpptest." "blue" TRUE)
endif()
