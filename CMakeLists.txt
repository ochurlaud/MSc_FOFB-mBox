cmake_minimum_required (VERSION 2.8)
project(mBox++)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")

find_package(Armadillo  REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIR})

find_package(ZeroMQ 3 REQUIRED)
include_directories(${ZEROMQ_INCLUDE_DIR})

find_package(PythonLibs 2.7 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})

find_package(PythonInterp 2.7 REQUIRED)
find_package(NumPy REQUIRED)
include_directories(${NUMPY_INCLUDE_DIR})

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)

# This is just to add colors to the output
if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Green       "${Esc}[32m")
endif()

# DUMMY_RFM_DRIVER
#      1 if on a computer which doesn't support the driver
#      0 if the computer supports the driver
#
add_definitions(-DDUMMY_DRIVER=0)
if (DUMMY_DRIVER)
    set(DUMMY_RFM_DRIVER true)
    message(STATUS "Using ${Green}dummy RFM driver${ColourReset}")
else()
    find_package(Rfm2g REQUIRED)
    include_directories(${RFM2G_INCLUDE_DIR})
    set(DUMMY_RFM_DRIVER false)
    message(STATUS "Using ${Green}real RFM driver${ColourReset}")
endif()

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/cmake/config.h.cmake ${CMAKE_SOURCE_DIR}/src/config.h )

include_directories(src)
add_subdirectory(src)
