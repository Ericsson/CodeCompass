install(DIRECTORY
  images scripts style fonts
  DESTINATION ${INSTALL_WEBROOT_DIR}
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "[^.]*")

install(FILES
  index.html
  startpage.html
  credits.html
  #PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE
  DESTINATION ${INSTALL_WEBROOT_DIR})

# Install npm packages
install(CODE "set(CC_PACKAGE \"${CMAKE_CURRENT_SOURCE_DIR}/package.json\")")
install(CODE "set(INSTALL_SCRIPTS_DIR \"${INSTALL_WEBROOT_DIR}/scripts/\")")
install(SCRIPT InstallGUI.cmake)