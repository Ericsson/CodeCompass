include_directories(
    include
    ${PROJECT_SOURCE_DIR}/util/include
    ${PROJECT_SOURCE_DIR}/webserver/include
    ${PROJECT_SOURCE_DIR}/model/include
    ${PROJECT_BINARY_DIR}/service/language/gen-cpp
    ${PROJECT_BINARY_DIR}/service/project/gen-cpp
    ${PROJECT_SOURCE_DIR}/service/project/include
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp)

include_directories(SYSTEM
        ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

# Generate thrift files
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CSharpService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-csharp
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen netstd
    -o ${CMAKE_CURRENT_BINARY_DIR}
    -I ${PROJECT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/csharpservice.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/csharpservice.thrift
  COMMENT
    "Generating Thrift for csharpservice.thrift")

# Create cpp static library from thrift files
add_library(csharpservicethrift STATIC
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CSharpService.cpp)

target_compile_options(csharpservicethrift PUBLIC -fPIC)

add_dependencies(csharpservicethrift languagethrift commonthrift projectthrift)

add_subdirectory(src_csharp)

add_library(csharpservice SHARED
        src/csharpservice.cpp
        src/plugin.cpp)

target_compile_options(csharpservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(csharpservice
        util
        model
        mongoose
        csharpservicethrift
        projectservice
        projectthrift
        languagethrift
        gvc
        ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS csharpservice DESTINATION ${INSTALL_SERVICE_DIR})