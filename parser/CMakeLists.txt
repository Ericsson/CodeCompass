include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${LLVM_INCLUDE_DIRS}
)

set(SOURCES
  src/plugin_handler.cpp
  src/sourcemanager.cpp
  src/parser.cpp
)

add_executable(CodeCompass_parser
  ${SOURCES}
)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(CodeCompass_parser
  util
  model
  ${llvm_libs} 
  boost_thread
  boost_filesystem
  boost_system
  boost_program_options
  boost_log
  magic
  ${ODB_LIBRARIES}    
  ${CMAKE_DL_LIBS}
)

install(TARGETS CodeCompass_parser
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})