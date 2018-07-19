#
# Fetch some of the environment information that is necessary to run CodeCompass
#

# Transforms the given list of files to a list of their directory names.
# The names are uniqued.
# @parameter _outputName - The variable where the output
# @parameter _fileList - The list of files to transform.
function(filelist_to_dirlist_unique _outputName _fileList)
  list(APPEND _dirs "")
  foreach(_file IN LISTS _fileList)
    get_filename_component(_dir "${_file}" DIRECTORY)
    list(APPEND _dirs "${_dir}")
  endforeach()
  list(REMOVE_DUPLICATES _dirs)
  set(${_outputName} "${_dirs}" PARENT_SCOPE)
endfunction(filelist_to_dirlist_unique)

# Adds the given path as an environment variable that is exported to the install
# targets.
# @parameter _library - The library or context for which the variable is set.
# @parameter _envVar - The variable to which the value should be exported.
# @parameter _value - The value to export.
function(mark_run_env_path _library _envVar _value)
  set(RUNENV_${_library}_${_envVar} "${_value}"
    CACHE
    INTERNAL
    "Environment variable of ${_library} exported to install."
    FORCE)
  mark_as_advanced(RUNENV_${_library}_${_envVar})
endfunction(mark_run_env_path)

# Retrieve the effective RPATH (for runtime library resolution). This includes
# some generic lookup rules, and the library paths that CMake found for the
# libs specified.
# @parameter _libs - The libraries whose library path should be. This is an arbitrary long argument
function(set_rpath_with_libraries _libs)
  # Always begin with the RPATH inherited to the scope where this function is
  # called.
  set(_temp "${CMAKE_INSTALL_RPATH}")
  foreach (_item ${ARGV})
    string(CONCAT _temp "${_temp};${RUNENV_${_item}_LD_LIBRARY_PATH}")
  endforeach()
  set(CMAKE_INSTALL_RPATH ${_temp} PARENT_SCOPE)
endfunction(set_rpath_with_libraries)

# Set the library path for CodeCompass plugin inter-dependencies.
mark_run_env_path(ParserPlugins LD_LIBRARY_PATH
  "../${INSTALL_LIB_DIR_NAME}/${INSTALL_PARSER_DIR_NAME};$ORIGIN/../${INSTALL_LIB_DIR_NAME}/${INSTALL_PARSER_DIR_NAME}")
mark_run_env_path(ServicePlugins LD_LIBRARY_PATH
  "../${INSTALL_LIB_DIR_NAME}/${INSTALL_SERVICE_DIR_NAME};$ORIGIN/../${INSTALL_LIB_DIR_NAME}/${INSTALL_SERVICE_DIR_NAME}")

# Set some common third-parties' library path.
# ODB
filelist_to_dirlist_unique(_odbLibDirs "${ODB_LIBRARIES}")
mark_run_env_path(ODB LD_LIBRARY_PATH "${_odbLibDirs}")

# Thrift
filelist_to_dirlist_unique(_thriftLibDirs "${THRIFT_LIBTHRIFT_LIBRARIES}")
mark_run_env_path(Thrift LD_LIBRARY_PATH "${_thriftLibDirs}")
