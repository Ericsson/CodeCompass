include_directories(
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_types.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LanguageService.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LanguageService.h
    ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gen-js
  COMMAND
    ${THRIFT_EXECUTABLE} --gen cpp --gen js
      -o ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/language.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/language.thrift
  COMMENT
    "Generating Thrift for language.thrift")

add_library(languagethrift STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LanguageService.cpp)

target_compile_options(languagethrift PUBLIC -fPIC)

add_dependencies(languagethrift projectthrift)
install_js_thrift()
