install(DIRECTORY
        public src
        DESTINATION ${INSTALL_WEBROOT_REACT_DIR}/app
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "[^.]*")

install(FILES
        tsconfig.json
        next.config.js
        thrift-codegen.sh
        .env
        DESTINATION ${INSTALL_WEBROOT_REACT_DIR}/app)

# Install React application
install(CODE "set(CC_PACKAGE \"${CMAKE_CURRENT_SOURCE_DIR}/package.json\")")
install(CODE "set(INSTALL_WEBROOT_DIR ${INSTALL_WEBROOT_DIR})")
install(CODE "set(INSTALL_WEBROOT_REACT_DIR ${INSTALL_WEBROOT_REACT_DIR})")
install(CODE "set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})")
install(CODE "set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})")
install(SCRIPT InstallReact.cmake)
