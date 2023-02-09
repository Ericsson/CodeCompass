message("Generating files from Thrift...")

execute_process(
  COMMAND npm install --save-dev @creditkarma/thrift-typescript
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/webgui-new)

execute_process(
  COMMAND npm run thrift-codegen
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/webgui-new)

message("Generating files finished.")