set(SOURCES
  async-logging.cc
  countdown-latch.cc
  cur-thread.cc
  log-file.cc
  logging.cc
  log-stream.cc
  log-util.cc)
add_library(${NAME} STATIC ${SOURCES})
