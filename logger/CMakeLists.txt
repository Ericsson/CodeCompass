set(_src
  src/ldlogger-hooks.c
  src/ldlogger-logger.c
  src/ldlogger-tool.c
  src/ldlogger-tool-gcc.c
  src/ldlogger-tool-javac.c
  src/ldlogger-util.c)

add_definitions(-D_GNU_SOURCE -D__LOGGER_MAIN__)

add_executable(logger ${_src})
target_link_libraries(logger
  ${CMAKE_DL_LIBS})

add_library(ldlogger SHARED ${_src})
target_compile_options(ldlogger PUBLIC -Wno-unknown-pragmas)
target_link_libraries(ldlogger
  ${CMAKE_DL_LIBS})

install(TARGETS ldlogger
  DESTINATION ${INSTALL_LIB_DIR}/x86_64)

install(TARGETS logger
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})