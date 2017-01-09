add_subdirectory(userguide)

install(DIRECTORY
  images scripts style fonts
  DESTINATION ${INSTALL_WEBROOT_DIR}
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "[^.]*")

install(FILES
  index.html
  startpage.html
  credits.html
  DESTINATION ${INSTALL_WEBROOT_DIR})

# Install npm packages
install(CODE "set(CC_PACKAGE \"${CMAKE_CURRENT_SOURCE_DIR}/package.json\")")
install(CODE "set(INSTALL_SCRIPTS_DIR \"${INSTALL_WEBROOT_DIR}/scripts/\")")
install(SCRIPT InstallGUI.cmake)