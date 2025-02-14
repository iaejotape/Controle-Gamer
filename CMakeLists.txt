# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(button_joystick)

pico_sdk_init()

add_executable(button_joystick
    button_joystick.c
)

target_link_libraries(button_joystick
    pico_stdlib
    hardware_adc
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(button_joystick)

include_directories(/path/to/tusb_config)

# Adicione o caminho para o diretório onde o tusb_config.h está localizado
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

