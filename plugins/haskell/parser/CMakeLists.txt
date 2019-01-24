find_package(Clang REQUIRED CONFIG)

llvm_map_components_to_libnames(llvm_libs support core irreader)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include)

include_directories(SYSTEM
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS})

link_directories(${LLVM_LIBRARY_DIRS})

add_definitions(${LLVM_DEFINITIONS})

add_library(haskellparser SHARED
  src/haskellparser.cpp)

target_link_libraries(haskellparser
  clangTooling
  clangFrontend
  LLVMMCParser
  LLVMOption
  ${llvm_libs})

target_compile_options(haskellparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS haskellparser DESTINATION ${INSTALL_PARSER_DIR})
