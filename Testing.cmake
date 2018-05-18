# Testing-specific configurations and setup calls.

if (NOT DEFINED TEST_DB)
  set(FUNCTIONAL_TESTING_ENABLED
    FALSE
    CACHE INTERNAL
    "Whether testing executable functionality was configured for the build.")

  message(WARNING "TEST_DB variable not set. Skipping generation of functional \
                   tests... To enable, set a database connection string which  \
                   can be used for the parsing of the test projects' files.")
else()
  enable_testing()

  # Initialize the variable in the cache and mark it as a string.
  set(TEST_DB "" CACHE STRING
    "Database connection string used in running functional tests.")

  set(FUNCTIONAL_TESTING_ENABLED
    TRUE
    CACHE INTERNAL
    "Whether testing executable functionality was configured for the build.")

  # Ensure that if TEST_DB is set, then it uses the database backend CodeCompass
  # will be built with.
  string(FIND "${TEST_DB}" "${DATABASE}:" testDbUsesProperBackend)
  if (NOT TEST_DB STREQUAL "" AND NOT testDbUsesProperBackend EQUAL 0)
    string(FIND "${TEST_DB}" ":" findPos)
    string(SUBSTRING "${TEST_DB}" 0 ${findPos} testDbBackend)
    message("${testDbBackend}")
    message(FATAL_ERROR "CodeCompass is being built with database backend     \
                         '${DATABASE}', but TEST_DB was set to use            \
                         '${testDbBackend}'. The same database system must be \
                         used, otherwise the tests won't run.")
  endif()
endif()
