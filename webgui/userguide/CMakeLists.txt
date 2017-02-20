install(FILES
  header.html
  footer.html
  Doxyfile
  DESTINATION ${INSTALL_WEBROOT_DIR}/userguide)

install(CODE "set(INSTALL_USERGUIDE_DIR \"${INSTALL_WEBROOT_DIR}/userguide\")")
get_property(_userguides GLOBAL PROPERTY USERGUIDES)
install(CODE "set(USERGUIDES \"${_userguides}\")")
install(SCRIPT InstallUserguide.cmake)

install_webplugin(webgui)
