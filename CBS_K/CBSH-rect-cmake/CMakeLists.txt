﻿# CMakeList.txt : CMake project for CBSH-rect-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
project(CBS-K)

# set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
elseif()
	set(CMAKE_CXX_FLAGS "-Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O1")
endif()

set (CMAKE_CXX_STANDARD 14)
#SET(BUILD_SHARED_LIBS OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")


include_directories(../mapVisualiser/)
include_directories(".")


file(GLOB_RECURSE SOURCES "*.cpp")
add_executable(CBS-K ${SOURCES})

# add_executable (CBS-K
# 			constrained_agents_loader.h
# 			constrained_agents_loader.cpp
# 			constrained_map_loader.h
# 			constrained_map_loader.cpp
# 			driver.cpp
# 			agents_loader.h
# 			compute_heuristic.h
# 			ICBSNode.h
# 			ICBSSearch.h
# 			LLNode.h
# 			map_loader.h
# 			MDD.h
# 			RectangleReasoning.h
# 			SingleAgentICBS.h
# 			agents_loader.cpp
# 			compute_heuristic.cpp
# 			ICBSNode.cpp
# 			ICBSSearch.cpp
# 			LLNode.cpp
# 			map_loader.cpp
# 			MDD.cpp
# 			RectangleReasoning.cpp
# 			SingleAgentICBS.cpp
# 			ReservationTable.h
# 			ReservationTable.cpp
# 			Conflict.h
# 			Conflict.cpp
# 			ConstraintTable.cpp
# 			ConstraintTable.h
# 			CorridorReasoning.cpp
# 			CorridorReasoning.h
# 			egraph_reader.h
# 			common.h
# 			common.cpp
# 			# ../mapVisualiser/Visualiser.h
# 			# ../mapVisualiser/Visualiser.cpp
# 			MDDNode.h
# 			MDDNode.cpp
# 			ICBSHSearchPairAnalysis.cpp ICBSHSearchPairAnalysis.h Path.h)






# Google's sparsehash
if (WIN32)
	cmake_policy(SET CMP0074 OLD)
#	set( "C:/Users/czcz2/Google Drive/MIT/GCITR/boost_1_71_0/boost")

	include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/sparsehash/src")
	include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/sparsehash/src/windows")
	include_directories("C:/Users/czcz2/Anaconda3/include")
	include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/boost_1_71_0")
	include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/boost_1_71_0/stage/lib")
	include_directories("C:/Users/czcz2/Anaconda3/libs")
	include_directories("C:/Users/czcz2/Anaconda3/include")
	#include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-32/include")
	#include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/include")
	#include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-32/lib")
	#include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/lib")
	#include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-32/bin")
	#include_directories("C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/bin")
	# set(SFML_DIR "C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/lib/cmake/SFML")
	# set(SFML_INCLUDE_DIR "C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/include")
	

	set(PYTHON_INCLUDE_DIRS "C:/Users/czcz2/Anaconda3/include")
	set(PYTHON_LIBRARIES "C:/Users/czcz2/Anaconda3/libs")
	find_package(PythonInterp 3.7 REQUIRED)
	# This goes after, since it uses PythonInterp as hint
	find_package(PythonLibs 3.7 REQUIRED)
	include_directories(${PYTHON_INCLUDE_DIRS})
	MESSAGE(${PYTHON_LIBRARIES} ${PYTHON_INCLUDE_DIRS})


	# Detect and add SFML
	#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
	#Find any version 2.X of SFML
	#See the FindSFML.cmake file for additional details and instructions
	# find_package(SFML 2.5.1 REQUIRED  graphics window system)
	# if(SFML_FOUND)
	# include_directories(${SFML_INCLUDE_DIR})
	# target_link_libraries(CBS-K ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
	# endif()
	
	SET(Boost_NO_BOOST_CMAKE ON)
	set(BOOST_ROOT "C:/Users/czcz2/Google Drive/MIT/GCITR/boost_1_61_0/")
	set(BOOST_LIBRARYDIR "C:/Users/czcz2/Google Drive/MIT/GCITR/boost_1_61_0/lib64-msvc-14.0")
	FIND_PACKAGE(Boost 1.61.0 COMPONENTS system program_options python)
	IF(Boost_FOUND)
		SET(Boost_USE_STATIC_LIBS OFF)
		SET(Boost_USE_MULTITHREADED ON)
		SET(Boost_USE_STATIC_RUNTIME OFF)
		include_directories(${Boost_INCLUDEDIR} )

		add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
		TARGET_LINK_LIBRARIES(CBS-K ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})


	ELSEIF(NOT Boost_FOUND)
	  MESSAGE(FATAL_ERROR "Unable to find all boost libs")
	ENDIF()

elseif(APPLE)
	add_definitions('-std=c++11')

	message(STATUS ">>>> Found APPLE <<<<")
	set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
	#	add_definitions('-DBoost_NO_BOOST_CMAKE=ON')

	find_package(PythonInterp 3.7 EXACT REQUIRED)
	find_package(PythonLibs 3.7 EXACT REQUIRED)

	include_directories(${PYTHON_INCLUDE_DIRS})

	set(Boost_USE_STATIC_LIBS OFF)
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_USE_STATIC_RUNTIME OFF)

	# Other boost packages still use normal boost library
	FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options thread python)
	include_directories(${Boost_INCLUDE_DIRS})

#	# Find py-boost include dir and py-boost lib
#	find_path(BOOST_PYTHON_INCLUDE_DIR boost/python.hpp HINTS ../py-boost/include)
#	find_library(BOOST_PYTHON NAMES libboost_python37.a HINTS ../py-boost/lib)
#	include_directories(${BOOST_PYTHON_INCLUDE_DIR})

	#Print find result. Make its correct
#	message(STATUS ${PYTHON_INCLUDE_DIRS})
#	message(STATUS ${BOOST_PYTHON_INCLUDE_DIR})
#	message(STATUS ${BOOST_PYTHON})

	target_link_libraries(CBS-K  ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${BOOST_PYTHON})


else()
	file(READ "/etc/issue" ETC_ISSUE)
	string(REGEX MATCH "Ubuntu|Manjaro" DIST ${ETC_ISSUE})
    if(DIST STREQUAL "Ubuntu")
		message(STATUS ">>>> Found Ubuntu <<<<")
		set(boostPython python-py36)
	elseif(DIST STREQUAL "Manjaro")
		message(STATUS ">>>> Found Manjaro <<<<")
		set(boostPython python38)
	else()
		message(STATUS ">>>> Found unknown distribution <<<<")
		set(boostPython python37)
	endif()

	find_path(SPARSEHASH_INCLUDE_DIRS sparsehash/dense_hash_map)
	if (NOT SPARSEHASH_INCLUDE_DIRS)
		message(FATAL_ERROR "Could not find <sparsehash/dense_hash_map>.")
	endif ()
	include_directories(SYSTEM ${SPARSEHASH_INCLUDE_DIRS} )

	find_package(PythonInterp 3.8 REQUIRED)

	# This goes after, since it uses PythonInterp as hint
	find_package(PythonLibs 3.8 REQUIRED)
	include_directories(${PYTHON_INCLUDE_DIRS})
	MESSAGE(${PYTHON_LIBRARIES} ${PYTHON_INCLUDE_DIRS})

	FIND_PACKAGE(Boost)
	IF(Boost_FOUND)
	  SET(Boost_USE_STATIC_LIBS OFF)
	  SET(Boost_USE_MULTITHREADED ON)
	  SET(Boost_USE_STATIC_RUNTIME OFF)
	  FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options python)
	  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

	  TARGET_LINK_LIBRARIES(CBS-K ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})


	  

	ELSEIF(NOT Boost_FOUND)
	  MESSAGE(FATAL_ERROR "Unable to find correct Boost version. Did you set BOOST_ROOT?")
	ENDIF()

	

endif()
