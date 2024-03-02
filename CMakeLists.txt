cmake_minimum_required(VERSION 3.16)

project(SADG)

IF(NOT CMAKE_BUILD_TYPE)
    #SET(CMAKE_BUILD_TYPE "DEBUG")
    SET(CMAKE_BUILD_TYPE "RELEASE")
    #SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
    #SET(CMAKE_BUILD_TYPE "MINSIZEREL")
ENDIF()

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(Boost)
IF(Boost_FOUND)
  SET(Boost_USE_STATIC_LIBS OFF)
  SET(Boost_USE_MULTITHREADED ON)
  SET(Boost_USE_STATIC_RUNTIME OFF)
  FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options)
ELSEIF(NOT Boost_FOUND)
  MESSAGE(FATAL_ERROR "Unable to find correct Boost version. Did you set BOOST_ROOT?")
ENDIF()

include_directories("inc")
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/**.cpp")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Werror -Ofast")

add_executable(sadg ${SOURCES})
TARGET_LINK_LIBRARIES(sadg ${Boost_LIBRARIES})

add_custom_target(clean_all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/sadg
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.so
)


set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_BINARY_DIR}/sadg;
     ${CMAKE_BINARY_DIR}/build;
     ${CMAKE_BINARY_DIR}/*.so"
)