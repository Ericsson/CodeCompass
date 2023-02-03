# Utility functions
include(${CMAKE_SOURCE_DIR}/Functions.cmake)

message("Install npm packages...")

if(${CC_PACKAGE} IS_NEWER_THAN ${INSTALL_WEBROOT_REACT_DIR}/app/package.json)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy ${CC_PACKAGE} ${INSTALL_WEBROOT_REACT_DIR}/app/package.json
    WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR})

  execute_process(
    COMMAND npm install
    WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)
endif()

message("Installation of npm packages are finished.")

message("Building React App...")

execute_process(
  COMMAND npm run build
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

# Move build directory out of application directory
file(RENAME ${INSTALL_WEBROOT_REACT_DIR}/app/build ${INSTALL_WEBROOT_REACT_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  # Remove application source code in case of Release build
  file(REMOVE ${INSTALL_WEBROOT_REACT_DIR}/app)
endif()

message("Building React App finished.")
