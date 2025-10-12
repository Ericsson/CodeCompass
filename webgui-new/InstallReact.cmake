# Utility functions
include(${CMAKE_SOURCE_DIR}/Functions.cmake)

message("Install npm packages...")

if(${CC_PACKAGE_LOCK} IS_NEWER_THAN ${INSTALL_WEBROOT_REACT_DIR}/app/package-lock.json)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy ${CC_PACKAGE} ${INSTALL_WEBROOT_REACT_DIR}/app/package.json
    COMMAND ${CMAKE_COMMAND} -E copy ${CC_PACKAGE_LOCK} ${INSTALL_WEBROOT_REACT_DIR}/app/package-lock.json
    WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR})

  execute_process(
    COMMAND npm install
    WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)
endif()

message("Installation of npm packages are finished.")

message("Generating TypeScript files from Thrift...")

# Generate TypeScript from Thrift
execute_process(
  COMMAND bash -c "chmod +x thrift-codegen.sh"
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

execute_process(
  COMMAND bash thrift-codegen.sh --thrift-source ${CMAKE_SOURCE_DIR}
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

message("Generating TypeScript files from Thrift finished.")

message("Building React App...")

execute_process(
  COMMAND npm run build
  WORKING_DIRECTORY ${INSTALL_WEBROOT_REACT_DIR}/app)

# Move build directory out of application directory
file(REMOVE_RECURSE ${INSTALL_WEBROOT_REACT_DIR}/out)
file(RENAME ${INSTALL_WEBROOT_REACT_DIR}/app/out ${INSTALL_WEBROOT_REACT_DIR}/out)

# Create symbolic link
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ../webgui-new/out ${INSTALL_WEBROOT_DIR}/new)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  # Remove application source code in case of Release build
  file(REMOVE ${INSTALL_WEBROOT_REACT_DIR}/app)
endif()

message("Building React App finished.")
