set(ODB_SOURCES
        include/model/pythonastnode.h
        include/model/pythonclass.h
        include/model/pythondocumentation.h
        include/model/pythonentity.h
        include/model/pythonfunction.h
        include/model/pythonimport.h
        include/model/pythoninheritance.h
        include/model/pythontype.h
        include/model/pythonvariable.h)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(pythonmodel ${ODB_CXX_SOURCES})
target_link_libraries(pythonmodel model)

install_sql()
