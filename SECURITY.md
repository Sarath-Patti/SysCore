# Security Policy

SysCore is an open-source C systems programming library that provides unified abstractions for POSIX platform services. Because low-level systems libraries operate directly with process memory, synchronization primitives, and operating system resources, security issues are taken seriously.

We encourage responsible disclosure of any potential vulnerabilities found in the project so that maintainers can evaluate, fix, and publish appropriate security updates.

# Supported Versions

Security updates and patches are evaluated for supported major release versions as indicated in the table below:

| Version | Supported |
|---|---|
| **1.x** | ✅ Supported |
| **< 1.0** | ❌ Unsupported |

# Reporting a Vulnerability

If you discover a security vulnerability or potential memory safety issue in SysCore, please report it responsibly.

When reporting an issue, please include as much technical context as possible:
- **SysCore Version**: Exact library release version or commit hash.
- **Operating System**: Distribution and kernel details (e.g., Ubuntu 22.04 LTS, macOS 14).
- **Compiler**: Compiler name and version (e.g., GCC 11.4, AppleClang 15.0).
- **Build Configuration**: CMake build flags and build type (`Debug` vs `Release`).
- **Steps to Reproduce**: Minimal code snippet or step-by-step instructions to reproduce the issue.
- **Expected Behavior**: Description of what should happen under safe operation.
- **Actual Behavior**: Description of the observed failure, unexpected crash, or memory anomaly.
- **Relevant Logs**: Terminal output, backtraces, or diagnostic tool logs if available.

### Reporting Channel
Please report security issues using GitHub's private vulnerability reporting feature (Security Advisories) on the repository if available, or contact the repository maintainers through private channels. For general bugs without security impact, open a standard public GitHub issue.

# What to Report

Security issues relevant to SysCore include, but are not limited to:

- **Memory Safety**: Buffer overflows, out-of-bounds reads/writes, use-after-free, or uninitialized memory reads.
- **Resource Leaks**: Memory or descriptor leaks that could lead to denial of service or resource exhaustion under load.
- **Race Conditions**: Synchronization flaws in multi-threaded or multi-process primitives that cause data corruption or deadlocks.
- **Permission Issues**: Insecure file permissions set on IPC FIFOs, shared memory objects, or named semaphores.
- **Undefined Behavior**: Compiler-driven undefined behavior with potential security implications.
- **API Safety Flaws**: Unsafe macro implementations or missing parameter checks leading to memory corruption.

# Responsible Disclosure

To ensure security issues are resolved safely:
- **Private Communication**: Avoid disclosing potential vulnerabilities publicly in public forums, social media, or open issue trackers prior to maintainer review.
- **Technical Details**: Provide sufficient technical detail to allow maintainers to replicate and verify the issue.
- **Review Window**: Allow maintainers reasonable time to investigate, address, and verify security reports before publishing details.

# Security Best Practices

Applications using SysCore should adhere to the following best practices:

- **Use Release Builds**: Compile with `-DCMAKE_BUILD_TYPE=Release` for production deployments to ensure compiler optimizations are active.
- **Check Return Codes**: Always check `syscore_error_t` return values to detect and handle runtime failures explicitly.
- **Validate Inputs**: Validate buffer capacities, path lengths, and parameter ranges before passing them to library APIs.
- **Maintain Toolchains**: Keep C compilers and build toolchains updated to receive vendor security patches.
- **Supported Compilers**: Compile code using standard C17 compliant compilers (`-std=c17`).
- **Test Thoroughly**: Execute regression tests using build sanitizers (e.g., AddressSanitizer) during application development.

# Scope

This security policy applies to:
- SysCore C library source code under `src/` and header files under `include/`.
- Project build system scripts (`CMakeLists.txt`).
- Public API interfaces and macros.
- Technical documentation where security implications or resource ownership rules are specified.

# Acknowledgements

Contributors who responsibly report valid, verified security vulnerabilities may be recognized in release notes and changelogs, at the maintainers' discretion.
