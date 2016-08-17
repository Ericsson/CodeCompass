include_directories(
  include/mongoose
)

set(SOURCES src/mongoose.c)

add_library(mongoose SHARED ${SOURCES})