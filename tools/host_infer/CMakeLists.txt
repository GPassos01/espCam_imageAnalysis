cmake_minimum_required(VERSION 3.16)
project(ei_host_infer C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Raiz do SDK EI relativo a este CMake
set(EI_SDK_ROOT ${CMAKE_SOURCE_DIR}/../../src/firmware/main/edge-impulse-sdk)
set(EI_MODEL_ROOT ${CMAKE_SOURCE_DIR}/../../src/firmware/main/tflite-model)
set(EI_PORTABLE_ROOT ${CMAKE_SOURCE_DIR}/../../src/firmware/main/portable)
set(EI_MODEL_PARAMS ${CMAKE_SOURCE_DIR}/../../src/firmware/main/model-parameters)

include_directories(
    ${CMAKE_SOURCE_DIR}/../../src/firmware/main
    ${CMAKE_SOURCE_DIR}/compat
    ${EI_SDK_ROOT}
    ${EI_SDK_ROOT}/classifier
    ${EI_SDK_ROOT}/dsp
    ${EI_SDK_ROOT}/tensorflow
    ${EI_SDK_ROOT}/porting
    ${EI_SDK_ROOT}/porting/posix
    ${EI_MODEL_ROOT}
    ${EI_MODEL_PARAMS}
    ${EI_PORTABLE_ROOT}
)

file(GLOB EI_POSIX_PORT
    ${EI_SDK_ROOT}/porting/posix/*.cpp
)

file(GLOB EI_TFLM_SOURCES
    ${EI_SDK_ROOT}/tensorflow/lite/micro/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/micro/kernels/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/micro/memory_planner/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/micro/arena_allocator/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/core/api/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/core/c/*.c
    ${EI_SDK_ROOT}/tensorflow/lite/kernels/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/kernels/internal/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/kernels/internal/reference/*.cc
    ${EI_SDK_ROOT}/tensorflow/lite/kernels/internal/optimized/*.cc
)

set(EI_KISSFFT
    ${EI_SDK_ROOT}/dsp/kissfft/kiss_fftr.cpp
    ${EI_SDK_ROOT}/dsp/kissfft/kiss_fft.cpp
)

add_definitions(
    -DEI_CLASSIFIER_ALLOCATION_STATIC=1
    -DEIDSP_QUANTIZE_FILTERBANK=0
    -DEI_CLASSIFIER_TFLITE_ENABLE_ESP_NN=0
)

add_executable(ei_host_infer
    main_host.cpp
    ${EI_POSIX_PORT}
    ${EI_TFLM_SOURCES}
    ${EI_MODEL_ROOT}/tflite_learn_573176_3_compiled.cpp
    ${EI_PORTABLE_ROOT}/portable_preprocess.c
    ${EI_KISSFFT}
)

target_link_libraries(ei_host_infer m pthread)


