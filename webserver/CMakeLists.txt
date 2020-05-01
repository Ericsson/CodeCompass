add_subdirectory(authenticators)

add_compile_definitions(
  MG_ENABLE_DEBUG=0

  # Turn off some unneeded by default enabled features of Mongoose.
  # Makes the binary smaller!
  CS_DISABLE_MD5=1
  CS_DISABLE_SHA1=1
  MG_DISABLE_HTTP_DIGEST_AUTH=1
  MG_ENABLE_ASYNC_RESOLVER=0
  MG_ENABLE_CALLBACK_USERDATA=0
  MG_ENABLE_DNS=0
  MG_ENABLE_FILESYSTEM=1
  MG_ENABLE_HTTP_WEBSOCKET=0
  MG_ENABLE_HTTP_URL_REWRITES=0
  MG_ENABLE_IPV6=0
  MG_ENABLE_MQTT=0

  # Only exposes a pthread_create() API, changes no behaviour w.r.t CodeCompass.
  MG_ENABLE_THREADS=0
)

find_package(OpenSSL)
if (NOT OPENSSL_FOUND)
  message(WARNING "OpenSSL libraries not found on the system. Webserver will "
                  "only be able to use conventional HTTP!")
  add_compile_definitions(MG_ENABLE_SSL=0)
else()
  message(STATUS "Found OpenSSL version ${OPENSSL_VERSION}: ${OPENSSL_LIBRARIES}")
  add_compile_definitions(MG_ENABLE_SSL=1)
endif()

add_library(mongoose STATIC
  src/mongoose.c)
target_include_directories(mongoose PUBLIC
  include/webserver)
target_compile_options(mongoose PUBLIC
  -fPIC)
if (OPENSSL_FOUND)
target_link_libraries(mongoose PUBLIC
  OpenSSL::SSL
  OpenSSL::Crypto)
endif()

add_executable(CodeCompass_webserver
  src/webserver.cpp
  src/authentication.cpp
  src/mainrequesthandler.cpp
  src/session.cpp
  src/sessionmanager.cpp
  src/mongooseserver.cpp)

set_target_properties(CodeCompass_webserver
  PROPERTIES ENABLE_EXPORTS 1)

target_include_directories(CodeCompass_webserver PUBLIC
  include
  ${PROJECT_SOURCE_DIR}/model/include
  ${PROJECT_SOURCE_DIR}/util/include)

find_boost_libraries(
  filesystem
  log
  program_options
  system
  thread)
target_link_libraries(CodeCompass_webserver
  util
  mongoose
  ${Boost_LINK_LIBRARIES}
  ${ODB_LIBRARIES}
  pthread
  dl)

install(TARGETS CodeCompass_webserver
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR})
