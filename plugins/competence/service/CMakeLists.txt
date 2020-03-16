include_directories(
        include
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
        ${PROJECT_BINARY_DIR}/service/project/gen-cpp
        ${PROJECT_SOURCE_DIR}/service/project/include
        ${PLUGIN_DIR}/model/include
        ${PROJECT_SOURCE_DIR}/util/include
        ${PROJECT_SOURCE_DIR}/webserver/include)

include_directories(SYSTEM
        ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_custom_command(
        OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/competence_constants.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/competence_constants.h
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/competence_types.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/competence_types.h
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CompetenceService.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CompetenceService.h
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gen-js
        COMMAND
        ${THRIFT_EXECUTABLE} --gen cpp --gen js
        -o ${CMAKE_CURRENT_BINARY_DIR}
        -I ${PROJECT_SOURCE_DIR}/service
        ${CMAKE_CURRENT_SOURCE_DIR}/competence.thrift
        DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/competence.thrift
        COMMENT
        "Generating Thrift for competence.thrift")

add_library(competencethrift STATIC
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/competence_constants.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/competence_types.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp/CompetenceService.cpp)

target_compile_options(competencethrift PUBLIC -fPIC)

add_dependencies(competencethrift projectthrift)

add_library(competenceservice SHARED
        src/plugin.cpp
        src/competenceservice.cpp
        src/diagram.cpp)

target_link_libraries(competenceservice
        util
        ${THRIFT_LIBTHRIFT_LIBRARIES}
        model
        competencemodel
        projectthrift
        projectservice
        commonthrift
        competencethrift
        git2)

install(TARGETS competenceservice DESTINATION ${INSTALL_SERVICE_DIR})
install_js_thrift()
