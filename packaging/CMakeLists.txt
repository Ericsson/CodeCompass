# Install shared objects to $CMAKE_INSTALL_PREFIX/lib/deps/
if(INSTALL_RUNTIME_DEPENDENCIES OR ENABLE_PACKAGING)
  install(TARGETS CodeCompass_parser CodeCompass_webserver util gitservice cppparser RUNTIME_DEPENDENCIES LIBRARY DESTINATION "${INSTALL_LIB_DIR}/${INSTALL_DEPS_DIR_NAME}")
endif()

if(NOT ENABLE_PACKAGING)
  return()
endif()

if(NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION "1.0")
endif()

set(PACKAGE_DIR "codecompass")

add_custom_target(
  appimage 

  COMMAND mkdir -p ${PACKAGE_DIR}/usr/

  COMMAND echo "Adding CodeCompass installation: ${CMAKE_INSTALL_PREFIX}"
  COMMAND cp -r ${CMAKE_INSTALL_PREFIX}/* ${PACKAGE_DIR}/usr/

  COMMAND echo "Removing webgui-new sources ..."
  COMMAND rm -rf ${PACKAGE_DIR}/usr/share/codecompass/webgui-new/app/

  COMMAND cp -r ${CMAKE_SOURCE_DIR}/packaging/appimage/* ${PACKAGE_DIR}
  COMMAND chmod +x ${PACKAGE_DIR}/AppRun

  COMMAND echo "---------- AppRun file ----------"
  COMMAND cat ${PACKAGE_DIR}/AppRun
  COMMAND echo "---------------------------------"

  COMMAND sed -i "s/\%PACKAGE_VERSION\%/${PACKAGE_VERSION}/" ${PACKAGE_DIR}/codecompass.desktop

  COMMAND echo "---------- Desktop file ----------"
  COMMAND cat ${PACKAGE_DIR}/codecompass.desktop
  COMMAND echo "----------------------------------"

  COMMAND echo "Building CodeCompass AppImage ..."
  COMMAND appimagetool-x86_64.AppImage ${PACKAGE_DIR}
  COMMAND echo "Done!"
)
