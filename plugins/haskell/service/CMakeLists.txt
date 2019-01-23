include_directories(
  include
  gen-cpp
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
  ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/haskell_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/haskell_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/haskell_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/haskell_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HaskellService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HaskellService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-hs/Haskell_Consts.hs
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-hs/Haskell_Types.hs
    ${CMAKE_CURRENT_SOURCE_DIR}/gen-hs
  COMMAND
    ${THRIFT_EXECUTABLE} --gen hs --gen cpp
      -o ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/haskell.thrift
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/haskell.thrift
  COMMENT
    "Generating Thrift for haskell.thrift")

add_library(haskellthrift STATIC
  gen-cpp/haskell_constants.cpp
  gen-cpp/haskell_types.cpp
  gen-cpp/HaskellService.cpp)

target_compile_options(haskellthrift PUBLIC -fPIC)

add_library(haskellservice SHARED
  src/haskellservice.cpp
  src/plugin.cpp)

target_compile_options(haskellservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(haskellservice
  util
  ${THRIFT_LIBTHRIFT_LIBRARIES}
  ${ODB_LIBRARIES}
  haskellthrift)

install(TARGETS haskellservice DESTINATION ${INSTALL_SERVICE_DIR})
