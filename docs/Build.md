# Build Overview

This guide describes how to configure, compile, test, benchmark, install, and troubleshoot the SysCore systems library.

- **Supported Platforms**: Ubuntu Linux, macOS.
- **Required Compiler Support**: GCC 9+, Clang 10+, or AppleClang 12+ supporting the C17 standard.
- **Build System**: CMake (minimum version 3.15).
- **Test Framework**: CTest.

# Prerequisites

Ensure the following tools are installed and available on your system path before configuring the project:

- **C17 Compiler**: GCC, Clang, or AppleClang.
- **CMake**: Version 3.15 or later.
- **Git**: Any modern version for cloning the repository.
- **Make/Ninja**: Default system build generators.

# Repository Setup

To check out the source code and prepare the build workspace, run the following commands:

```bash
git clone https://github.com/Sarath-Patti/SysCore.git
cd SysCore
mkdir -p build
```

# Building

SysCore uses CMake out-of-source builds to keep the repository source files isolated from compilation artifacts.

### Debug Build
A Debug configuration compiles code with debug symbols (`-g`) and without compiler optimization flags to simplify debugging:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Release Build
A Release configuration compiles code with optimization flags (typically `-O3`) to ensure peak runtime efficiency:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Clean Rebuild
To start a fresh compilation from scratch, purge the build cache and rebuild:
```bash
rm -rf build
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Alternatively, trigger a target rebuild via CMake:
```bash
cmake --build build --clean-first
```

# Running Tests

SysCore uses CMake CTest to manage and run its automated unit test suite.

### Running All Tests
To execute every unit test in the test suite:
```bash
ctest --test-dir build
```

### Running Individual Tests
To filter and execute a specific test by matching its name pattern:
```bash
ctest --test-dir build -R test_common
```

### Verbose Output
To output complete log information and assertions text for debugging:
```bash
ctest --test-dir build --verbose
```

# Running Benchmarks

Latency benchmark binaries are configured and compiled automatically during the standard build step.

### Purpose of Benchmarks
The benchmarks measure the latency overhead of core POSIX operations wrapped by the library:
- `thread_latency`: Thread spawn and join duration.
- `pipe_latency`: Anonymous pipe write/read round-trip timing.
- `semaphore_latency`: Unnamed semaphore wait/post execution delay.
- `mmap_latency`: Virtual memory page mapping and unmapping rates.

### Execution
Run the compiled benchmark executables directly from the build output directory:
```bash
./build/thread_latency
./build/pipe_latency
./build/semaphore_latency
./build/mmap_latency
```

# Installing

SysCore supports standard CMake target installation.

### CMake Install
To install public headers and static library files to the system prefix:
```bash
cmake --install build --prefix /usr/local
```

### Install Prefix
The destination directory is specified via the `--prefix` argument during installation, or by setting `CMAKE_INSTALL_PREFIX` at configuration time:
```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/syscore
```

### Exported Package Configuration
The installation registers targets and writes configuration files (`SysCoreConfig.cmake` and `SysCoreConfigVersion.cmake`) to:
```
<prefix>/lib/cmake/SysCore/
```

### Using SysCore in Another CMake Project
Once installed, other CMake configurations can search for and link to the SysCore static libraries:
```cmake
find_package(SysCore REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE SysCore::syscore_common SysCore::syscore_process)
```

# Build Options

The project currently uses the default compiler and warnings configurations. Standard CMake parameters are supported:

| Parameter | Type | Default Value | Description |
|---|---|---|---|
| `CMAKE_BUILD_TYPE` | String | None | Compilation configuration (e.g. `Debug`, `Release`). |
| `CMAKE_INSTALL_PREFIX` | Path | Platform Dependent | Target directory prefix for header and library installation. |

# Directory Layout

Summary of the repository layout:

- `include/`: Holds all public subsystem C header files.
- `src/`: Contains source code implementation files.
- `examples/`: Code examples illustrating functional usage.
- `tests/`: Automated unit test cases.
- `benchmarks/`: Latency testing programs.
- `docs/`: Design documents and markdown guides.
- `build/`: Target compilation output directory.

# Troubleshooting

### Compiler Not Found
- **Symptom**: CMake configuration fails reporting that the C compiler is missing.
- **Solution**: Set the environment variables `CC` and `CXX` to your compiler paths before running CMake, or verify that GCC or Clang is installed:
  ```bash
  export CC=gcc
  cmake -S . -B build
  ```

### Missing Pthread Library
- **Symptom**: Linker failures referencing undefined symbols for threads.
- **Solution**: Ensure your Unix system has the POSIX threads libraries installed. CMake handles standard library linking via `find_package(Threads)`.

### Permission Errors
- **Symptom**: Permission denied when running `cmake --install`.
- **Solution**: Use `sudo` if installing to protected system paths (such as `/usr/local`), or change the installation destination:
  ```bash
  sudo cmake --install build
  ```

### Unsupported Compiler
- **Symptom**: Compiler warnings or errors regarding C17 features.
- **Solution**: Upgrade your compiler toolchain to a version that supports C17 (GCC 9+ or Clang 10+).

### CMake Cache Issues
- **Symptom**: Compilation targets fail to update after changes to CMake configurations.
- **Solution**: Clear out the build cache directory and trigger compilation again to force a clean configuration.

### macOS Platform Differences
- **Symptom**: Deprecation warnings regarding semaphores.
- **Solution**: SysCore contains custom fallback logic to handle Apple's unnamed semaphore deprecation warnings internally. Ensure you are using the SysCore semaphores abstraction rather than calling POSIX `sem_*` functions directly.

# Best Practices

- **Out-of-Source Builds**: Always compile targets inside a separate directory (e.g., `build/`) to keep the source tree clean.
- **Release Builds for Benchmarks**: Compile with `-DCMAKE_BUILD_TYPE=Release` before running benchmarks to ensure compiler optimizations are active.
- **Run Tests Regularly**: Execute the CTest suite after major changes to confirm regression-free code.
- **Keep Generated Files out of Git**: Ensure your compiler outputs and build configurations are added to `.gitignore`.
