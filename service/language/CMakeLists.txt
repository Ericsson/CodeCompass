include_directories(
  ${PROJECT_BINARY_DIR}/service/project/gen-cpp)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

set(LANGUAGETHRIFT_OUTPUT_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_types.h
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LanguageService.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LanguageService.h)

set(LANGUAGETHRIFT_THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/language.thrift)

set(LANGUAGETHRIFT_ADDITIONAL_ARGS --gen cpp --gen js)

generate_from_thrift("${LANGUAGETHRIFT_OUTPUT_FILES}" "${LANGUAGETHRIFT_THRIFT_FILE}" "${LANGUAGETHRIFT_ADDITIONAL_ARGS}")

set(languagethrift_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_constants.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/language_types.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/LanguageService.cpp)

add_library(languagethrift STATIC ${languagethrift_sources})

target_compile_options(languagethrift PUBLIC -fPIC)

add_dependencies(languagethrift projectthrift)
install_js_thrift()
