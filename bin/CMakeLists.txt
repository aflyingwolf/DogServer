SET(DEP
    ${OUTPUT_LIB}
    -ldl
    -lpthread
    -luuid
    -lgfortran)
MESSAGE(STATUS "Dep in server Demo:${DEP}")

SET(BINS log-test dog-server dog-client)
foreach(bin ${BINS})
  MESSAGE(STATUS "generate make for ${bin}")
  add_executable(${bin} ${bin}.cc)
  target_link_libraries(${bin} ${DEP})
endforeach()
