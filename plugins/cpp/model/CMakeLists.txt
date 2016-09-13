set(ODB_SOURCES
  include/model/cppastnode.h
  include/model/cppentity.h
  include/model/cppenum.h
  include/model/cppfriendship.h
  include/model/cppfunction.h
  include/model/cppinheritance.h
  include/model/cppnamespace.h
  include/model/cpprelation.h
  include/model/cpptypedef.h
  include/model/cpptype.h
  include/model/cppvariable.h
)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(cppmodel ${ODB_CXX_SOURCES})

install_sql(include/model/)
