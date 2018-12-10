# The C++ reparser plugin depends on the C++ plugin's components to be built.

if ("${cpp_PLUGIN_DIR}" STREQUAL "")
  # Use SEND_ERROR here so a build file is not generated at the end.
  # CodeCompass might use a lot of plugins and produce a lengthy build, so
  # a warning at configure time would easily be missed by the users.
  message(SEND_ERROR
    "C++ Reparse plugin found without C++ plugin in the plugins directory.")
endif()

add_subdirectory(service)

install_webplugin(webgui)
