find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(Boost REQUIRED COMPONENTS python)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${Python3_INCLUDE_DIRS})

add_library(pythonparser SHARED
  src/pythonparser.cpp)

target_link_libraries(pythonparser
  model
  ${Boost_LIBRARIES}
  ${Python3_LIBRARIES})

target_compile_options(pythonparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS pythonparser DESTINATION ${INSTALL_PARSER_DIR})
install(
        DIRECTORY ${PLUGIN_DIR}/parser/pyparser/
        DESTINATION ${INSTALL_PARSER_DIR}/pyparser/)
