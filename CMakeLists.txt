cmake_minimum_required(VERSION 3.2.0)
project(koral)

FIND_PACKAGE(CUDA REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -std=c++11 -march=native -ftracer -ftree-vectorize -mavx2 -mfma -fomit-frame-pointer")
SET(CUDA_NVCC_FLAGS "-arch=sm_61 --use_fast_math -O3" CACHE STRING "nvcc flags" FORCE)
SET (CUDA_VERBOSE_BUILD ON CACHE BOOL "nvcc verbose" FORCE)
SET(LIB_TYPE STATIC) 
include_directories(include)
CUDA_ADD_LIBRARY(koral ${LIB_TYPE} src/CUDALERP.cu src/CLATCH.cu src/CUDAK2NN.cu)

enable_testing()
add_subdirectory(test)