cmake_minimum_required(VERSION 3.5.1)
project(CodeCompassCSharp)

add_custom_target(dotnetbuild ALL
COMMAND dotnet build -o ./build
WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)