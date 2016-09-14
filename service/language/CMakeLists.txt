include_directories(
  ../project/gen-cpp
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/language_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/language_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/language_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/language_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LanguageService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LanguageService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
  COMMAND
    thrift --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/language.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/language.thrift
  COMMENT
    "Generate thrift for language.thrift")

add_library(languagethrift STATIC
  gen-cpp/language_constants.cpp
  gen-cpp/language_types.cpp
  gen-cpp/LanguageService.cpp)

target_compile_options(languagethrift PUBLIC -fPIC)

add_dependencies(languagethrift projectthrift)
