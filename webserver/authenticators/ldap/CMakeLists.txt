find_package(OpenLdap REQUIRED)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/util/include
  ${PROJECT_SOURCE_DIR}/webserver/include
  ${PROJECT_SOURCE_DIR}/webserver/authenticators/ldap/ldap-cpp/
  ${OPENLDAP_INCLUDE_DIR})

add_library(ldapauth SHARED
  src/plugin.cpp
  ldap-cpp/cldap_entry.cpp
  ldap-cpp/cldap_mod.cpp
  ldap-cpp/cldap_server.cpp)

target_compile_options(ldapauth PUBLIC -Wno-unknown-pragmas)

target_link_libraries(ldapauth
  ${OPENLDAP_LIBRARIES}
  util)

install(TARGETS ldapauth DESTINATION ${INSTALL_AUTH_DIR})
