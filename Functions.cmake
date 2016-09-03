# Generate ODB files from sources
# @return ODB_CXX_SOURCES - odb cxx source files
function(generate_odb_files _src)
  foreach(_file ${_src})
    get_filename_component(_dir ${_file} DIRECTORY)

    string(REPLACE ".h" "-odb.cxx" _cxx ${_file})
    string(REPLACE ".h" "-odb.hxx" _hxx ${_file})
    string(REPLACE ".h" "-odb.ixx" _ixx ${_file})
    string(REPLACE ".h" "-odb.sql" _sql ${_file})

    add_custom_command(
      OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/${_cxx}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_hxx}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_ixx}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_sql}
      COMMAND
        ${ODB} ${ODBFLAGS}
        -o ${CMAKE_CURRENT_SOURCE_DIR}/${_dir}
        -I ${CMAKE_CURRENT_SOURCE_DIR}/include
        -I ${CMAKE_SOURCE_DIR}/model/include
        -I ${CMAKE_SOURCE_DIR}/util/include
        ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
      DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
      COMMENT "Building odb for ${_file}")

    list(APPEND SOURCES ${_cxx})
  endforeach(_file)

  set(ODB_CXX_SOURCES ${SOURCES} PARENT_SCOPE)
endfunction(generate_odb_files)

# add new odb static library
function(add_odb_library _name)
  add_library(${_name} STATIC ${ARGN})
  target_compile_options(${_name} PUBLIC -Wno-unknown-pragmas -fPIC)
  target_link_libraries(${_name} ${ODB_LIBRARIES})
  target_include_directories(${_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/model/include
    ${CMAKE_SOURCE_DIR}/util/include)
endfunction(add_odb_library)

function(install_sql _dir)
  install(
    DIRECTORY ${_dir}
    DESTINATION share/codecompass/sql
    FILES_MATCHING PATTERN "*.sql")
endfunction(install_sql)
