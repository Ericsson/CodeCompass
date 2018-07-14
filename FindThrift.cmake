# - Try to find LibThrift
#  LIBTHRIFT_FOUND - System has LibThrift
#  LIBTHRIFT_INCLUDE_DIRS - The LibThrift include directories
#  LIBTHRIFT_LIBRARIES - The libraries needed to use LibThrift
#  LIBTHRIFT_DEFINITIONS - Compiler switches required for using LibThrift

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBTHRIFT QUIET "libthrift")
set(LIBTHRIFT_DEFINITIONS ${PC_LIBTHRIFT_CFLAGS_OTHER})

find_path(libthrift_INCLUDE_DIR
  NAMES thrift/Thrift.h
  HINTS ${PC_LIBTHRIFT_INCLUDE_DIR})

find_library(libthrift_LIBRARY
  NAMES thrift libthrift
  HINTS
    ${THIRFT_LIBRARY_PATH}
    ${PC_LIBTHIRFT_LIBRARY_DIRS})

find_program(thrift_BIN
  NAMES thrift
  HINTS
    ${libthrift_INCLUDE_DIR}/../bin)

mark_as_advanced(libthrift_INCLUDE_DIR libthrift_LIBRARY thrift_BIN)

set(THRIFT_LIBTHRIFT_INCLUDE_DIRS ${libthrift_INCLUDE_DIR}
  CACHE STRING "Thrift libthrift include dirs")
set(THRIFT_LIBTHRIFT_LIBRARIES ${libthrift_LIBRARY}
  CACHE STRING "Thrift libthrift library")
set(THRIFT_EXECUTABLE ${thrift_BIN}
  CACHE STRING "Thrift executable")

if(THRIFT_LIBTHRIFT_INCLUDE_DIRS AND THRIFT_LIBTHRIFT_LIBRARIES)
  set(THRIFT_LIBTHRIFT_FOUND TRUE)
endif()

if(Thrift_FIND_REQUIRED AND
    (NOT THRIFT_LIBTHRIFT_FOUND OR NOT THRIFT_EXECUTABLE))
  message(FATAL_ERROR "Couldn't find Thrift.")
endif()

set(THRIFT_INCLUDE_DIRS ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})
set(THRIFT_LIBRARIES ${THRIFT_LIBTHRIFT_LIBRARIES})

execute_process(COMMAND ${THRIFT_EXECUTABLE} --version
  OUTPUT_VARIABLE Thrift_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Found ${Thrift_VERSION}: ${THRIFT_EXECUTABLE}")
