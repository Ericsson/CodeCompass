include_directories(
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/parser/include
  ${PLUGIN_DIR}/model/include)

add_library(competenceparser SHARED
        src/competenceparser.cpp
        src/gitoperations.cpp
        src/expertisecalculation.cpp)

target_link_libraries(competenceparser
        model
        competencemodel
        util
        git2)

target_compile_options(competenceparser PUBLIC -Wno-unknown-pragmas)

install(TARGETS competenceparser DESTINATION ${INSTALL_PARSER_DIR})
