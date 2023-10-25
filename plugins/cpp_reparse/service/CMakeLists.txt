find_package(Clang REQUIRED CONFIG)

include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_BINARY_DIR}/service/language/gen-cpp
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp
  ${PROJECT_SOURCE_DIR}/service/project/include
  ${cpp_PLUGIN_DIR}/model/include
  ${cpp_PLUGIN_DIR}/parser/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS}
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS})

link_directories(${LLVM_LIBRARY_DIRS})

add_definitions(${LLVM_DEFINITIONS})

set(CPPREPARSETHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cppreparse_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cppreparse_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cppreparse_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cppreparse_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppReparseService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppReparseService.h)

set(CPPREPARSETHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cppreparse.thrift)

set(CPPREPARSETHRIFT_ADDITIONAL_ARGS --gen cpp --gen js:jquery -I ${PROJECT_SOURCE_DIR}/service)

generate_from_thrift("${CPPREPARSETHRIFT_OUTPUT_FILES}" "${CPPREPARSETHRIFT_THRIFT_FILE}" "${CPPREPARSETHRIFT_ADDITIONAL_ARGS}")

set(cppreparsethrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cppreparse_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/cppreparse_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CppReparseService.cpp)

add_library(cppreparsethrift STATIC ${cppreparsethrift_sources})

target_compile_options(cppreparsethrift PUBLIC -fPIC)

add_dependencies(cppreparsethrift commonthrift projectthrift)

add_library(cppreparseservice SHARED
  src/plugin.cpp
  src/cppreparseservice.cpp
  src/astcache.cpp
  src/asthtml.cpp
  src/databasefilesystem.cpp
  src/reparser.cpp)

target_compile_options(cppreparseservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(cppreparseservice
  util
  model
  cppmodel
  mongoose
  projectservice
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  cppreparsethrift
  clangTooling
  clangFrontend
  clangBasic
  clangAST
  clang
  )

install(TARGETS cppreparseservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
