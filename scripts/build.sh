#!/bin/bash

# Pico Firewall Monitor - Build Script
# This script automates the build process for the Pico Firewall Monitor project

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project directories
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

# Functions
print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}→ $1${NC}"
}

# Check for PICO_SDK_PATH
check_sdk() {
    if [ -z "$PICO_SDK_PATH" ]; then
        print_error "PICO_SDK_PATH environment variable is not set"
        print_info "Please set it with: export PICO_SDK_PATH=/path/to/pico-sdk"
        exit 1
    fi
    
    if [ ! -d "$PICO_SDK_PATH" ]; then
        print_error "PICO_SDK_PATH does not exist: $PICO_SDK_PATH"
        exit 1
    fi
    
    print_success "Pico SDK found at: $PICO_SDK_PATH"
}

# Clean build directory
clean() {
    print_info "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
    print_success "Build directory cleaned"
}

# Configure project
configure() {
    print_info "Configuring project..."
    
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DPICO_SDK_PATH="${PICO_SDK_PATH}" \
             -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    print_success "Project configured"
}

# Build project
build() {
    print_info "Building project..."
    
    cd "${BUILD_DIR}"
    
    # Get number of CPU cores for parallel build
    CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    cmake --build . -j${CPU_CORES}
    
    print_success "Build completed successfully"
}

# Show build information
show_info() {
    print_header "Build Information"
    
    if [ -f "${BUILD_DIR}/pico_firewall_monitor.uf2" ]; then
        print_success "UF2 file: ${BUILD_DIR}/pico_firewall_monitor.uf2"
        ls -lh "${BUILD_DIR}/pico_firewall_monitor.uf2"
    fi
    
    if [ -f "${BUILD_DIR}/pico_firewall_monitor.elf" ]; then
        print_success "ELF file: ${BUILD_DIR}/pico_firewall_monitor.elf"
        ls -lh "${BUILD_DIR}/pico_firewall_monitor.elf"
    fi
    
    if [ -f "${BUILD_DIR}/pico_firewall_monitor.bin" ]; then
        print_success "BIN file: ${BUILD_DIR}/pico_firewall_monitor.bin"
        ls -lh "${BUILD_DIR}/pico_firewall_monitor.bin"
    fi
    
    echo ""
    print_info "To flash the device:"
    echo "  1. Hold the BOOTSEL button while connecting the Pico W to USB"
    echo "  2. Copy the .uf2 file to the RPI-RP2 drive"
    echo "  3. The device will reboot automatically"
}

# Show configuration wizard
config_wizard() {
    print_header "Configuration Wizard"
    
    CONFIG_FILE="${PROJECT_DIR}/wifi_config.template"
    
    print_info "Creating Wi-Fi configuration template..."
    
    cat > "${CONFIG_FILE}" << EOF
# Pico Firewall Monitor - Wi-Fi Configuration
# Edit this file with your Wi-Fi credentials before building

WIFI_SSID="YourWiFiSSID"
WIFI_PASSWORD="YourWiFiPassword"

# Optional: Device hostname
DEVICE_HOSTNAME="pico-firewall"

# Optional: Static IP (leave empty for DHCP)
STATIC_IP=""
NETMASK=""
GATEWAY=""
DNS=""
EOF
    
    print_success "Configuration template created: ${CONFIG_FILE}"
    print_info "Edit this file with your Wi-Fi credentials"
}

# Main menu
show_menu() {
    echo ""
    print_header "Pico Firewall Monitor - Build Menu"
    echo "  1) Clean"
    echo "  2) Configure"
    echo "  3) Build"
    echo "  4) Clean & Build (Full Rebuild)"
    echo "  5) Create Config Template"
    echo "  6) Show Build Info"
    echo "  0) Exit"
    echo ""
    echo -n "Select an option: "
}

# Parse command line arguments
case "${1:-}" in
    clean)
        check_sdk
        clean
        ;;
    configure)
        check_sdk
        configure
        ;;
    build)
        check_sdk
        build
        show_info
        ;;
    rebuild)
        check_sdk
        clean
        configure
        build
        show_info
        ;;
    config)
        config_wizard
        ;;
    info)
        show_info
        ;;
    help|--help|-h)
        echo "Usage: $0 [command]"
        echo ""
        echo "Commands:"
        echo "  clean     - Clean build directory"
        echo "  configure - Configure project with CMake"
        echo "  build     - Build the project"
        echo "  rebuild   - Clean and rebuild"
        echo "  config    - Create Wi-Fi configuration template"
        echo "  info      - Show build information"
        echo "  help      - Show this help message"
        echo ""
        echo "If no command is given, an interactive menu is shown."
        ;;
    *)
        # Interactive mode
        while true; do
            show_menu
            read -r choice
            
            case $choice in
                1)
                    check_sdk
                    clean
                    ;;
                2)
                    check_sdk
                    configure
                    ;;
                3)
                    check_sdk
                    build
                    show_info
                    ;;
                4)
                    check_sdk
                    clean
                    configure
                    build
                    show_info
                    ;;
                5)
                    config_wizard
                    ;;
                6)
                    show_info
                    ;;
                0)
                    print_info "Exiting..."
                    exit 0
                    ;;
                *)
                    print_error "Invalid option"
                    ;;
            esac
        esac
        ;;
esac
