#!/bin/bash

# Modern build script for MinimalistBlockChainSystem
# Usage: ./build.sh [options]

set -e  # Exit on any error

# Default values
BUILD_TYPE="Debug"
BUILD_TESTS="ON"
BUILD_DIR="build"
CLEAN_BUILD=false
VERBOSE=false
INSTALL=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type TYPE       Build type: Debug, Release, RelWithDebInfo (default: Debug)"
    echo "  -d, --dir DIR         Build directory (default: build)"
    echo "  -c, --clean           Clean build (remove build directory first)"
    echo "  -v, --verbose         Verbose output"
    echo "  -i, --install         Install after building"
    echo "      --no-tests        Disable building tests"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Debug build with tests"
    echo "  $0 -t Release         # Release build"
    echo "  $0 -c -t Release      # Clean release build"
    echo "  $0 --no-tests -i      # Build without tests and install"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# Validate build type
case $BUILD_TYPE in
    Debug|Release|RelWithDebInfo)
        ;;
    *)
        print_error "Invalid build type: $BUILD_TYPE"
        print_info "Valid types: Debug, Release, RelWithDebInfo"
        exit 1
        ;;
esac

# Main script
print_info "Starting build process..."
print_info "Build Type: $BUILD_TYPE"
print_info "Build Directory: $BUILD_DIR"
print_info "Tests Enabled: $BUILD_TESTS"

# Check for required dependencies
print_info "Checking dependencies..."

# Check for cmake
if ! command -v cmake &> /dev/null; then
    print_error "CMake is required but not installed."
    exit 1
fi

# Check for vcpkg (since we're using vcpkg for dependency management)
if ! command -v vcpkg &> /dev/null; then
    print_error "vcpkg is required but not found in PATH."
    print_info "Please ensure vcpkg is installed and available in your PATH."
    exit 1
fi

# Check if vcpkg.json exists
if [[ ! -f "vcpkg.json" ]]; then
    print_error "vcpkg.json manifest not found."
    print_info "This project uses vcpkg for dependency management."
    exit 1
fi

# Check for pkg-config (still needed for GLib via vcpkg)
if ! command -v pkg-config &> /dev/null; then
    print_error "pkg-config is required but not installed."
    print_info "Please install pkg-config: brew install pkg-config"
    exit 1
fi

print_success "vcpkg dependency management configured"

# Clean build if requested
if [[ "$CLEAN_BUILD" == true ]]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure
print_info "Configuring project..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_TESTING="$BUILD_TESTS"
)

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cd "$BUILD_DIR"

cmake "${CMAKE_ARGS[@]}" .. || {
    print_error "Configuration failed"
    exit 1
}

print_success "Configuration completed"

# Build
print_info "Building project..."
BUILD_ARGS=()

if [[ "$VERBOSE" == true ]]; then
    BUILD_ARGS+=(--verbose)
fi

cmake --build . "${BUILD_ARGS[@]}" || {
    print_error "Build failed"
    exit 1
}

print_success "Build completed successfully"

# Install if requested
if [[ "$INSTALL" == true ]]; then
    print_info "Installing..."
    cmake --install . || {
        print_error "Installation failed"
        exit 1
    }
    print_success "Installation completed"
fi

# Show build summary
print_info "Build Summary:"
print_info "  Build Type: $BUILD_TYPE"
print_info "  Build Directory: $BUILD_DIR"
print_info "  Tests: $BUILD_TESTS"

if [[ -d "bin" ]]; then
    print_info "  Executables created in: $BUILD_DIR/bin/"
    ls -la bin/
fi

if [[ "$BUILD_TESTS" == "ON" ]] && [[ -d "test" ]]; then
    print_info "  Test executables in: $BUILD_DIR/test/"
    print_info "  Run tests with: ctest"
fi

print_success "All done! 🎉"