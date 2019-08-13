find_package(Git REQUIRED)

add_subdirectory(parser)
add_subdirectory(service)

install_webplugin(webgui)