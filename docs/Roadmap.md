# Roadmap Overview

This document outlines the evolutionary history, current release state, and future engineering directions for the SysCore library. The roadmap provides transparency for users, maintainers, and contributors regarding completed milestones and planned enhancements.

### Development Philosophy
SysCore is built following a disciplined systems engineering methodology:
- **Incremental Engineering**: Functional capabilities are introduced as self-contained, modular subsystems built on top of a common error-handling foundation.
- **Cross-Platform Parity**: Every subsystem is designed to compile cleanly and operate consistently across supported operating systems (Linux and macOS) prior to release.
- **Zero-Warning Strictness**: Code quality is maintained by compiling under standard C17 with strict compiler warning flags (`-Wall -Wextra -Wpedantic -Werror`).

### Versioning Strategy
SysCore follows [Semantic Versioning](https://semver.org/) (`MAJOR.MINOR.PATCH`):
- **MAJOR**: Indicates structural changes or breaking API revisions.
- **MINOR**: Marks the addition of new modular functionality or subsystem additions in a backward-compatible manner.
- **PATCH**: Identifies backward-compatible bug fixes, portability enhancements, or documentation updates.

# Completed Milestones

The following table records the development progression of SysCore from initial inception through the current release:

| Version | Status | Description |
|---|---|---|
| **v0.1** | Completed | Project foundation, common logging infrastructure, error code mapping (`syscore_error_t`), and versioning headers. |
| **v0.2** | Completed | Process management module wrapping POSIX `fork`, `execvp`, `waitpid`, and process ID queries. |
| **v0.3** | Completed | Inter-Process Communication (IPC) module introducing anonymous pipes and named FIFOs. |
| **v0.4** | Completed | POSIX threading module providing thread creation, joining, sleep helpers, and attribute configuration. |
| **v0.5** | Completed | Synchronization module wrapping POSIX mutexes, condition variables, and read-write locks. |
| **v0.6** | Completed | Shared Memory module managing POSIX shared memory objects, virtual address mapping, and cleanup. |
| **v0.7** | Completed | Semaphore module supporting unnamed and named POSIX semaphores with internal macOS fallback emulation. |
| **v0.8** | Completed | POSIX Message Queue module with custom shared-memory and semaphore emulation for macOS support. |
| **v0.9** | Completed | Virtual Memory Mapping module wrapping POSIX `mmap`, `munmap`, `mprotect`, and `msync`. |
| **v1.0** | Completed | Stable Release v1.0.0. API consolidation, full CTest test suite, latency benchmarks, CMake installation targets, and GitHub Actions CI integration. |

# Version 1.0 Highlights

The v1.0.0 release establishes SysCore as a stable POSIX systems programming library. Key achievements in this release include:

- **Modular Architecture**: Nine decoupled subsystems providing clean C abstractions for process control, IPC, threading, synchronization, and memory management.
- **Cross-Platform Support**: Unified public APIs verified on Ubuntu Linux (GCC) and macOS (AppleClang) with transparent macOS fallback emulation for unnamed semaphores and message queues.
- **Automated Testing**: Integrated CTest test suite covering all library subsystems for regression prevention.
- **Comprehensive Documentation**: Complete documentation suite including Architecture, Build, API Reference, Examples Catalog, Portability Guide, and Project Roadmap.
- **Modern CMake Setup**: Full out-of-source CMake build scripts with target installation support (`cmake --install`) and exported package targets (`SysCoreConfig.cmake`).
- **Continuous Integration**: GitHub Actions CI pipeline executing automated builds and tests on Linux and macOS host platforms.
- **Demonstration Programs**: 21 example applications under `examples/` illustrating API patterns and lifecycle management.

# Future Roadmap

The following table summarizes planned subsystem additions and architectural enhancements for post-v1.0 releases:

| Planned Version | Planned Work | Description |
|---|---|---|
| **v1.1.0** | Socket Networking | Portable wrappers for TCP stream and UDP datagram socket creation, binding, listening, and connection management. |
| **v1.2.0** | Event Loop | Asynchronous event demultiplexer mapping to native OS notification mechanisms (`epoll` on Linux, `kqueue` on macOS). |
| **v1.3.0** | Signal Handling | Unified POSIX signal interceptor hooks and thread-safe signal notification channels. |
| **v1.4.0** | Timer Queue | Portable high-resolution timer abstractions supporting single-shot timers and recurring interval callbacks. |
| **v1.5.0** | Thread Pool | Scalable worker thread pool subsystem for scheduling concurrent application job tasks across fixed thread worker sets. |

# Development Principles

Future development of SysCore is guided by the following principles:

- **Backward Compatibility Goals**: Public APIs established in the v1.x series are intended to maintain source compatibility throughout the major release cycle whenever practical.
- **Modular Growth**: New features are introduced as standalone modules or non-breaking additions to existing headers without introducing unnecessary dependencies.
- **Documentation-First Approach**: New APIs and architectural modifications must be documented alongside implementation changes.
- **Testing Before Release**: New subsystem functionality must include corresponding unit tests in the test suite prior to release merging.
- **Cross-Platform Consistency**: Features are designed to maintain equivalent behavior on all supported platforms, utilizing internal emulation layers where OS native APIs differ.

# Contribution Philosophy

Contributions to SysCore follow a structured workflow to maintain code quality and stability:

- **Small Incremental Changes**: Changes should be submitted as focused, logical units of work to simplify review and verification.
- **Code Review**: All modifications undergo review to ensure adherence to C17 standards, clean formatting, and error-handling conventions.
- **Automated Testing**: Every contribution must pass the full automated test suite on both Linux and macOS in CI.
- **Documentation Updates**: Code changes that alter or expand public interfaces must include corresponding updates to `README.md`, `docs/API.md`, and relevant subsystem documentation.

# Release Strategy

Releases are managed through a structured lifecycle:

1. **Feature Development**: New subsystems or enhancements are developed in modular increments conforming to project coding standards.
2. **Testing & Validation**: Implementation code is validated against compiler warning flags (`-Werror`) and verified through CTest execution across Linux and macOS.
3. **Documentation Sync**: Subsystem guides, API references, and changelogs are updated to match the release capabilities.
4. **Release Tagging**: Formal releases are tagged in version control (`vX.Y.Z`) following Semantic Versioning guidelines.

# Long-Term Vision

The long-term objective of SysCore is to serve as a reliable, portable C systems programming library for Unix-like environments. By offering clean abstractions for POSIX platform services, SysCore simplifies low-level daemon development, system utilities creation, and concurrent software engineering without compromising control or performance.
