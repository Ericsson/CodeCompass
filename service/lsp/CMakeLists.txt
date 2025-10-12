include_directories(include)

add_library(lspservice STATIC
  src/lspservice.cpp
  src/lsp_types.cpp)

target_compile_options(lspservice PUBLIC -fPIC)

target_link_libraries(lspservice
  ${Boost_LIBRARIES})
