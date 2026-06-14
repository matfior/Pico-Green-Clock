# CMakeLists.txt
# For Pico-Green-Clock
#
# Builds for both supported boards from this single file:
#   Pico W (Wi-Fi + NTP, default):  cmake -S . -B build
#   Original Pico (no Wi-Fi/NTP):   cmake -S . -B build -DPICO_BOARD=pico
#
# The PICO_W compile definition (used by #ifdef PICO_W in the source) is
# derived from PICO_BOARD below — no source edit needed to switch boards.

cmake_minimum_required(VERSION 3.13)

# Must be set before pico_sdk_import.cmake so the Pico SDK configures board support.
if(NOT PICO_BOARD AND NOT DEFINED ENV{PICO_BOARD})
  set(PICO_BOARD pico_w)
endif()

include(pico_sdk_import.cmake)

project(Pico-Green-Clock C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(Pico-Green-Clock
        Pico-Green-Clock.c
        Ds3231.c)

# -Wno-pointer-sign: the firmware deliberately stores strings as UCHAR (unsigned char) so that
# font / accented-character codes above 0x7F compare correctly; that convention clashes with the
# char* used by string literals and libc, producing hundreds of harmless -Wpointer-sign warnings.
# Disable just that one warning (C only) and keep the rest of -Wall -Wextra active.
target_compile_options(Pico-Green-Clock PRIVATE
        -Wall -Wextra
        $<$<COMPILE_LANGUAGE:C>:-Wno-pointer-sign>)

target_include_directories(Pico-Green-Clock PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

# Send Pico's output to USB and also to UART (for debug purposes).
pico_enable_stdio_uart(Pico-Green-Clock 1)
pico_enable_stdio_usb(Pico-Green-Clock 1)

# Create map/bin/hex/uf2 files etc.
pico_add_extra_outputs(Pico-Green-Clock)

target_link_libraries(Pico-Green-Clock PRIVATE
        hardware_adc
        hardware_flash
        hardware_i2c
        hardware_pwm
        hardware_sync
        pico_stdlib
        pico_unique_id
        pico_multicore)

if(PICO_BOARD STREQUAL "pico_w")
  target_sources(Pico-Green-Clock PRIVATE picow_ntp_client.c)
  target_compile_definitions(Pico-Green-Clock PRIVATE PICO_W)
  target_link_libraries(Pico-Green-Clock PRIVATE
          pico_cyw43_arch_lwip_threadsafe_background)
endif()
