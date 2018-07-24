set_rpath_with_libraries(ODB)

map_model_include_directories(${PROJECT_SOURCE_DIR}/model/include)


include_directories(SYSTEM
  ${ODB_INCLUDE_DIRS})

add_executable(CodeCompass_parser
  src/pluginhandler.cpp
  src/sourcemanager.cpp
  src/parser.cpp)

target_include_directories(CodeCompass_parser PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${ODB_MODEL_INCLUDE_DIRECTORIES})

set_target_properties(CodeCompass_parser
  PROPERTIES ENABLE_EXPORTS 1)

find_boost_libraries(
  filesystem
  log
  program_options
  system
  thread)
target_link_libraries(CodeCompass_parser
  util
  model
  ${Boost_LINK_LIBRARIES}
  ${ODB_LIBRARIES}
  ${CMAKE_DL_LIBS}
  magic
  pthread)

install(TARGETS CodeCompass_parser
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
