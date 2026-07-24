# Contributing to SysCore

Thank you for your interest in contributing to SysCore. SysCore is a modular C systems programming library designed to provide portable, clean abstractions over POSIX platform services.

Whether you are fixing a bug, adding test cases, refining documentation, or proposing new subsystem capabilities, your contributions help make SysCore a more reliable framework for developers.

# Ways to Contribute

There are several ways you can contribute to the project:

- **Reporting Bugs**: Open an issue describing reproducible steps, operating system details, compiler version, and observed vs. expected behavior.
- **Improving Documentation**: Fix typos, clarify API references, refine system diagrams, or improve guide sections.
- **Adding Tests**: Expand unit test coverage under `tests/` for edge cases or system boundary conditions.
- **Improving Examples**: Refine code samples under `examples/` to better demonstrate API patterns and resource cleanups.
- **Fixing Bugs**: Submit pull requests resolving open issues or compiler warnings.
- **Refactoring Code**: Improve modularity, memory safety, or platform abstraction routines without altering public API behaviors.
- **Adding New POSIX Modules**: Propose and implement new modular subsystem wrappers (e.g. sockets, event loop, timers).

> [!NOTE]
> For large feature additions or architectural modifications, please open an issue to discuss the design before starting implementation.

# Development Environment

To build, test, and contribute to SysCore, ensure your environment includes the following tools:

| Tool | Minimum Version | Description |
|---|---|---|
| **Git** | 2.0+ | Source code version control. |
| **CMake** | 3.15+ | Build configuration and test driver generator. |
| **C Compiler** | GCC 9+ or AppleClang 12+ | C17 compliant compiler (`-std=c17`). |
| **Make / Ninja** | Standard | Build tool generator invoked by CMake. |

### Installation Guidance

On Ubuntu / Debian systems:
```bash
sudo apt update
sudo apt install build-essential cmake git
```

On macOS (using Homebrew or Xcode Command Line Tools):
```bash
xcode-select --install
brew install cmake git
```

# Repository Setup

SysCore enforces out-of-source builds to keep build artifacts isolated from source files. Clone the repository and configure the build directory as follows:

```bash
git clone https://github.com/Sarath-Patti/SysCore.git
cd SysCore
mkdir build
cd build
cmake ..
cmake --build .
```

To configure a Debug build with debugging symbols:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

To configure a Release build for performance testing:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

# Development Workflow

Follow this step-by-step workflow when working on changes:

1. **Create a Feature Branch**:
   ```bash
   git checkout -b feature/my-contribution
   ```
2. **Apply Code Formatting**: Adhere to the formatting rules defined in `.clang-format`, `.clang-tidy`, and `.editorconfig`.
3. **Verify Warning-Free Compilation**: The build system compiles code with strict warning options (`-Wall -Wextra -Wpedantic -Werror`). Ensure your changes compile cleanly without warnings.
4. **Execute Automated Tests**: Run CTest to confirm all tests pass.
5. **Update Documentation**: Update relevant markdown files under `docs/` or `README.md` if public APIs or behaviors were modified.

# Coding Standards

All C source code in SysCore must follow these standards:

- **Language Standard**: Strict C17 (`-std=c17`). Avoid compiler-specific extensions (`-fasm`, non-standard macros).
- **Function Naming**: All public library interfaces must use the lower_snake_case format:
  ```c
  syscore_<module>_<action>()
  ```
- **Error Handling**: Functions returning operation status must return `syscore_error_t` (`SYSCORE_SUCCESS` on success, or a negative error enum on failure). Output handles or payloads must be passed via pointer parameters.
- **Header vs. Implementation Separation**: Public declarations reside strictly inside `include/<module>/`. Private structures, system header includes, and platform fallback logic must remain inside `src/<module>/`.
- **Cross-Platform Portability**: Public headers must remain platform-agnostic. Platform-specific preprocessor checks (`#if defined(...)`) must remain contained within internal `.c` files.

# Testing Guidelines

SysCore uses CMake CTest to run unit tests.

- **Adding Tests**: Place new test source files in the `tests/` directory (e.g., `tests/test_feature.c`). Include `test_helpers.h` for assertion macros.
- **CTest Registration**: Register new test binaries in the root `CMakeLists.txt` using `add_executable()` and `add_test()`.
- **Running Tests**: Run the full test suite from the `build` directory:
  ```bash
  ctest --output-on-failure
  ```
- **Multi-Platform Verification**: Verify that your test cases run cleanly on both Linux and macOS.

# Documentation Guidelines

Documentation is an integral part of SysCore.

- When creating or modifying public functions, update `docs/API.md` with function prototypes, parameter descriptions, error codes, and example usages.
- Update `README.md` and `docs/Architecture.md` if introducing a new module or structural component.
- Use standard GitHub Flavored Markdown and relative file links (e.g. `docs/API.md`) instead of hardcoded local file paths.

# Submitting Pull Requests

Before submitting a Pull Request (PR):

1. **Self-Review**: Review your code diff to ensure no scratch files, debug print calls, or unformatted code are committed.
2. **Clear Title & Description**: Write a concise PR title and describe what changed, why, and how it was verified.
3. **Reference Issues**: Link any related issue numbers (e.g., `Fixes #12`).
4. **CI Validation**: Ensure all GitHub Actions CI build matrix jobs (Ubuntu GCC and macOS AppleClang) pass cleanly.
5. **Address Feedback**: Participate constructively during code review iterations.

# Code of Conduct

All contributors are expected to adhere to the project's [Code of Conduct](CODE_OF_CONDUCT.md). Please maintain a respectful, welcoming, and collaborative environment for everyone.
