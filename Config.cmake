# Installation directory for libraries
set(INSTALL_LIB_DIR_NAME "lib")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR_NAME}")

# Installation directory for parser libraries
set(INSTALL_PARSER_DIR_NAME "parserplugin")
set(INSTALL_PARSER_DIR "${INSTALL_LIB_DIR}/${INSTALL_PARSER_DIR_NAME}")

# Installation directory for service libraries
set(INSTALL_SERVICE_DIR_NAME "serviceplugin")
set(INSTALL_SERVICE_DIR "${INSTALL_LIB_DIR}/${INSTALL_SERVICE_DIR_NAME}")

# Installation directory of web GUI
set(INSTALL_WEBROOT_DIR "${CMAKE_INSTALL_PREFIX}/share/codecompass/webgui")

# Installation directory of SQL files
set(INSTALL_SQL_DIR "${CMAKE_INSTALL_PREFIX}/share/codecompass/sql")

# Global variable to collect userguides
define_property(GLOBAL PROPERTY USERGUIDES
  BRIEF_DOCS "Global list of userguides"
  FULL_DOCS  "Global list of userguides")
set_property(GLOBAL PROPERTY USERGUIDES
  "${CMAKE_CURRENT_SOURCE_DIR}/webgui/userguide/userguide.md")

# Installation directory of third-party JS scripts
set(INSTALL_WEBPLUGIN_DIR "${INSTALL_WEBROOT_DIR}/scripts/plugins")

# Installation directory of GUI scripts
set(INSTALL_SCRIPTS_DIR "${INSTALL_WEBROOT_DIR}/scripts/codecompass")

# Installation directory of generated Thrift API stubs
set(INSTALL_GEN_DIR "${INSTALL_SCRIPTS_DIR}/generated")

# Installation directory for Java libraries
set(INSTALL_JAVA_LIB_DIR "${INSTALL_LIB_DIR}/java")

# Installation directory for executables
set(INSTALL_BIN_DIR "bin")

set(DATABASE sqlite CACHE STRING "Database type")
string(TOUPPER ${DATABASE} DATABASE_U)

# Set up the dynamic libraries' runtime path to the install folder
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
string(CONCAT CMAKE_INSTALL_RPATH
  ".;$ORIGIN;..;$ORIGIN/.."
  ";../${INSTALL_LIB_DIR_NAME}"
  ";$ORIGIN/../${INSTALL_LIB_DIR_NAME}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Odb commands
set(ODB odb)
set(ODBFLAGS
  --std c++11
  --database ${DATABASE}
  --generate-query
  --generate-schema
  --schema-format sql
  --sql-file-suffix -odb
  --pgsql-server-version 9.1
  --default-pointer "std::shared_ptr")

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -pedantic\
  -std=c++14 \
  -DDATABASE_${DATABASE_U} \
  -DBOOST_LOG_DYN_LINK")

# Cmake module directory (FindOdb, FindThrift etc.)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

# Debug and Release version flags
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
