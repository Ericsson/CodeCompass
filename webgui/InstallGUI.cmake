message("Install npm packages...")

execute_process(
  COMMAND cp ${CC_PACKAGE} ${INSTALL_SCRIPTS_DIR}/package.json
  COMMAND npm install
  WORKING_DIRECTORY ${INSTALL_SCRIPTS_DIR})

message("Installation of npm packages are finished.")