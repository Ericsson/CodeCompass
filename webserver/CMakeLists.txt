configure_file(
  src/config.h.in
  ${PROJECT_SOURCE_DIR}/webserver/src/config.h)

add_executable(CodeCompass_webserver
  src/webserver.cpp
  src/mainrequesthandler.cpp
  src/threadedmongoose.cpp
  src/mongoose.c)

target_include_directories(CodeCompass_webserver PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

target_link_libraries(CodeCompass_webserver
  util
  boost_thread
  boost_filesystem
  boost_system
  boost_program_options
  boost_log
  ${ODB_LIBRARIES}
  pthread
  dl)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
