map_model_include_directories(
  ${PLUGIN_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/model/include)
include_directories(${ODB_MODEL_INCLUDE_DIRECTORIES})

include_directories(
  ${PLUGIN_DIR}/service/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
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

# Add test to the project to run by ctest
add_test(NAME allCppTest COMMAND cpptest
  "mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/sources/build &&\
  cd ${CMAKE_CURRENT_SOURCE_DIR}/sources/build &&\
  cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=on"
  "${CMAKE_INSTALL_PREFIX}/bin/keepalive ${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
  -d \"${TEST_DB}\"\
  -n cpptest \
  -i ${CMAKE_CURRENT_SOURCE_DIR}/sources/build/compile_commands.json \
  -w ${CMAKE_CURRENT_SOURCE_DIR}/workdir/ -f"
  "${TEST_DB}")
