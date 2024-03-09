cmake_minimum_required(VERSION 3.16)

project(SADG)

if(NOT CMAKE_BUILD_TYPE)
    #set(CMAKE_BUILD_TYPE "DEBUG")
    set(CMAKE_BUILD_TYPE "RELEASE")
    #set(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
    #set(CMAKE_BUILD_TYPE "MINSIZEREL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Werror -Ofast")

find_package(Boost)
if(Boost_FOUND)
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
  find_package(Boost REQUIRED COMPONENTS system program_options filesystem)
elseif(NOT Boost_FOUND)
  message(FATAL_ERROR "Unable to find correct Boost version. Did you set BOOST_ROOT?")
endif()

file(GLOB_RECURSE SOURCES "src/**.cpp")
list(FILTER SOURCES EXCLUDE REGEX "(src/simulate.cpp)|(src/generate.cpp)|(src/enumerate_grouping.cpp)")

set(SIMULATE_SOURCES ${SOURCES} "src/simulate.cpp")
set(GENERATE_SOURCES ${SOURCES} "src/generate.cpp")
set(GROUPING_SOURCES ${SOURCES} "src/enumerate_grouping.cpp")

add_executable(simulate ${SIMULATE_SOURCES})
target_include_directories(simulate PUBLIC "inc")
target_include_directories(simulate PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(simulate ${Boost_LIBRARIES})

add_executable(generate ${GENERATE_SOURCES})
target_include_directories(generate PUBLIC "inc")
target_include_directories(generate PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(generate ${Boost_LIBRARIES})

add_executable(enumerate_grouping ${GROUPING_SOURCES})
target_include_directories(enumerate_grouping PUBLIC "inc")

add_custom_target(clean_all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/simulate
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/generate
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/enumerate_grouping
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.so
)