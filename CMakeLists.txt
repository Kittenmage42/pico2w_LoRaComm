cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

# Include the Pico SDK initialisation script
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_LoRaComm)

# Initialize the Pico SDK
pico_sdk_init()

# Add your executable and source files
add_executable(pico_LoRaComm
    main.cpp
    ssd1306.c
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(pico_LoRaComm 1)
pico_enable_stdio_uart(pico_LoRaComm 0)


# Link the Pico SDK to your project
target_link_libraries(pico_LoRaComm 
    pico_stdlib
    hardware_i2c
    pico_cyw43_arch_none
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico_LoRaComm)