# CMakeLists.txt para sistema de monitoramento com comparação de imagens + WiFi sniffer

# Coletar todos os arquivos fonte do Edge Impulse SDK
file(GLOB_RECURSE EI_SDK_SOURCES
    "edge-impulse-sdk/porting/espressif/*.cpp"
    "edge-impulse-sdk/porting/espressif/*.c"
)

# Coletar os arquivos do TensorFlow Lite Micro necessários
file(GLOB TFLITE_MICRO_SOURCES
    "edge-impulse-sdk/tensorflow/lite/micro/*.cc"
    "edge-impulse-sdk/tensorflow/lite/micro/*.c"
    "edge-impulse-sdk/tensorflow/lite/micro/kernels/*.cc"
    "edge-impulse-sdk/tensorflow/lite/micro/kernels/*.c"
    "edge-impulse-sdk/tensorflow/lite/micro/memory_planner/*.cc"
    "edge-impulse-sdk/tensorflow/lite/micro/memory_planner/*.c"
    "edge-impulse-sdk/tensorflow/lite/micro/arena_allocator/*.cc"
    "edge-impulse-sdk/tensorflow/lite/micro/arena_allocator/*.c"
)

# Coletar os arquivos do TensorFlow Lite Core
file(GLOB TFLITE_CORE_SOURCES
    "edge-impulse-sdk/tensorflow/lite/core/c/*.c"
    "edge-impulse-sdk/tensorflow/lite/core/c/*.cc"
    "edge-impulse-sdk/tensorflow/lite/core/api/*.cc"
    "edge-impulse-sdk/tensorflow/lite/core/api/*.c"
)

# Coletar os arquivos de kernel internos do TensorFlow Lite
file(GLOB TFLITE_KERNEL_SOURCES
    "edge-impulse-sdk/tensorflow/lite/kernels/*.cc"
    "edge-impulse-sdk/tensorflow/lite/kernels/*.c"
    "edge-impulse-sdk/tensorflow/lite/kernels/internal/*.cc"
    "edge-impulse-sdk/tensorflow/lite/kernels/internal/*.c"
    "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/*.cc"
    "edge-impulse-sdk/tensorflow/lite/kernels/internal/optimized/*.cc"
)

idf_component_register(
    SRCS
        # Arquivos principais da aplicação
        "main.c"
        "model/ei_inference.cpp"
        "portable/portable_preprocess.c"
        "model/mqtt_send.c"
        "model/init_net.c"
        "model/init_hw.c"
        "model/wifi_sniffer.c"
        "model/chip_info.c"
        "model/compare.c"
        "model/test_dataset.c"
        "components/spiffs_manager/spiffs_manager.c"
        "components/network_manager/network_manager.c"
        "components/analysis_manager/analysis_manager.c"
        # Arquivo do modelo compilado
        "tflite-model/tflite_learn_573176_3_compiled.cpp"
        # Edge Impulse SDK e TensorFlow Lite Micro
        ${EI_SDK_SOURCES}
        ${TFLITE_MICRO_SOURCES}
        ${TFLITE_CORE_SOURCES}
        ${TFLITE_KERNEL_SOURCES}
    INCLUDE_DIRS
        "."
        "model"
        "portable"
        "components/spiffs_manager"
        "components/network_manager"
        "components/analysis_manager"
        "edge-impulse-sdk"
        "model-parameters"
        "tflite-model"
        "edge-impulse-sdk/porting/espressif/esp-dsp/modules/common/include"
        "edge-impulse-sdk/porting/espressif/esp-dsp/modules/fft/include"
    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        mqtt
        driver
        esp32-camera
        esp_psram
        esp_timer
        spiffs
    PRIV_REQUIRES json spi_flash
)

# Suavizar warnings do Edge Impulse (não falhar o build por avisos do SDK)
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=uninitialized
    -Wno-error=deprecated-declarations
    -Wno-error=missing-field-initializers
    -Wno-missing-field-initializers
    -Wno-deprecated-declarations
    -Wno-error=maybe-uninitialized
    -fno-lto
    # Flags necessárias para o TensorFlow Lite Micro
    -DEI_CLASSIFIER_ALLOCATION_STATIC=1
    -DEI_TENSOR_ARENA_LOCATION=.ext_ram.bss
    -DTF_LITE_DISABLE_X86_NEON
    -DEIDSP_QUANTIZE_FILTERBANK=0
    -DEI_CLASSIFIER_TFLITE_ENABLE_ESP_NN=0
)

spiffs_create_partition_image(storage ../data)

