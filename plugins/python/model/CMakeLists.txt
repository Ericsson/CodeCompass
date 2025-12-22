set(ODB_SOURCES
    include/model/pyname.h
)

generate_odb_files("${ODB_SOURCES}")

add_odb_library(pythonmodel ${ODB_CXX_SOURCES})
target_link_libraries(pythonmodel model)

install_sql()
