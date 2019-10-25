include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})
  
add_executable(CodeCompass_webserver
  src/webserver.cpp
  src/mainrequesthandler.cpp
  src/threadedmongoose.cpp
  src/lsphandler.cpp)

set_target_properties(CodeCompass_webserver
  PROPERTIES ENABLE_EXPORTS 1)

add_library(mongoose STATIC src/mongoose.c )

target_include_directories(mongoose PUBLIC include)
target_compile_options(mongoose PUBLIC -fPIC)

target_include_directories(CodeCompass_webserver PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/service/lsp/include
  ${PROJECT_SOURCE_DIR}/service/language/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/service/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/model/include)

find_boost_libraries(
  filesystem
  log
  program_options
  system
  thread)
target_link_libraries(CodeCompass_webserver
  util
  lspservice
  cppservice
  mongoose
  ${Boost_LINK_LIBRARIES}
  ${ODB_LIBRARIES}
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  pthread
  dl)

add_dependencies(CodeCompass_webserver projectthrift)
add_dependencies(CodeCompass_webserver languagethrift)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
