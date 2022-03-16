cmake_minimum_required(VERSION 3.5.1)
project(CodeCompassCSharp)

add_custom_target(dotnetbuildservice ALL
COMMAND dotnet build -o ${CMAKE_CURRENT_BINARY_DIR}/csharpservice
WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/csharpservice 
DESTINATION ${INSTALL_SERVICE_DIR}
USE_SOURCE_PERMISSIONS)
