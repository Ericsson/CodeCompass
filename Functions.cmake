# Map the given (any argument long) model source directories to include paths
# that properly span the source and the build folder, ensuring that
# model-related headers can be found properly.
# @param _directories - The list of "include" directories, either relative or
# absolute, which contain "model/entity.h" files.
# @return ODB_MODEL_INCLUDE_DIRECTORIES - The include directories that were mapped.
function(map_model_include_directories)
  foreach(_dir ${ARGV})
    get_filename_component(absolute "${_dir}" ABSOLUTE)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}"
      binary_dir "${absolute}")

    list(APPEND include_dirs
      "${absolute}"
      "${absolute}/model"
      "${binary_dir}"
      "${binary_dir}/model")
  endforeach()
  set(ODB_MODEL_INCLUDE_DIRECTORIES "${include_dirs}" PARENT_SCOPE)
endfunction(map_model_include_directories)

# Generate ODB files from sources entity definitions.
# @return ODB_CXX_SOURCES - ODB-generated translation unit source files.
function(generate_odb_files _src)
  foreach(_file ${_src})
    get_filename_component(_dir ${_file} DIRECTORY)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_dir})

    string(REPLACE ".h" "-odb.cxx" _cxx ${_file})
    string(REPLACE ".h" "-odb.hxx" _hxx ${_file})
    string(REPLACE ".h" "-odb.ixx" _ixx ${_file})
    string(REPLACE ".h" "-odb.sql" _sql ${_file})

    add_custom_command(
      OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/${_cxx}
        ${CMAKE_CURRENT_BINARY_DIR}/${_hxx}
        ${CMAKE_CURRENT_BINARY_DIR}/${_ixx}
        ${CMAKE_CURRENT_BINARY_DIR}/${_sql}
      COMMAND
        ${ODB_EXECUTABLE} ${ODBFLAGS}
          -o ${CMAKE_CURRENT_BINARY_DIR}/${_dir}
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

# Add a new static library target that links against ODB.
function(add_odb_library _name)
  map_model_include_directories(${CMAKE_SOURCE_DIR}/model/include)
  include_directories(SYSTEM ${ODB_INCLUDE_DIRS})

  # Make sure the library created here depends on the ODB libs.
  set_rpath_with_libraries(ODB)

  add_library(${_name} STATIC ${ARGN})
  target_compile_options(${_name} PUBLIC -Wno-unknown-pragmas -fPIC)
  target_link_libraries(${_name} ${ODB_LIBRARIES})
  target_include_directories(${_name} PUBLIC
    ${ODB_MODEL_INCLUDE_DIRECTORIES}
    ${CMAKE_SOURCE_DIR}/util/include)
endfunction(add_odb_library)

# This function can be used to install the ODB generated .sql files from the
# build directory to the install directory. These files will be used to create
# the database schema before the parsing session.
# @param _dir The model directory where the entities are defined.
function(install_sql _dir)
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_dir}
    DESTINATION ${INSTALL_SQL_DIR}
    FILES_MATCHING PATTERN "*.sql")
endfunction(install_sql)

# Add a new static library target that generates Thrift RPC stubs and links the
# generated code against Thrift.
# @param _name - The name of the Thrift file (relative to the current source
#                folder) to build.
# @param _generators - The semicolon (;) separated list of Thrift generators to
#                      fire.
# @param _dependencies - List of semicolon(;) separated Thrift library names
#                        (e.g. "common") on which the current library depends.
function(add_thrift_library _name _generators _dependencies)
  foreach(dependency_lib IN LISTS _dependencies)
    if (NOT ${dependency_lib}thrift_SOURCE_DIR
        OR NOT ${dependency_lib}thrift_BINARY_DIR)
      message(SEND_ERROR "Usage of Thrift library ${dependency_lib} as a       \
                          dependency for ${_name}, but this dependency has not \
                          been generated yet.")
      return()
    endif()

    list(APPEND INCLUDE_ARGS "-I" "${${dependency_lib}thrift_SOURCE_DIR}/..")
  endforeach()

  define_property(DIRECTORY
    PROPERTY THRIFT_CONFIGURED_GENERATORS
    BRIEF_DOCS "List of Thrift generators that has already been configured to \
                execute for the current source folder."
    FULL_DOCS  "List of Thrift generators that has already been configured to \
                execute for the current source folder.")

  foreach(generator IN LISTS _generators)
    list(APPEND GENERATOR_OPTS "--gen" "${generator}")
    get_property(known_targets DIRECTORY
      PROPERTY THRIFT_CONFIGURED_GENERATORS)

    if (${generator} STREQUAL "cpp")
      # C++ targets always contain a _constants and _types C++ class.
      list(APPEND OUTPUT_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/${_name}_constants.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/${_name}_constants.h"
        "${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/${_name}_types.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/${_name}_types.h")

      # C++ outputs also contain for each `service` declaration a ServiceName
      # class.
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.thrift thrift_contents)
      string(REGEX MATCHALL "service ([^\n{]+)" services "${thrift_contents}")
      foreach(service IN LISTS services)
        # The result of "REGEX MATCHALL" contains the full match, not just the
        # matched group, so it needs to be trimmed.
        string(REPLACE "service " "" service "${service}")
        string(STRIP "${service}" service)

        list(APPEND OUTPUT_FILES
          "${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/${service}.cpp"
          "${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/${service}.h")
      endforeach()

      # In case of C++, user code usually depends on the stub headers generated
      # by Thrift, so this needs to be added as an include path.
      include_directories(${CMAKE_CURRENT_BINARY_DIR}/gen-cpp)
      foreach(dependency_lib IN LISTS _dependencies)
        include_directories(${${dependency_lib}thrift_BINARY_DIR}/gen-cpp)
      endforeach()

      # Add the system headers and libraries to the current folder's scope
      # too, as compilation of Thrift stubs will depend on these.
      include_directories(SYSTEM ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})
      set_rpath_with_libraries(Thrift)

      # Create the target C++ library.
      foreach(out_file IN LISTS OUTPUT_FILES)
        if(out_file MATCHES "\\.cpp$")
          list(APPEND cpp_files "${out_file}")
        endif()
      endforeach()

      add_library(${_name}thrift STATIC
        ${cpp_files})
      target_compile_options(${_name}thrift PUBLIC -fPIC)

      foreach(dependency_lib IN LISTS _dependencies)
        add_dependencies(${_name}thrift ${dependency_lib}thrift)
      endforeach()
    elseif(${generator} STREQUAL "js" OR ${generator} STREQUAL "js:jquery")
      # The JavaScript dependencies need not be listed individually as they
      # are only copied to the output folder anyways.
      # The folder is marked as an output so `make clean` removes it properly.
      install_js_thrift("${CMAKE_CURRENT_BINARY_DIR}/gen-js")
    elseif(${generator} STREQUAL "java")
      # The same applies to the Java generator. No actions need to be taken
      # here for now, the targets use properly laid out dependencies before an
      # add_jar() call.
    else()
      message(FATAL_ERROR "Unknown generator '${generator}' supplied to      \
                           add_thrift_library. Please enhance the function's \
                           logic to know how and where this generator        \
                           generates.")
    endif()

    # CMakeLists.txt files can contain multiple calls to this function, with
    # different Thrift files to generate into the same structure. In case the
    # same generator is fired twice, the "gen-foo" folder should only be added
    # once as a target, otherwise an error will be presented.
    # (We want the error to be presented for files!)
    if (NOT known_targets MATCHES generator)
      list(APPEND OUTPUT_FILES "${CMAKE_CURRENT_BINARY_DIR}/gen-${generator}")
    endif()
    set_property(DIRECTORY
      APPEND
      PROPERTY THRIFT_CONFIGURED_GENERATORS
      "${generator}")
  endforeach()

  add_custom_command(
    OUTPUT
      ${OUTPUT_FILES}
    COMMAND
      ${THRIFT_EXECUTABLE} ${GENERATOR_OPTS}
        -o ${CMAKE_CURRENT_BINARY_DIR}
        ${INCLUDE_ARGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.thrift
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.thrift
    COMMENT
      "Generating Thrift for ${_name}.thrift")

  set(${_name}thrift_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    CACHE INTERNAL "The source folder for the ${_name}thrift's definition.")
  mark_as_advanced(${_name}thrift_SOURCE_DIR)
  set(${_name}thrift_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CACHE INTERNAL "The folder for the ${_name}thrift's generated code.")
  mark_as_advanced(${_name}thrift_BINARY_DIR)
endfunction(add_thrift_library)

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
