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

# Adds the given path to one of the paths that are exported to the install
# location into an environment script.
# @parameter _envVar - The variable to which the value should be exported.
# @parameter _value - The value to export.
function(mark_as_run_env_path _envVar _value)
  if (NOT RUNENV_${_envVar})
    set(_envList "${_value}")
  else()
    set(_envList "${RUNENV_${_envVar}}")
    list(APPEND _envList ${_value})
  endif()

  set(RUNENV_${_envVar} "${_envList}"
    CACHE INTERNAL "Environment variable exported to install." FORCE)
  mark_as_advanced(RUNENV_${_envVar})
endfunction(mark_as_run_env_path)

# Reset environment variables that were cached.
set(RUNENV_LD_LIBRARY_PATH ""
  CACHE INTERNAL "Environment variable exported to install." FORCE)

# LLVM
mark_as_run_env_path(LD_LIBRARY_PATH "${LLVM_BUILD_LIBRARY_DIR}")

# ODB
filelist_to_dirlist_unique(_odbLibDirs "${ODB_LIBRARIES}")
mark_as_run_env_path(LD_LIBRARY_PATH "${_odbLibDirs}")

# Thrift
filelist_to_dirlist_unique(_thriftLibDirs "${THRIFT_LIBTHRIFT_LIBRARIES}")
mark_as_run_env_path(LD_LIBRARY_PATH "${_thriftLibDirs}")
