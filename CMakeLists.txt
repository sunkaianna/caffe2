cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(Caffe2 CXX C)

# TODO(bwasti): versioning

# We want CMake to GLOB everything every time.
execute_process(COMMAND find "${PROJECT_SOURCE_DIR}" -name "CMakeLists.txt" -exec touch {} \;)

# Useful functions.
function (exclude OUTPUT INPUT)
set(EXCLUDES ${ARGN})
foreach(EXCLUDE ${EXCLUDES})
        list(REMOVE_ITEM INPUT "${EXCLUDE}")
endforeach()
set(${OUTPUT} ${INPUT} PARENT_SCOPE)
endfunction(exclude)

function (prepend OUTPUT PREPEND)
set(OUT "")
foreach(ITEM ${ARGN})
        list(APPEND OUT "${PREPEND}${ITEM}")
endforeach()
set(${OUTPUT} ${OUT} PARENT_SCOPE)
endfunction(prepend)

# ---[ CMake scripts + modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

enable_testing()

# External projects
include(ExternalProject)

include(cmake/Utils.cmake)
include(cmake/Summary.cmake)

# options
option(USE_THREADS "Use Threads" ON)
option(USE_NERVANA_GPU "Use Nervana GPU backend" OFF)
option(USE_GLOG "Use GLOG" ON)
option(USE_GFLAGS "Use GFLAGS" ON)
option(USE_LMDB "Use LMDB" ON)
option(USE_LEVELDB "Use LMDB" ON)
option(USE_OPENCV "Use openCV" ON)
option(USE_CUDA "Use Cuda" ON)
option(USE_ZMQ "Use ZMQ" OFF)
option(USE_ROCKSDB "Use RocksDB" ON)
option(USE_REDIS "Use Redis" OFF)
option(USE_MPI "Use MPI" ON)
option(BUILD_SHARED_LIBS "Build libcaffe2.so" ON)
option(USE_OPENMP "Use OpenMP for parallel code" ON)
option(BUILD_PYTHON "Build python binaries" ON)

# options that do not affect the main binaries, but affects testing binaries
option(BUILD_TEST "Build C++ test binaries (need gtest and gbenchmark)" ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not set - defaulting to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build from: Debug Release RelWithDebInfo MinSizeRel Coverage." FORCE)
endif()

# ---[ Dependencies
include(cmake/Dependencies.cmake)

# ---[ Misc checks to cope with various compiler modes
include(cmake/MiscCheck.cmake)

# ---[ Set link flag, handle additional deps for gcc 5.x
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0.0)
  message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION}: Adding gcc and gcc_s libs to link line")
  list(APPEND Caffe2_DEPENDENCY_LIBS gcc_s gcc)
endif()

# ---[ Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "binaries")

# ---[ Build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")

# ---[ Include path needed for proto
include_directories(BEFORE ${CMAKE_BINARY_DIR})

# ---[ Third party builds.
include_directories(${CMAKE_SOURCE_DIR})

# ---[ Old caffe protobuf.
add_subdirectory(caffe/proto)

# ---[ Main build
add_subdirectory(caffe2)

# ---[ Copy all python files to build directory
if (BUILD_PYTHON)
  # ---[ Generate and create all needed __init__.py files as they aren't present in source tree
  set(Caffe2_INIT_PY
          ${CMAKE_BINARY_DIR}/caffe2/__init__.py
          ${CMAKE_BINARY_DIR}/caffe2/python/__init__.py
          ${CMAKE_BINARY_DIR}/caffe2/python/mint/static/__init__.py
          ${CMAKE_BINARY_DIR}/caffe2/python/mint/static/css/__init__.py
          ${CMAKE_BINARY_DIR}/caffe2/python/mint/templates/__init__.py
          ${CMAKE_BINARY_DIR}/caffe2/python/mint/__init__.py
          ${CMAKE_BINARY_DIR}/caffe2/proto/__init__.py
          ${CMAKE_BINARY_DIR}/caffe/__init__.py
          ${CMAKE_BINARY_DIR}/caffe/proto/__init__.py
  )
  foreach(init ${Caffe2_INIT_PY})
      # create each __init__.py file in build directory
      file(WRITE ${init} "")
  endforeach()

  # Copy the files
  message(STATUS ${CMAKE_BINARY_DIR})
  # Pick up static python files
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/caffe2 DESTINATION ${CMAKE_INSTALL_PREFIX}
          FILES_MATCHING PATTERN "*.py")
  # Caffe proto files
  install(DIRECTORY ${CMAKE_BINARY_DIR}/caffe DESTINATION ${CMAKE_INSTALL_PREFIX}
          FILES_MATCHING PATTERN "*.py")
  # Caffe2 proto files
  install(DIRECTORY ${CMAKE_BINARY_DIR}/caffe2 DESTINATION ${CMAKE_INSTALL_PREFIX}
          FILES_MATCHING PATTERN "*.py")
endif()

Caffe2_print_configuration_summary()
