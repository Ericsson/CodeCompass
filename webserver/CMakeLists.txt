include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_subdirectory(authenticators)

add_executable(CodeCompass_webserver
  src/webserver.cpp
  src/authentication.cpp
  src/mainrequesthandler.cpp
  src/session.cpp
  src/sessionmanager.cpp
  src/threadedmongoose.cpp)

set_target_properties(CodeCompass_webserver
  PROPERTIES ENABLE_EXPORTS 1)

add_library(mongoose STATIC src/mongoose.c )
target_compile_definitions(mongoose PRIVATE -DNS_ENABLE_SSL)
target_include_directories(mongoose PUBLIC include)
target_compile_options(mongoose PUBLIC -fPIC)
target_link_libraries(mongoose PRIVATE ssl)

target_include_directories(CodeCompass_webserver PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/service/lsp/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/service/include
  ${PROJECT_SOURCE_DIR}/plugins/cpp/model/include)

target_link_libraries(CodeCompass_webserver
  util
  lspservice
  cppservice
  mongoose
  ${Boost_LIBRARIES}
  ${ODB_LIBRARIES}
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  pthread
  dl)

add_dependencies(CodeCompass_webserver projectthrift)
add_dependencies(CodeCompass_webserver languagethrift)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
