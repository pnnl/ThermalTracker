cmake_minimum_required (VERSION 3.0.0)
project (THERMALTRACKER) 

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(QtCommon)

set(COMPANY "Pacific Northwest National Laboratory")
set(COPYRIGHT "Copyright (c) 2015 Battelle Memorial Institute")
set(IDENTIFIER "com.pnnl.gov.thtracker")
set(RESOURCE_FOLDER gui/res)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CXX_OPTIMIZATION_FLAG -O0)
else()
  set(CXX_OPTIMIZATION_FLAG -O3)
endif()

set(CMAKE_CXX_FLAGS "${CXX_OPTIMIZATION_FLAG} -fPIC ${CMAKE_CXX_FLAGS}")

if(APPLE)
  set(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ ${CMAKE_EXE_LINKER_FLAGS}")
else()
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)
find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc video)

include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
include_directories(tracker gui)

subdirs(tracker gui)

