# - Try to find LibThrift
#  LIBTHRIFT_FOUND - System has LibThrift
#  LIBTHRIF_INCLUDE_DIRS - The LibThrift include directories
#  LIBTHRIF_LIBRARIES - The libraries needed to use LibThrift
#  LIBTHRIF_DEFINITIONS - Compiler switches required for using LibThrift

find_package(PkgConfig)
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

set(THRIFT_LIBTHRIFT_INCLUDE_DIRS ${libthrift_INCLUDE_DIR} CACHE STRING "Thrift libthrift include dirs")
set(THRIFT_LIBTHRIFT_LIBRARIES ${libthrift_LIBRARY} CACHE STRING "Thrift libthrift library")
set(THRIFT_EXECUTABLE ${thrift_BIN} CACHE STRING "Thrift executable")

if(THRIFT_LIBTHRIFT_INCLUDE_DIRS AND THRIFT_LIBTHRIFT_LIBRARIES)
	set(THRIFT_LIBTHRIFT_FOUND TRUE)
endif()

set(THRIFT_INCLUDE_DIRS ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})
set(THRIFT_LIBRARIES ${THRIFT_LIBTHRIFT_LIBRARIES})