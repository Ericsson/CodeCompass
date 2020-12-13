find_package(PythonLibs REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})

include_directories(
        include
        ${CMAKE_SOURCE_DIR}/util/include
        ${CMAKE_SOURCE_DIR}/model/include
        ${CMAKE_SOURCE_DIR}/parser/include
        ${PLUGIN_DIR}/model/include)

add_library(pythonparser SHARED
        src/pythonparser.cpp)

find_package (Python)

target_link_libraries(pythonparser
        util
        model
        pythonmodel
        ${PYTHON_LIBRARIES}
        ${Boost_LIBRARIES})

target_compile_options(pythonparser PUBLIC -Wno-unknown-pragmas)
target_link_options(pythonparser PUBLIC -Xlinker -export-dynamic)

install(TARGETS pythonparser DESTINATION ${INSTALL_PARSER_DIR})
install(
        DIRECTORY ${PLUGIN_DIR}/parser/src/scripts/
        DESTINATION ${INSTALL_PARSER_DIR}/scripts/python
        FILES_MATCHING PATTERN "*.py")
