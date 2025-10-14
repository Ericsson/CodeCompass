# Set .NET build configuration based on CMAKE_BUILD_TYPE
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DOTNET_CONFIG "Debug")
else()
    set(DOTNET_CONFIG "Release")
endif()

add_subdirectory(parser)
add_subdirectory(service)

install_webplugin(webgui)