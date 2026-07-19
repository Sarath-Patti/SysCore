# SysCore Build & Packaging Guide

SysCore requires a standard C17 compliant compiler and CMake 3.15 or later.

## Quick Start

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Running Tests

Integrate with CTest:

```bash
cd build
ctest --output-on-failure
```

## Installation and Exports

SysCore supports standard CMake installation routines:

```bash
cmake --install build --prefix /usr/local
```

This installs public headers, static libraries, and exports `SysCoreConfig.cmake` target files for easy inclusion in other CMake projects.
