cmake_minimum_required(VERSION 3.13)

# Initialize the SDK
include(pico_sdk_import.cmake)

project(room-sensor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Specify the directories where to find the header files
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)



# Automatically add all .c files in the src directory
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.c")

# Add the source files for your project
add_executable(room-sensor
    main.c
    ${SRC_FILES}
)

set(ENABLE_CJSON_TEST Off)
set(ENABLE_CJSON_UTILS Off)

add_subdirectory(libs/cJSON)
target_include_directories(room-sensor PRIVATE include libs/cJSON)

# add nanopb
add_subdirectory(libs/nanopb)
target_include_directories(room-sensor PRIVATE include libs/nanopb)

target_link_libraries(room-sensor 
    pico_stdlib 
    hardware_i2c
    hardware_adc
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    cjson
    protobuf-nanopb
)


# Enable stdio_usb for serial output and stdio_uart for UART output if needed
pico_enable_stdio_usb(room-sensor 1)
pico_enable_stdio_uart(room-sensor 1)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(room-sensor)


