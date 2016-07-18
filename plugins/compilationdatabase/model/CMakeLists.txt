include_directories(  
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${ODB_INCLUDE_DIRS}
)

set(ODB_SOURCES
  include/model/buildaction.h
  include/model/buildlog.h
  include/model/buildlogmessage.h
  include/model/buildsource.h
  include/model/buildtarget.h 
)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(compilationdatabasemodel ${ODB_CXX_SOURCES})
target_link_libraries(compilationdatabasemodel
  model
)

install(TARGETS compilationdatabasemodel
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_PARSER_DIR})