# Install shared objects to $CMAKE_INSTALL_PREFIX/lib/deps/
if(INSTALL_RUNTIME_DEPENDENCIES OR ENABLE_PACKAGING)
  install(TARGETS CodeCompass_parser CodeCompass_webserver RUNTIME_DEPENDENCIES LIBRARY DESTINATION "${INSTALL_LIB_DIR}/${INSTALL_DEPS_DIR_NAME}")
endif()

if(NOT ENABLE_PACKAGING)
  return()
endif()

if(NOT PACKAGE_FILE_NAME)
  set(PACKAGE_FILE_NAME "codecompass-dev")
endif()

if(NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION "1.0")
endif()

set(PACKAGE_DIR "${PACKAGE_FILE_NAME}")

add_custom_target(
  deb_package

  COMMAND mkdir -p ${PACKAGE_DIR}/DEBIAN
  COMMAND mkdir -p ${PACKAGE_DIR}/usr/share/codecompass

  COMMAND echo "Adding CodeCompass installation ..."
  COMMAND cp -r ${CMAKE_INSTALL_PREFIX}/* ${PACKAGE_DIR}/usr/share/codecompass/

  COMMAND echo "Removing webgui-new sources ..."
  COMMAND rm -rf ${PACKAGE_DIR}/usr/share/codecompass/share/codecompass/webgui-new/app/

  COMMAND cp ${CMAKE_SOURCE_DIR}/packaging/deb/postinst ${PACKAGE_DIR}/DEBIAN/postinst
  COMMAND chmod +x ${PACKAGE_DIR}/DEBIAN/postinst

  COMMAND echo "---------- postinst file ----------"
  COMMAND cat ${PACKAGE_DIR}/DEBIAN/postinst
  COMMAND echo "-----------------------------------"

  COMMAND cp ${CMAKE_SOURCE_DIR}/packaging/deb/control ${PACKAGE_DIR}/DEBIAN/control
  COMMAND sed -i "s/\%PACKAGE_VERSION\%/${PACKAGE_VERSION}/" ${PACKAGE_DIR}/DEBIAN/control
  COMMAND sed -i "s/\%PACKAGE_DEPENDS\%/${PACKAGE_DEPENDS}/" ${PACKAGE_DIR}/DEBIAN/control

  COMMAND echo "---------- Manifest file ----------"
  COMMAND cat ${PACKAGE_DIR}/DEBIAN/control
  COMMAND echo "-----------------------------------"

  COMMAND echo "Building package ${PACKAGE_FILE_NAME} ..."
  COMMAND dpkg-deb --build ${PACKAGE_DIR}
  COMMAND echo "Done!"
)
