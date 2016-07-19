include_directories(
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/model/include
  ${LLVM_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
)

set(SOURCES
  src/db/dbutil.cpp
  src/parser/parseutil.cpp
  src/dynamiclibrary.cpp
  src/logutil.cpp
)

add_library(util STATIC ${SOURCES})
target_compile_options(util PUBLIC -fPIC)
target_link_libraries(util
  gvc
  boost_regex
  sqlite3)