find_package(Git2 REQUIRED)

add_subdirectory(parser)
add_subdirectory(service)

install_webplugin(webgui)