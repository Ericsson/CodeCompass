# Utility functions
include(../Functions.cmake)

message("Install npm packages...")

if(${CC_PACKAGE} IS_NEWER_THAN ${INSTALL_SCRIPTS_DIR}/package.json)
  execute_process(
    COMMAND cp ${CC_PACKAGE} ${INSTALL_SCRIPTS_DIR}/package.json
    WORKING_DIRECTORY ${INSTALL_SCRIPTS_DIR})

  execute_process(
    COMMAND npm install
    WORKING_DIRECTORY ${INSTALL_SCRIPTS_DIR})
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink dojo-util node_modules/util
  WORKING_DIRECTORY ${INSTALL_SCRIPTS_DIR})

message("Installation of npm packages are finished.")

message("Initiating the Dojo Build System...")

# Collect CodeCompass view JavaScript files
set(DOJO_VIEWLIST "")
file(GLOB _jsfiles RELATIVE "${INSTALL_SCRIPTS_DIR}" "${INSTALL_SCRIPTS_DIR}/codecompass/view/[^A-Z]*.js")
foreach(_jsfile ${_jsfiles})
  string(REGEX REPLACE "\\.js*$" "" _jsfile ${_jsfile})
  list(APPEND DOJO_VIEWLIST "'${_jsfile}'")
endforeach()
join("${DOJO_VIEWLIST}"  ", " DOJO_VIEWLIST)

set(DOJO_OPTIMIZE "")       # optimization for non-layer modules
set(DOJO_LAYEROPTIMIZE "")  # optimization for layer modules
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(DOJO_LAYEROPTIMIZE "closure")
endif()
configure_file(${CC_PROFILE} ${INSTALL_SCRIPTS_DIR}/codecompass.profile.js)

execute_process(
  COMMAND node_modules/util/buildscripts/build.sh --profile codecompass.profile.js
  WORKING_DIRECTORY ${INSTALL_SCRIPTS_DIR})

# The build-report may leak sensitive information
file(REMOVE ${INSTALL_SCRIPTS_DIR}/release/build-report.txt)

message("The Dojo Build System finished.")
