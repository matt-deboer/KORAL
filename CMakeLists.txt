cmake_minimum_required(VERSION 3.2.0)
project(koral)

FIND_PACKAGE(CUDA REQUIRED)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -std=c++11 -march=native -ftracer -ftree-vectorize -mavx2 -mfma -fomit-frame-pointer")
SET(CUDA_NVCC_FLAGS "-arch=sm_61 --use_fast_math -O3" CACHE STRING "nvcc flags" FORCE)
SET (CUDA_VERBOSE_BUILD ON CACHE BOOL "nvcc verbose" FORCE)
SET(LIB_TYPE STATIC) 
# include_directories(include)

CUDA_ADD_LIBRARY(koral ${LIB_TYPE} src/CUDALERP.cu src/CLATCH.cu src/CUDAK2NN.cu)
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
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/koral)

install(TARGETS koral
    EXPORT koral
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(EXPORT koral DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(TARGETS koral DESTINATION lib)
# install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# install(koral-targets
#     FILE
#         koralTargets.cmake
#     NAMESPACE
#         koral::
#     DESTINATION
#         ${INSTALL_CONFIGDIR}
# )

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/koral-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/koral-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/koral-config.cmake
    # ${CMAKE_CURRENT_BINARY_DIR}/koralConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# export(EXPORT koral-targets
#     FILE ${CMAKE_CURRENT_BINARY_DIR}/koralTargets.cmake
#     NAMESPACE koral::)

# export(PACKAGE koral)

enable_testing()
add_subdirectory(test)