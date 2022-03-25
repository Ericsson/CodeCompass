cmake_minimum_required(VERSION 3.5.1)
project(CodeCompassCSharp)

add_custom_target(dotnetbuildservice ALL
COMMAND ${THRIFT_EXECUTABLE} -gen netstd -r ${CMAKE_CURRENT_SOURCE_DIR}/../csharpservice.thrift
COMMAND dotnet build -o ${CMAKE_CURRENT_BINARY_DIR}/csharpservice
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/csharpservice 
DESTINATION ${INSTALL_SERVICE_DIR}
USE_SOURCE_PERMISSIONS)
