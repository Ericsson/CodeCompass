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

add_library(compilationdatabasemodel STATIC ${ODB_CXX_SOURCES})

target_compile_options(compilationdatabasemodel PUBLIC -Wno-unknown-pragmas)
target_link_libraries(compilationdatabasemodel
  ${ODB_LIBRARIES}    
)