cmake_minimum_required(VERSION 2.6)

set(NAME "DogServer")
set(LIBNAME "lib${NAME}.a")
set(OUTPUT_LIB "lib${NAME}.a")

SET(CMAKE_BUILD_TYPE "DEBUG")
#SET(CMAKE_BUILD_TYPE "RELEASE")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -fPIC -g -Wno-deprecated")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -fPIC -O3 -Wall -Wno-deprecated")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -fPIC -O3 -Wall -Wno-deprecated")

MESSAGE(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE(STATUS "CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")

add_compile_options(-std=c++11)
SET(BUILD_PATH ${CMAKE_BINARY_DIR})
SET(SRC_PATH ${CMAKE_SOURCE_DIR})

include_directories(${SRC_PATH}/src)

link_directories(${BUILD_PATH})

add_subdirectory(src)
add_subdirectory(bin)

ADD_CUSTOM_TARGET(${LIBNAME} ALL
    COMMAND cp src/${LIBNAME} ${OUTPUT_LIB}
    COMMAND echo CREATE ${OUTPUT_LIB} > ar.mac
    COMMAND echo SAVE >> ar.mac
    COMMAND echo END >> ar.mac
    COMMAND echo OPEN ${OUTPUT_LIB} > ar.mac
    #COMMAND echo ADDLIB ${LIB_PATH}/libasr.a >> ar.mac
    COMMAND echo SAVE >> ar.mac
    COMMAND echo END >> ar.mac
    COMMAND ar -M < ar.mac
    COMMAND rm ar.mac
    WORKING_DIRECTORY ${BUILD_PATH})
ADD_DEPENDENCIES(${LIBNAME} ${NAME})
