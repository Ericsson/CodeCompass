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
  pythonmodel
  ${Boost_LIBRARIES}
  ${Python3_LIBRARIES})

target_compile_options(pythonparser PUBLIC -Wno-unknown-pragmas)

set(VENV_DIR "${PLUGIN_DIR}/parser/venv/")
if(NOT EXISTS ${VENV_DIR})
  message("Creating Python virtual environment: ${VENV_DIR}")
  execute_process(
    COMMAND python3 -m venv venv
    WORKING_DIRECTORY ${PLUGIN_DIR}/parser/)
endif()

message("Installing Python dependencies...")
execute_process(
  COMMAND venv/bin/pip install -r requirements.txt
  WORKING_DIRECTORY ${PLUGIN_DIR}/parser/)

install(TARGETS pythonparser DESTINATION ${INSTALL_PARSER_DIR})
install(
  DIRECTORY pyparser/
  DESTINATION ${INSTALL_PYTHON_DIR}/pyparser)
install(
  DIRECTORY venv/
  DESTINATION ${INSTALL_PYTHON_DIR}/venv)

