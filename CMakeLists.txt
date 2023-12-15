cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(radiochat C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

# Initialize the SDK
pico_sdk_init()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/RadioLib" "${CMAKE_CURRENT_BINARY_DIR}/RadioLib")

add_executable(${PROJECT_NAME}
        radiochat.cpp
        tcpip.cpp
        dnsserver/dnsserver.c
        dhcpserver/dhcpserver.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_divider
        hardware_spi
        hardware_gpio
        pico_multicore
        pico_bootsel_via_double_reset
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        RadioLib
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})