include_directories(
        ${PLUGIN_DIR}/model/include
        ${PLUGIN_DIR}/service/include
        ${CMAKE_CURRENT_BINARY_DIR}/../service/gen-cpp
        ${PROJECT_SOURCE_DIR}/model/include
        ${PROJECT_SOURCE_DIR}/plugins/cpp_metrics/model/include
        ${PROJECT_BINARY_DIR}/plugins/cpp_metrics/model/include)

include_directories(SYSTEM
        ${THRIFT_LIBTHRIFT_INCLUDE_DIRS})

add_executable(cppmetricsservicetest
        src/cppmetricstest.cpp
        src/cppmetricsservicetest.cpp)

add_executable(cppmetricsparsertest
        src/cppmetricstest.cpp
        src/cppmetricsparsertest.cpp)

target_compile_options(cppmetricsservicetest PUBLIC -Wno-unknown-pragmas)
target_compile_options(cppmetricsparsertest PUBLIC -Wno-unknown-pragmas)

target_link_libraries(cppmetricsservicetest
        model
        cppmodel
        cppservice
        ${Boost_LIBRARIES}
        ${GTEST_BOTH_LIBRARIES}
        pthread)

target_link_libraries(cppmetricsparsertest
        cppmetricsmodel
        model
        util
        cppmodel
        ${Boost_LIBRARIES}
        ${GTEST_BOTH_LIBRARIES}
        pthread)

if (NOT FUNCTIONAL_TESTING_ENABLED)
    fancy_message("Skipping generation of test project cppmetricstest." "yellow" TRUE)
else()
    set_property(DIRECTORY APPEND PROPERTY
            ADDITIONAL_MAKE_CLEAN_FILES
            "${CMAKE_CURRENT_BINARY_DIR}/build"
            "${CMAKE_CURRENT_BINARY_DIR}/workdir")

    add_test(NAME cppmetricsservice COMMAND cppmetricsservicetest
            "echo \"Test database used: ${TEST_DB}\" && \
       rm -rf ${CMAKE_CURRENT_BINARY_DIR}/build && \
       mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cd ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cmake ${CMAKE_CURRENT_SOURCE_DIR}/sources/service \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=on"
            "${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
       --database \"${TEST_DB}\" \
       --name cppmetricsservicetest \
       --input ${CMAKE_CURRENT_BINARY_DIR}/build/compile_commands.json \
       --workspace ${CMAKE_CURRENT_BINARY_DIR}/workdir/ \
       --force"
            "${TEST_DB}")

    add_test(NAME cppmetricsparser COMMAND cppmetricsparsertest
            "echo \"Test database used: ${TEST_DB}\" && \
       rm -rf ${CMAKE_CURRENT_BINARY_DIR}/build && \
       mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cd ${CMAKE_CURRENT_BINARY_DIR}/build && \
       cmake ${CMAKE_CURRENT_SOURCE_DIR}/sources/parser \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=on"
            "${CMAKE_INSTALL_PREFIX}/bin/CodeCompass_parser \
       --database \"${TEST_DB}\" \
       --name cppparsertest \
       --input ${CMAKE_CURRENT_BINARY_DIR}/build/compile_commands.json \
       --workspace ${CMAKE_CURRENT_BINARY_DIR}/workdir/ \
       --force"
            "${TEST_DB}")

    fancy_message("Generating test project for cppmetricstest." "blue" TRUE)
endif()