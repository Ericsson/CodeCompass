#include_directories(
#  include
#)
#
#set(SOURCES
#  include/plugin/pluginhandler.h
#  include/plugin/pluginhelper.h
#  include/plugin/servicenotavailexception.h
#)
#
#
#add_library(pluginframework STATIC ${SOURCES})
#set_target_properties(pluginframework PROPERTIES LINKER_LANGUAGE CXX)