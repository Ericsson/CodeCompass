# Common things
include_directories(SYSTEM
  ${ODB_INCLUDE_DIRS})

if(WITH_PLUGIN AND WITHOUT_PLUGIN)
  message(FATAL_ERROR "The WITH_PLUGIN and WITHOUT_PLUGIN options cannot be used together.")
endif()

# Add all subdirectories to the build
file(GLOB plugins RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*")

# Remove subdirectories according to the provided WITH_PLUGIN or WITHOUT_PLUGIN variables
if(WITH_PLUGIN)
  if(NOT ("metrics" IN_LIST WITH_PLUGIN)
      OR NOT ("search" IN_LIST WITH_PLUGIN))
    message(WARNING "The search and metrics plugins cannot be left out, "
            "they will be compiled anyway.")
  endif()
  foreach(_plugin ${plugins})
    if(NOT ("${_plugin}" IN_LIST WITH_PLUGIN)
       AND NOT ("${_plugin}" STREQUAL "metrics")
       AND NOT ("${_plugin}" STREQUAL "search")
       AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin}")
      list(REMOVE_ITEM plugins ${_plugin})
    endif()
  endforeach()
elseif(WITHOUT_PLUGIN)
  if("metrics" IN_LIST WITHOUT_PLUGIN
     OR "search" IN_LIST WITHOUT_PLUGIN)
    message(WARNING "The search and metrics plugins cannot be left out, "
            "they will be compiled anyway.")
  endif()
  foreach(_plugin ${plugins})
    if(("${_plugin}" IN_LIST WITHOUT_PLUGIN)
       AND NOT ("${_plugin}" STREQUAL "metrics")
       AND NOT ("${_plugin}" STREQUAL "search")
       AND (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin}"))
      list(REMOVE_ITEM plugins ${_plugin})
    endif()
  endforeach()
endif()

# Set unique plugin directory variable for each plugin
message(STATUS "Found the following CodeCompass plugins:")
foreach(_plugin ${plugins})
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin}")
    set(${_plugin}_PLUGIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin}")
    set(${_plugin}_PLUGIN_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${_plugin}")
    set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/${_plugin}")
    message(STATUS "  ${_plugin}")
  endif()
endforeach(_plugin)

# Add plugins
foreach(_plugin ${plugins})
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin}")
    set(PLUGIN_NAME ${_plugin})
    set(PLUGIN_DIR ${${_plugin}_PLUGIN_DIR})
    set(PLUGIN_BINARY_DIR ${${_plugin}_PLUGIN_BINARY_DIR})
    fancy_message("Loading plugin ${_plugin}" "blue" TRUE)
    add_subdirectory(${_plugin})
  endif()
endforeach(_plugin)
