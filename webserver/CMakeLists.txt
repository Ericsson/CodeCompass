set_rpath_with_libraries(ODB)

add_executable(CodeCompass_webserver
  src/webserver.cpp
  src/mainrequesthandler.cpp
  src/threadedmongoose.cpp )

set_target_properties(CodeCompass_webserver
  PROPERTIES ENABLE_EXPORTS 1)

add_library(mongoose STATIC src/mongoose.c )

target_include_directories(mongoose PUBLIC include)
target_compile_options(mongoose PUBLIC -fPIC)

target_include_directories(CodeCompass_webserver PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

find_boost_libraries(
  filesystem
  log
  program_options
  system
  thread)

target_link_libraries(CodeCompass_webserver
  util
  mongoose
  ${Boost_LINK_LIBRARIES}
  ${ODB_LIBRARIES}
  pthread
  dl)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
