find_package(yaml-cpp REQUIRED)

add_subdirectory(model)
add_subdirectory(parser)
add_subdirectory(service)

install_webplugin(webgui)