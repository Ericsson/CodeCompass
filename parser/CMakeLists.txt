include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${LLVM_INCLUDE_DIRS}
  ${ODB_INCLUDE_DIRS})

add_executable(CodeCompass_parser
  src/pluginhandler.cpp
  src/sourcemanager.cpp
  src/parser.cpp)

target_link_libraries(CodeCompass_parser
  util
  model
  boost_thread
  boost_filesystem
  boost_system
  boost_program_options
  boost_log
  ${ODB_LIBRARIES}    
  ${CMAKE_DL_LIBS}
  magic
  pthread)

install(TARGETS CodeCompass_parser
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
