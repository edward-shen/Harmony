cmake_minimum_required(VERSION 3.5.0)
project(Harmony)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

if (WIN32)
set (CMAKE_PREFIX_PATH "C:/Qt5/5.9.1/msvc2015/")
endif (WIN32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)

file(GLOB hw_SRC
    "${CMAKE_SOURCE_DIR}/harmony-ui/*.cpp"
    "${CMAKE_SOURCE_DIR}/harmony-ui/*.h"
    "${CMAKE_SOURCE_DIR}/harmony-ui/*.ui"
)

include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_SOURCE_DIR}/harmony-ui)
include_directories (${PROJECT_BINARY_DIR})

add_library (HarmonyWindow STATIC 
  ${hw_SRC}
)
include_directories(${Qt5Widgets_INCLUDE_DIRS})
target_link_libraries (HarmonyWindow Qt5::Widgets)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/harmony-ui ${CMAKE_SOURCE_DIR}/include/np1sec/src ${CMAKE_SOURCE_DIR}/include/spread-src-4.4.0/include ${CMAKE_SOURCE_DIR}/include/gcrypt ${CMAKE_SOURCE_DIR}/include/json)

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
target_link_libraries(harmony spread-core spread gcrypt HarmonyWindow)
endif (WIN32)

if (UNIX)
find_package (Threads)
add_library(spread STATIC IMPORTED)
set_target_properties(spread PROPERTIES IMPORTED_LOCATION /usr/local/lib/libspread.so)
add_library(spread-core STATIC IMPORTED)
set_target_properties(spread-core PROPERTIES IMPORTED_LOCATION /usr/local/lib/libspread-core.so)
add_library(gcrypt STATIC IMPORTED)
set_target_properties(gcrypt PROPERTIES IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libgcrypt.so)
target_link_libraries(harmony spread-core spread gcrypt HarmonyWindow ${CMAKE_THREAD_LIBS_INIT})
endif (UNIX)

install(TARGETS harmony DESTINATION bin)
