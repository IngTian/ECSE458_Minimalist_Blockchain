# MinimalistBlockChainSystem

A modular blockchain implementation in C with a modern CMake build system.

## 📁 Project Structure

```
MinimalistBlockChainSystem/
├── CMakeLists.txt              # Root CMake configuration
├── build.sh                    # Modern build script
├── generate_coverage_report.sh # Test coverage script
├── README.md                   # This file
├── cmake/                      # CMake modules and configuration
│   ├── Dependencies.cmake      # External dependency configuration
│   └── BuildConfig.cmake      # Build type and compiler settings
├── src/                        # Source code organized by modules
│   ├── main.c                 # Main application entry point
│   ├── utils/                 # Core utilities library
│   │   ├── CMakeLists.txt     # Utils module build configuration
│   │   ├── cryptography.{c,h} # Cryptographic functions (secp256k1)
│   │   ├── log_utils.{c,h}    # Logging utilities
│   │   ├── mysql_util.{c,h}   # Database utilities
│   │   ├── socket_util.{c,h}  # Network utilities
│   │   ├── sys_utils.{c,h}    # System utilities
│   │   ├── mjson.{c,h}        # JSON parsing
│   │   └── constants.h        # Global constants
│   ├── model/                 # Blockchain data models
│   │   ├── CMakeLists.txt     # Models module build configuration
│   │   ├── block/             # Block-related code
│   │   │   ├── CMakeLists.txt
│   │   │   ├── block.{c,h}
│   │   │   └── block_persistence.{c,h}
│   │   └── transaction/       # Transaction-related code
│   │       ├── CMakeLists.txt
│   │       ├── transaction.{c,h}
│   │       └── transaction_persistence.{c,h}
│   ├── cli/                   # Command-line interface
│   │   ├── CMakeLists.txt     # CLI module build configuration
│   │   ├── interpreter.{c,h}  # Command interpreter
│   │   └── shell.{c,h}        # Interactive shell
│   └── socket/                # Network executables source
│       ├── CMakeLists.txt     # Socket module (executables only)
│       ├── client.c           # Network client
│       └── server.c           # Network server
├── apps/                      # Application executables
│   └── CMakeLists.txt         # Executable build configuration
└── test/                      # Unit tests
    ├── CMakeLists.txt         # Test configuration
    └── model/
        ├── transaction_test.c
        └── block_test.c
```

## 🏗️ Build System Architecture

### Modular Design

The project uses a modern, modular CMake structure with the following benefits:

- **Separation of Concerns**: Each module has its own CMakeLists.txt
- **Clean Dependencies**: Clear dependency relationships between modules
- **Scalability**: Easy to add new modules or modify existing ones
- **Modern CMake**: Uses target-based approach with imported targets

### Module Hierarchy

```
BlockChain::Utils (libBlockChainUtils.a)
    ├── Provides: Cryptography, logging, JSON, database, socket utilities
    └── Dependencies: MySQL::Client, GLib::GLib, secp256k1::secp256k1

BlockChain::Models (libBlockChainModels.a)
    ├── Provides: Block and transaction data structures
    └── Dependencies: BlockChain::Utils + external libraries

BlockChain::CLI (libCliModule.a)
    ├── Provides: Command-line interface and interpreter
    └── Dependencies: BlockChain::Utils, BlockChain::Models + external libraries

Applications (executables)
    ├── main, server, client, shell
    └── Dependencies: All libraries + external libraries
```

## 🚀 Quick Start

### Prerequisites

- CMake 3.16 or later
- C11 compatible compiler (GCC/Clang)
- pkg-config
- Required libraries (auto-detected):
  - MySQL client library (`mysqlclient`)
  - GLib 2.0 (`glib-2.0`)
  - secp256k1 cryptographic library
  - Check testing framework (optional, for tests)

#### macOS Installation

```bash
# Install dependencies using Homebrew
brew install cmake mysql glib secp256k1 check
```

### Building the Project

#### Option 1: Using the Modern Build Script (Recommended)

```bash
# Clean debug build with tests
./build.sh -c

# Release build
./build.sh -t Release

# Clean release build without tests
./build.sh -c -t Release --no-tests

# Verbose build with installation
./build.sh -v -i

# Show help
./build.sh -h
```

#### Option 2: Manual CMake

```bash
# Create and configure build directory
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..

# Build
cmake --build .

# Install (optional)
cmake --install .
```

### Running the Applications

After building, executables are located in `build/bin/`:

```bash
# Main blockchain application
./build/bin/main

# Interactive shell (macOS only)
./build/bin/shell

# Network server
./build/bin/server

# Network client
./build/bin/client
```

### Running Tests

```bash
# Run all tests
cd build && ctest

# Run specific tests
./build/test/test_transaction
./build/test/test_block
```

## 🔧 Build Configuration

### Build Types

- **Debug**: Development build with debugging symbols (`-g -O0`)
- **Release**: Optimized build for production (`-O3 -DNDEBUG`)
- **RelWithDebInfo**: Optimized build with debug info (`-O2 -g`)

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTING` | `ON` | Build unit tests |
| `BUILD_DOCS` | `OFF` | Build documentation |
| `ENABLE_COVERAGE` | `ON` | Enable test coverage |

### Example Configurations

```bash
# Production build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF ..

# Development with coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..

# Custom install directory
cmake -DCMAKE_INSTALL_PREFIX=/opt/blockchain ..
```

## 📚 Module Documentation

### Utils Module (`BlockChain::Utils`)

Core utilities providing foundational functionality:

- **Cryptography**: secp256k1 private/public key generation, signing
- **Database**: MySQL connection and query utilities
- **Logging**: Structured logging with different severity levels
- **JSON**: Lightweight JSON parsing and generation
- **Networking**: Socket utilities for client-server communication
- **System**: Timestamp and system-level utilities

### Models Module (`BlockChain::Models`)

Blockchain data structures and persistence:

- **Block**: Blockchain block structure and operations
- **Transaction**: Transaction structure with inputs/outputs
- **Persistence**: Database storage and retrieval for blocks and transactions

### CLI Module (`BlockChain::CLI`)

Interactive command-line interface:

- **Interpreter**: Command parsing and execution
- **Shell**: Interactive REPL for blockchain operations

## 🔍 Dependencies

### External Libraries

| Library | Purpose | Version |
|---------|---------|---------|
| MySQL Client | Database persistence | 8.0+ |
| GLib | Data structures and utilities | 2.0+ |
| secp256k1 | Elliptic curve cryptography | Latest |
| Check | Unit testing framework | Latest |

### Dependency Management

Dependencies are automatically discovered and configured through:

1. **pkg-config**: For MySQL and GLib
2. **find_library**: For secp256k1 and Check
3. **Imported Targets**: Modern CMake target-based linking

## 🧪 Testing

The project includes comprehensive unit tests using the Check framework:

- `test_transaction`: Transaction creation, validation, and serialization
- `test_block`: Block creation, mining, and validation

### Test Coverage

Generate coverage reports (macOS):

```bash
./generate_coverage_report.sh
```

## 🚀 Deployment

### Installation

```bash
# System-wide installation
sudo cmake --install build --prefix /usr/local

# User installation
cmake --install build --prefix ~/.local
```

### Distribution

Create distribution packages:

```bash
# Create tarball
cmake --build build --target package

# Create installer (if configured)
cpack
```

## 🔧 Development

### Adding New Modules

1. Create module directory under `src/`
2. Add `CMakeLists.txt` with library definition
3. Update root `CMakeLists.txt` to add subdirectory
4. Define dependencies and exported targets

### Code Style

- C11 standard compliance
- Consistent naming conventions
- Comprehensive error handling
- Memory management best practices

### Debugging

Debug builds include:

- Full debugging symbols (`-g`)
- No optimization (`-O0`)
- Additional compiler warnings
- AddressSanitizer support (optional)

## 📄 License

[Add license information here]

## 🤝 Contributing

[Add contribution guidelines here]

## 📞 Support

[Add support information here]
> Authors: [@Junjian Chen](https://github.com/JoeyChen-95), [@Jiaxiang E](https://github.com/JiaxiangE), [@Ing Tian](https://github.com/IngTian), [@Shichang Zhang](https://github.com/Shichang-Zhang)
> Date: 2022-09-22
---
## Abstract
In this project, we will implement a minimalist blockchain system mimicking the original Bitcoin design for scalability study, as laid out in Bitcoin: A Peer-to-Peer Electronic Cash System. Furthermore, we will investigate how our model can scale under different hardware and software settings, e.g., with nearly unlimited RAM, powerful GPU, etc.

This project can be fruitful because it may provide insights into how to scale a blockchain system effectively and economically and thus foresee the bottlenecks and opportunities for blockchain technology.

Regarding deliverables, we will first design and implement a minimalist blockchain system suitable for scalability study, i.e., configurable at criteria where we wish to investigate. Then, we will see how the system may respond to nearly infinite RAM. Next, we will learn how a more powerful computing unit, e.g., GPU, affects its performance. After that, we will deploy our system on a global scale to identify if telecommunication is a bottleneck. Finally, we may investigate in conjunction with the price trends of various hardware to spot
possible opportunities.