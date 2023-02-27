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

# Generate TypeScript from Thrift
execute_process(
  COMMAND
    npm run thrift-codegen-build
      ${CMAKE_SOURCE_DIR}/service/workspace/workspace.thrift
      ${CMAKE_SOURCE_DIR}/service/project/project.thrift
      ${CMAKE_SOURCE_DIR}/service/project/common.thrift
      ${CMAKE_SOURCE_DIR}/service/plugin/plugin.thrift
      ${CMAKE_SOURCE_DIR}/service/language/language.thrift
      ${CMAKE_SOURCE_DIR}/service/authentication/authentication.thrift
      ${CMAKE_SOURCE_DIR}/plugins/dummy/service/dummy.thrift
      ${CMAKE_SOURCE_DIR}/plugins/git/service/git.thrift
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

# Convert imports to default
execute_process(
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/remover.sh
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app/generated)

message("Building React App...")

execute_process(
  COMMAND npm run build
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

# Move build directory out of application directory
execute_process(
  COMMAND /bin/sh -c "rm -rf ../out && mv -f ./out ../"
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

# Create symbolic link
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${INSTALL_WEBROOT_REACT_DIR}/out ${INSTALL_WEBROOT_DIR}/new)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  # Remove application source code in case of Release build
  file(REMOVE ${INSTALL_WEBROOT_REACT_DIR}/app)
endif()

message("Building React App finished.")
