cmake_minimum_required(VERSION 3.2.0)
project(koral)

find_package(CUDA REQUIRED)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -std=c++11 -march=native -ftracer -ftree-vectorize -mavx2 -mfma -fomit-frame-pointer")
set(CUDA_NVCC_FLAGS "-arch=sm_61 --use_fast_math -O3" CACHE STRING "nvcc flags" FORCE)
set(CUDA_VERBOSE_BUILD ON CACHE BOOL "nvcc verbose" FORCE)
set(LIB_TYPE STATIC) 
# include_directories(include)

cuda_add_library(koral ${LIB_TYPE} src/CUDALERP.cu src/CLATCH.cu src/CUDAK2NN.cu)
#Set target properties
target_include_directories(koral
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# target_compile_features(koral PRIVATE cxx_11)
target_compile_options(koral PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Ofast -std=c++11 -march=native -ftracer -ftree-vectorize -mavx2 -mfma -fomit-frame-pointer>)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/koral/cmake)

install(TARGETS koral
    EXPORT koralTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT koralTargets DESTINATION ${INSTALL_CONFIGDIR} FILE koral-config.cmake)
install(DIRECTORY include/koral DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

enable_testing()
add_subdirectory(test)