find_package(LibGit2 REQUIRED)

add_subdirectory(parser)
add_subdirectory(service)

install_webplugin(webgui)