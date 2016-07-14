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

# Generate thrift source files
# @parameter _thriftFiles - List of thrift files
# @parameter _languages   - List of languages
# @parameter _dependeny   - Dependency target
function(generate_thrift_files _thriftFiles _languages _dependeny)
  if(NOT TARGET ${_dependeny})
    message(ERROR "${_dependency} is not a target!")
  endif()
  foreach(_language ${_languages})
    foreach(_thriftFile ${_thriftFiles})
      get_filename_component(_thriftFileName ${_thriftFile} NAME_WE)
      add_custom_command(
        OUTPUT
          ${CMAKE_CURRENT_SOURCE_DIR}/gen-${_language}
        COMMAND
          thrift --gen ${_language} -o ${CMAKE_CURRENT_SOURCE_DIR} ${_thriftFile}
        DEPENDS
          ${_thriftFile}
        COMMENT "Generate thrift for ${_thriftFile}")

      set(_thriftTarget generate_${_thriftFileName}_${_language}_thrift_files)
      add_custom_target(${_thriftTarget}
        DEPENDS
          ${CMAKE_CURRENT_SOURCE_DIR}/gen-${_language})
      add_dependencies(${_dependeny} ${_thriftTarget})
    endforeach()
  endforeach()
endfunction(generate_thrift_files)