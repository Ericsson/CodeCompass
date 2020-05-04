set(ODB_SOURCES
  include/model/buildaction.h
  include/model/buildlog.h
  include/model/buildsourcetarget.h
  include/model/filecontent.h
  include/model/file.h
  include/model/fileloc.h
  include/model/position.h
  include/model/statistics.h
  include/model/useremail.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(model ${ODB_CXX_SOURCES})

install_sql()
