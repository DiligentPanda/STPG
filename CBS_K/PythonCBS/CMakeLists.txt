﻿# CMakeList.txt : CMake project for CBSH-rect-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

IF(NOT CMAKE_BUILD_TYPE)
  #SET(CMAKE_BUILD_TYPE "DEBUG")
  SET(CMAKE_BUILD_TYPE "RELEASE")
  #SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
  #SET(CMAKE_BUILD_TYPE "MINSIZEREL")
ENDIF()

ADD_LIBRARY(PythonCBS SHARED
		PythonCBS.cpp
		PythonCBS.h
		../CBSH-rect-cmake/flat_map_loader.h
		../CBSH-rect-cmake/flat_map_loader.cpp
		../CBSH-rect-cmake/agents_loader.h
		../CBSH-rect-cmake/compute_heuristic.h
		../CBSH-rect-cmake/ICBSNode.h
		../CBSH-rect-cmake/ICBSSearch.h
		../CBSH-rect-cmake/LLNode.h
		../CBSH-rect-cmake/map_loader.h
		../CBSH-rect-cmake/MDD.h
		../CBSH-rect-cmake/RectangleReasoning.h
		../CBSH-rect-cmake/SingleAgentICBS.h
		../CBSH-rect-cmake/agents_loader.cpp
	   ../CBSH-rect-cmake/compute_heuristic.cpp
		../CBSH-rect-cmake/ICBSNode.cpp
		../CBSH-rect-cmake/ICBSSearch.cpp
		../CBSH-rect-cmake/LLNode.cpp
		../CBSH-rect-cmake/map_loader.cpp
		../CBSH-rect-cmake/MDD.cpp
		../CBSH-rect-cmake/RectangleReasoning.cpp
		../CBSH-rect-cmake/SingleAgentICBS.cpp
		../CBSH-rect-cmake/ReservationTable.h
		../CBSH-rect-cmake/ReservationTable.cpp
		../CBSH-rect-cmake/Conflict.h
		../CBSH-rect-cmake/Conflict.cpp
		../CBSH-rect-cmake/ConstraintTable.cpp
		../CBSH-rect-cmake/ConstraintTable.h
		../CBSH-rect-cmake/CorridorReasoning.cpp
		../CBSH-rect-cmake/CorridorReasoning.h
		../CBSH-rect-cmake/egraph_reader.h
		../CBSH-rect-cmake/common.h
		../CBSH-rect-cmake/common.cpp
		../CBSH-rect-cmake/MDDNode.h
		../CBSH-rect-cmake/MDDNode.cpp
		../CBSH-rect-cmake/ICBSHSearchPairAnalysis.cpp ../CBSH-rect-cmake/ICBSHSearchPairAnalysis.h)



INCLUDE_DIRECTORIES("../CBSH-rect-cmake")

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
	set(SFML_DIR "C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/lib/cmake/SFML")
	set(SFML_INCLUDE_DIR "C:/Users/czcz2/Google Drive/MIT/GCITR/SFML/SFML-2.5.1-64/include")
	

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
	# target_link_libraries(PythonCBS ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
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
		TARGET_LINK_LIBRARIES(PythonCBS ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})


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
#
#	#Print find result. Make its correct
#	message(STATUS ${PYTHON_INCLUDE_DIRS})
#	message(STATUS ${BOOST_PYTHON_INCLUDE_DIR})
#	message(STATUS ${BOOST_PYTHON})

	target_link_libraries(PythonCBS ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${BOOST_PYTHON})

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

		TARGET_LINK_LIBRARIES(PythonCBS ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})




	ELSEIF(NOT Boost_FOUND)
		MESSAGE(FATAL_ERROR "Unable to find correct Boost version. Did you set BOOST_ROOT?")
	ENDIF()



endif()
