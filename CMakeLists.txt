cmake_minimum_required(VERSION 3.1)
project(Harmony)
set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include/np1sec/src ${CMAKE_SOURCE_DIR}/include/spread-src-4.4.0/include ${CMAKE_SOURCE_DIR}/include/gcrypt)

file(GLOB harmony_SRC
    "${CMAKE_SOURCE_DIR}/*.h"
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/np1sec/src/*.h"
    "${CMAKE_SOURCE_DIR}/include/np1sec/src/*.cc"
)

set(SOURCES ${harmony_SRC})
add_executable(harmony ${harmony_SRC})

if (WIN32)
add_library(spread STATIC IMPORTED)
set_property(TARGET spread PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libspread.lib)
add_library(spread-core STATIC IMPORTED)
set_property(TARGET spread-core PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libspread-core.lib)
add_library(gcrypt STATIC IMPORTED)
set_property(TARGET gcrypt PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libgcrypt.lib)
target_link_libraries(harmony spread-core spread gcrypt)
endif (WIN32)

if (UNIX)
add_library(spread STATIC IMPORTED)
add_library(spread-core STATIC IMPORTED)
add_library(gcrypt STATIC IMPORTED)
target_link_libraries(harmony spread-core spread gcrypt)
endif (UNIX)

install(TARGETS harmony DESTINATION bin)