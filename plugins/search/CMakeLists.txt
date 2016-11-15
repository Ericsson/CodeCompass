add_subdirectory(common)
add_subdirectory(indexer)
add_subdirectory(parser)
add_subdirectory(service)

install_webplugin(webgui)
install(DIRECTORY
  lib/java/
  DESTINATION "${INSTALL_JAVA_LIB_DIR}")
