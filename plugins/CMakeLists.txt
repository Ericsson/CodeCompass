# Common things
include_directories(SYSTEM
  ${ODB_INCLUDE_DIRS})

# Add all subdirectories to the build
file(GLOB plugins RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*")

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
