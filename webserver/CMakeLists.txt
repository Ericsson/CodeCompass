add_executable(CodeCompass_webserver
  src/webserver.cpp
  src/mainrequesthandler.cpp
  src/threadedmongoose.cpp )

add_library(mongoose STATIC src/mongoose.c )

target_include_directories(mongoose PUBLIC include)
target_compile_options(mongoose PUBLIC -fPIC)

target_include_directories(CodeCompass_webserver PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

target_link_libraries(CodeCompass_webserver
  util
  mongoose
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
