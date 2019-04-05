# Generate ODB files from sources
# @return ODB_CXX_SOURCES - odb cxx source files
function(generate_odb_files _src)
  foreach(_file ${_src})
    get_filename_component(_dir ${_file} DIRECTORY)
    get_filename_component(_name ${_file} NAME)

    string(REPLACE ".h" "-odb.cxx" _cxx ${_name})
    string(REPLACE ".h" "-odb.hxx" _hxx ${_name})
    string(REPLACE ".h" "-odb.ixx" _ixx ${_name})
    string(REPLACE ".h" "-odb.sql" _sql ${_name})

    add_custom_command(
      OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/${_cxx}
        ${CMAKE_CURRENT_BINARY_DIR}/${_hxx}
        ${CMAKE_CURRENT_BINARY_DIR}/${_ixx}
        ${CMAKE_CURRENT_BINARY_DIR}/${_sql}
      COMMAND
        ${ODB_EXECUTABLE} ${ODBFLAGS}
          -o ${CMAKE_CURRENT_BINARY_DIR}
          -I ${CMAKE_CURRENT_SOURCE_DIR}/include
          -I ${CMAKE_SOURCE_DIR}/model/include
          -I ${CMAKE_SOURCE_DIR}/util/include
          -I ${ODB_INCLUDE_DIRS}
          ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
      DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
      COMMENT "Generating ODB for ${_file}")

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
    ${CMAKE_SOURCE_DIR}/model/include/model
    ${CMAKE_SOURCE_DIR}/util/include
    ${CMAKE_BINARY_DIR})
endfunction(add_odb_library)

# add new odb static library for a plugin
function(add_odb_library_plugin _name _plugin)
  add_odb_library(${_name} ${ARGN})
  target_include_directories(${_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/plugins/${_plugin}/model/include
    ${CMAKE_SOURCE_DIR}/plugins/${_plugin}/model/include/model
    ${CMAKE_BINARY_DIR}/plugins/${_plugin})
endfunction(add_odb_library_plugin)

# This function can be used to install the ODB generated .sql files to a
# specific directory. These files will be used to create database tables before
# the parsing session.
# @param _dir The model directory under which the .sql files are located.
function(install_sql _dir)
  file(GLOB SQL_FILES "${_dir}/*.sql")
  install(
    FILES ${SQL_FILES}
    DESTINATION ${INSTALL_SQL_DIR})
endfunction(install_sql)

# This function can be used to install the thrift generated .js files to a
# specific directory. These files will be used at the gui
# @param _dir The gen-js directory under which the .js files are located.
function(install_js_thrift _dir)
  install(
    DIRECTORY ${_dir}
    DESTINATION ${INSTALL_GEN_DIR}
    FILES_MATCHING PATTERN "*.js")
endfunction(install_js_thrift)

# Install plugins webgui
# @parameter _dir - webgui directory of the plugin
function(install_webplugin _dir)
  # Copy javascript modules
  file(GLOB _js "${_dir}/js/[A-Z]*.js")
  install(FILES ${_js} DESTINATION "${INSTALL_SCRIPTS_DIR}/view/component" )

  # Copy javascript plugins
  file(GLOB _js "${_dir}/js/[^A-Z]*.js")
  install(FILES ${_js} DESTINATION "${INSTALL_SCRIPTS_DIR}/view" )

  # Copy css files
  file(GLOB _css "${_dir}/css/*.css")
  install(FILES ${_css} DESTINATION "${INSTALL_WEBROOT_DIR}/style" )

  # Copy images
  file(GLOB _images "${_dir}/images/*.jpg" "${_dir}/images/*.png")
  install(FILES ${_images} DESTINATION "${INSTALL_WEBROOT_DIR}/images" )

  # Collect userguides
  file(GLOB _userguides "${_dir}/userguide/*.md")
  set_property(GLOBAL APPEND PROPERTY USERGUIDES "${_userguides}")
endfunction(install_webplugin)

# Finds the absolute paths for the given Boost libraries
# Use variable arguments for the Boost libraries to link
function(find_boost_libraries)
  foreach(_lib ${ARGV})
    foreach(_path ${Boost_LIBRARIES})
      if(_path MATCHES ".*boost_${_lib}\.so$")
        list(APPEND LIBS ${_path})
      endif(_path MATCHES ".*boost_${_lib}\.so$")
    endforeach(_path)
  endforeach(_lib)

  set(Boost_LINK_LIBRARIES ${LIBS} PARENT_SCOPE)
endfunction(find_boost_libraries)

# Prints a coloured, and optionally bold message to the console.
# _colour should be some ANSI colour name, like "blue" or "magenta".
function(fancy_message _str _colour _isBold)
  set(BOLD_TAG "")
  set(COLOUR_TAG "")

  if (_isBold)
    set(BOLD_TAG "--bold")
  endif()

  if (NOT (_colour STREQUAL ""))
    set(COLOUR_TAG "--${_colour}")
  endif()

  execute_process(COMMAND
    ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1
    ${CMAKE_COMMAND} -E cmake_echo_color ${COLOUR_TAG} ${BOLD_TAG} ${_str})
endfunction(fancy_message)
