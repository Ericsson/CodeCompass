cmake_minimum_required(VERSION 3.5.1)
project(CodeCompassCSharp)

add_custom_target(dotnetbuildservice ALL
  COMMAND ${THRIFT_EXECUTABLE} --gen netstd
    -o ${CMAKE_CURRENT_BINARY_DIR}
    -I ${PROJECT_SOURCE_DIR}/service
    -r ${CMAKE_CURRENT_SOURCE_DIR}/../csharpservice.thrift
  COMMAND dotnet new classlib -o ${CMAKE_CURRENT_BINARY_DIR}/gen-netstd/ --force
  COMMAND dotnet add reference ${CMAKE_CURRENT_BINARY_DIR}/gen-netstd/gen-netstd.csproj
  COMMAND dotnet add ${CMAKE_CURRENT_BINARY_DIR}/gen-netstd/gen-netstd.csproj reference ${CMAKE_SOURCE_DIR}/lib/csharp/thrift_netstd/Thrift/Thrift.csproj
  COMMAND dotnet build -o ${CMAKE_CURRENT_BINARY_DIR}/csharpservice
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/csharpservice 
DESTINATION ${INSTALL_SERVICE_DIR}
USE_SOURCE_PERMISSIONS)
