include_directories(
        include
        ${PROJECT_SOURCE_DIR}/util/include
        ${PROJECT_SOURCE_DIR}/model/include
        ${PROJECT_BINARY_DIR}/service/language/gen-cpp
        ${PROJECT_BINARY_DIR}/service/project/gen-cpp
        ${PROJECT_SOURCE_DIR}/service/project/include
        ${PLUGIN_DIR}/model/include)

include_directories(SYSTEM
        ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_library(pythonservice SHARED
        src/pythonservice.cpp)

target_compile_options(pythonservice PUBLIC -Wno-unknown-pragmas)

target_link_libraries(pythonservice
        util
        model
        pythonmodel
        mongoose
        projectservice
        languagethrift
        gvc
        ${THRIFT_LIBTHRIFT_LIBRARIES})

install(TARGETS pythonservice DESTINATION ${INSTALL_SERVICE_DIR})
