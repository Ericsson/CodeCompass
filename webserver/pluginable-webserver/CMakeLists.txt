include_directories(
  include
  ${PROJECT_SOURCE_DIR}/webserver/mongoose/include
  ${PROJECT_SOURCE_DIR}/webserver/pluginframework/include
  ${PROJECT_SOURCE_DIR}/webserver/uscs-api
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${LLVM_INCLUDE_DIRS}
  
)

set(SOURCES
  src/mongoose_main.cpp
  src/threadedmongoose.cpp
  src/mongoose_utility.cpp
  src/authentication.cpp
  src/main_request_handler.cpp
)

add_executable(CodeCompass_webserver
  ${SOURCES}
)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(CodeCompass_webserver
  util
  mongoose
  model
  ${llvm_libs} 
  boost_thread
  boost_filesystem
  boost_system
  boost_program_options
  boost_log
  ldap
)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
