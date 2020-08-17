add_subdirectory(userguide)

install(DIRECTORY
  images scripts style fonts
  DESTINATION ${INSTALL_WEBROOT_DIR}
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "[^.]*")

install(FILES
  login.html
  index.html
  startpage.html
  credits.html
  DESTINATION ${INSTALL_WEBROOT_DIR})

# Install npm packages
install(CODE "set(CC_PACKAGE \"${CMAKE_CURRENT_SOURCE_DIR}/package.json\")")
install(CODE "set(CC_PROFILE \"${CMAKE_CURRENT_SOURCE_DIR}/codecompass.profile.js.in\")")
install(CODE "set(INSTALL_SCRIPTS_DIR \"${INSTALL_WEBROOT_DIR}/scripts/\")")
install(CODE "set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})")
install(SCRIPT InstallGUI.cmake)