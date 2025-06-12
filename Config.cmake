# Set a default build type if none was specified
set(DEFAULT_BUILD_TYPE "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Installation directory for libraries
set(INSTALL_LIB_DIR_NAME "lib")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR_NAME}")

# Installation directory for parser libraries
set(INSTALL_PARSER_DIR_NAME "parserplugin")
set(INSTALL_PARSER_DIR "${INSTALL_LIB_DIR}/${INSTALL_PARSER_DIR_NAME}")

# Installation directory for service libraries
set(INSTALL_SERVICE_DIR_NAME "serviceplugin")
set(INSTALL_SERVICE_DIR "${INSTALL_LIB_DIR}/${INSTALL_SERVICE_DIR_NAME}")

# Installation directory for authentication libraries
set(INSTALL_AUTH_DIR_NAME "authplugin")
set(INSTALL_AUTH_DIR "${INSTALL_LIB_DIR}/${INSTALL_AUTH_DIR_NAME}")

# Installation directory of web GUI
set(INSTALL_WEBROOT_DIR "${CMAKE_INSTALL_PREFIX}/share/codecompass/webgui")

# Installation directory of the new React-based web GUI
set(INSTALL_WEBROOT_REACT_DIR "${CMAKE_INSTALL_PREFIX}/share/codecompass/webgui-new")

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

# Installation directory for the Python plugin
set(INSTALL_PYTHON_DIR "${INSTALL_LIB_DIR}/pythonplugin")

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
  ";$ORIGIN/../${INSTALL_LIB_DIR_NAME}"
  ";../${INSTALL_LIB_DIR_NAME}/${INSTALL_PARSER_DIR_NAME}"
  ";$ORIGIN/../${INSTALL_LIB_DIR_NAME}/${INSTALL_PARSER_DIR_NAME}"
  ";../${INSTALL_LIB_DIR_NAME}/${INSTALL_SERVICE_DIR_NAME}"
  ";$ORIGIN/../${INSTALL_LIB_DIR_NAME}/${INSTALL_SERVICE_DIR_NAME}"
  ";../${INSTALL_LIB_DIR_NAME}/${INSTALL_AUTH_DIR_NAME}"
  ";$ORIGIN/../${INSTALL_LIB_DIR_NAME}/${INSTALL_AUTH_DIR_NAME}")
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
  --include-prefix model/
  --include-with-brackets
  --default-pointer "std::shared_ptr")

# Set CXX standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set CXX flags
set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -pedantic\
  -DDATABASE_${DATABASE_U} \
  -DBOOST_LOG_DYN_LINK")

# Gold is the primary linker 
if(NOT DEFINED CODECOMPASS_LINKER)
    set(CODECOMPASS_LINKER "gold")
endif()

set(CMAKE_LINKER "${CODECOMPASS_LINKER}")

# Cmake module directory (FindOdb, FindThrift etc.)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

# Set build type specific flags here. Default values are:
# CMAKE_CXX_FLAGS_DEBUG:          -g
# CMAKE_CXX_FLAGS_RELEASE:        -O3 -DNDEBUG
# CMAKE_CXX_FLAGS_RELWITHDEBINFO: -O2 -g -DNDEBUG
# CMAKE_CXX_FLAGS_MINSIZEREL:     -Os -DNDEBUG
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -DNDEBUG")
