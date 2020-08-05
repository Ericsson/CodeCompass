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
  ${PROJECT_SOURCE_DIR}/util/include)

target_link_libraries(CodeCompass_webserver
  util
  mongoose
  ${Boost_LIBRARIES}
  ${ODB_LIBRARIES}
  pthread
  dl)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
