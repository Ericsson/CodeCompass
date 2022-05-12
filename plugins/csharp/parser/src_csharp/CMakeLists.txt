cmake_minimum_required(VERSION 3.5.1)
project(CodeCompassCSharp)

add_custom_target(dotnetbuild ALL
COMMAND dotnet build -r linux-x64 -o ${CMAKE_CURRENT_BINARY_DIR}/csharp
WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/csharp 
DESTINATION ${INSTALL_LIB_DIR}
USE_SOURCE_PERMISSIONS)
