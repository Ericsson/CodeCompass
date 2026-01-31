add_custom_target(dotnetbuildservice
  COMMAND ${THRIFT_EXECUTABLE} --gen netstd
    -o ${CMAKE_CURRENT_BINARY_DIR}
    -I ${PROJECT_SOURCE_DIR}/service
    -r ${CMAKE_CURRENT_SOURCE_DIR}/../csharpservice.thrift
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_target(dotnetaddclasslib
  COMMAND dotnet new classlib -o ${CMAKE_CURRENT_BINARY_DIR}/gen-netstd/ -f "net8.0" --force
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_dependencies(dotnetaddclasslib dotnetbuildservice)

add_custom_target(dotnetaddthriftlib
  COMMAND dotnet add package ApacheThrift --version 0.16.0
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen-netstd/"
)

add_dependencies(dotnetaddthriftlib dotnetaddclasslib)

add_custom_target(dotnetbuildfiles ALL
  COMMAND dotnet build -c ${DOTNET_CONFIG} -o ${CMAKE_CURRENT_BINARY_DIR}/csharpservice -p:BuildDir=${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_dependencies(dotnetbuildfiles dotnetaddthriftlib)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/csharpservice 
  DESTINATION ${INSTALL_SERVICE_DIR}
  USE_SOURCE_PERMISSIONS
)
