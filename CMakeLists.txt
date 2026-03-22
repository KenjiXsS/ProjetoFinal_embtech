# Pico Firewall Monitor - CMake Configuration
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pico SDK Configuration
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.0.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# Set board type
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(pico_firewall_monitor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(pico_firewall_monitor
    # Main application
    src/main.c
    
    # Configuration module
    src/config.c
    
    # Network monitor module
    src/network_monitor.c
    
    # Firewall module
    src/firewall.c
    
    # HTTP server module
    src/http_server.c
    
    # Web UI module
    src/web_ui.c
    
    # System monitor module
    src/system_monitor.c
)

# Program metadata
pico_set_program_name(pico_firewall_monitor "Pico Firewall Monitor")
pico_set_program_version(pico_firewall_monitor "2.0.0")

# Enable USB output, disable UART
pico_enable_stdio_uart(pico_firewall_monitor 0)
pico_enable_stdio_usb(pico_firewall_monitor 1)

# Add additional source files
target_sources(pico_firewall_monitor PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/config.c
    ${CMAKE_CURRENT_LIST_DIR}/src/network_monitor.c
    ${CMAKE_CURRENT_LIST_DIR}/src/firewall.c
    ${CMAKE_CURRENT_LIST_DIR}/src/http_server.c
    ${CMAKE_CURRENT_LIST_DIR}/src/web_ui.c
    ${CMAKE_CURRENT_LIST_DIR}/src/system_monitor.c
)

# Add include directories
target_include_directories(pico_firewall_monitor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link libraries
target_link_libraries(pico_firewall_monitor
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_flash
    hardware_watchdog
)

# Add extra outputs (UF2, HEX, etc.)
pico_add_extra_outputs(pico_firewall_monitor)

# Enable optimization for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(pico_firewall_monitor PRIVATE -O2)
endif()
