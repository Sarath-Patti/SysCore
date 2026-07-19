cmake_minimum_required(VERSION 3.15)
project(SysCore VERSION 1.0.0 LANGUAGES C)

# Use C17 standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable compiler warnings portably
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Common library target
add_library(syscore_common
    src/common/version.c
    src/common/logging.c
    src/common/errors.c
)

target_include_directories(syscore_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Demo executable target
add_executable(syscore_demo
    src/main.c
)

target_link_libraries(syscore_demo PRIVATE syscore_common)

# Process library target
add_library(syscore_process
    src/process/process.c
)

target_include_directories(syscore_process PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_process PUBLIC syscore_common)

# Process basic_fork executable target
add_executable(basic_fork
    examples/process/basic_fork.c
)

target_link_libraries(basic_fork PRIVATE syscore_process)

# IPC library target
add_library(syscore_ipc
    src/ipc/ipc.c
)

target_include_directories(syscore_ipc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_ipc PUBLIC syscore_common)

# IPC anonymous pipe demo target
add_executable(anonymous_pipe_demo
    examples/ipc/anonymous_pipe.c
)

target_link_libraries(anonymous_pipe_demo PRIVATE syscore_ipc syscore_process)

# IPC named FIFO demo target
add_executable(named_fifo_demo
    examples/ipc/named_fifo.c
)

target_link_libraries(named_fifo_demo PRIVATE syscore_ipc syscore_process)

# Find POSIX threads package
find_package(Threads REQUIRED)

# Threading library target
add_library(syscore_threading
    src/threading/threading.c
)

target_include_directories(syscore_threading PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_threading PUBLIC syscore_common Threads::Threads)

# Threading examples
add_executable(basic_thread
    examples/threading/basic_thread.c
)
target_link_libraries(basic_thread PRIVATE syscore_threading)

add_executable(thread_arguments
    examples/threading/thread_arguments.c
)
target_link_libraries(thread_arguments PRIVATE syscore_threading)

add_executable(multiple_threads
    examples/threading/multiple_threads.c
)
target_link_libraries(multiple_threads PRIVATE syscore_threading)

# Synchronization library target
add_library(syscore_sync
    src/sync/sync.c
)

target_include_directories(syscore_sync PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_sync PUBLIC syscore_common Threads::Threads)

# Synchronization examples
add_executable(mutex_demo
    examples/sync/mutex_demo.c
)
target_link_libraries(mutex_demo PRIVATE syscore_sync syscore_threading)

add_executable(producer_consumer
    examples/sync/producer_consumer.c
)
target_link_libraries(producer_consumer PRIVATE syscore_sync syscore_threading)

add_executable(read_write_lock
    examples/sync/read_write_lock.c
)
target_link_libraries(read_write_lock PRIVATE syscore_sync syscore_threading)

add_executable(condition_variable
    examples/sync/condition_variable.c
)
target_link_libraries(condition_variable PRIVATE syscore_sync syscore_threading)

# Shared Memory library target
add_library(syscore_memory
    src/memory/shared_memory.c
)

target_include_directories(syscore_memory PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_memory PUBLIC syscore_common)

# Check and link real-time library (rt) if available
find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
    target_link_libraries(syscore_memory PUBLIC ${RT_LIBRARY})
endif()

# Semaphore library target
add_library(syscore_semaphore
    src/sync/semaphore.c
)

target_include_directories(syscore_semaphore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_semaphore PUBLIC syscore_common Threads::Threads)

# Shared Memory & Semaphore examples
add_executable(shared_memory_demo
    examples/memory/shared_memory_demo.c
)
target_link_libraries(shared_memory_demo PRIVATE syscore_memory syscore_process)

add_executable(semaphore_demo
    examples/sync/semaphore_demo.c
)
target_link_libraries(semaphore_demo PRIVATE syscore_semaphore syscore_threading)

add_executable(named_semaphore_demo
    examples/sync/named_semaphore_demo.c
)
target_link_libraries(named_semaphore_demo PRIVATE syscore_semaphore syscore_process)

add_executable(shared_counter
    examples/memory/shared_counter.c
)
target_link_libraries(shared_counter PRIVATE syscore_memory syscore_semaphore syscore_process)

# Message Queue library target
add_library(syscore_mq
    src/ipc/message_queue.c
)

target_include_directories(syscore_mq PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_mq PUBLIC syscore_common syscore_memory syscore_semaphore)

if(RT_LIBRARY)
    target_link_libraries(syscore_mq PUBLIC ${RT_LIBRARY})
endif()

# Message Queue examples
add_executable(message_queue_demo
    examples/ipc/message_queue_demo.c
)
target_link_libraries(message_queue_demo PRIVATE syscore_mq syscore_process)

add_executable(message_queue_priority
    examples/ipc/message_queue_priority.c
)
target_link_libraries(message_queue_priority PRIVATE syscore_mq)

add_executable(producer_consumer_mq
    examples/ipc/producer_consumer_mq.c
)
target_link_libraries(producer_consumer_mq PRIVATE syscore_mq syscore_process)

# Memory Mapping library target
add_library(syscore_mmap
    src/memory/mmap.c
)

target_include_directories(syscore_mmap PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(syscore_mmap PUBLIC syscore_common)

# Memory Mapping examples
add_executable(mmap_anonymous_demo
    examples/memory/mmap_anonymous_demo.c
)
target_link_libraries(mmap_anonymous_demo PRIVATE syscore_mmap)

add_executable(mmap_file_demo
    examples/memory/mmap_file_demo.c
)
target_link_libraries(mmap_file_demo PRIVATE syscore_mmap)

add_executable(mmap_shared_demo
    examples/memory/mmap_shared_demo.c
)
target_link_libraries(mmap_shared_demo PRIVATE syscore_mmap syscore_process)

# List of all library targets for installation
set(SYSCORE_LIBS
    syscore_common
    syscore_process
    syscore_ipc
    syscore_threading
    syscore_sync
    syscore_memory
    syscore_semaphore
    syscore_mq
    syscore_mmap
)

# Apply INSTALL_INTERFACE to all library targets
foreach(lib IN LISTS SYSCORE_LIBS)
    target_include_directories(${lib} PUBLIC
        $<INSTALL_INTERFACE:include>
    )
endforeach()

# Install libraries and headers
install(TARGETS ${SYSCORE_LIBS}
    EXPORT SysCoreTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# Export targets file
install(EXPORT SysCoreTargets
    FILE SysCoreTargets.cmake
    NAMESPACE SysCore::
    DESTINATION lib/cmake/SysCore
)

# Generate and install package version & config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SysCoreConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/SysCoreConfig.cmake"
"include(CMakeFindDependencyMacro)
include(CMakeFindFrameworks OPTIONAL)
include(\"\${CMAKE_CURRENT_LIST_DIR}/SysCoreTargets.cmake\")
"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SysCoreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SysCoreConfigVersion.cmake"
    DESTINATION lib/cmake/SysCore
)

# Enable CTest Automated Testing
enable_testing()

add_executable(test_common tests/test_common.c)
target_link_libraries(test_common PRIVATE syscore_common)
add_test(NAME test_common COMMAND test_common)

add_executable(test_process tests/test_process.c)
target_link_libraries(test_process PRIVATE syscore_process)
add_test(NAME test_process COMMAND test_process)

add_executable(test_ipc tests/test_ipc.c)
target_link_libraries(test_ipc PRIVATE syscore_ipc)
add_test(NAME test_ipc COMMAND test_ipc)

add_executable(test_threading tests/test_threading.c)
target_link_libraries(test_threading PRIVATE syscore_threading)
add_test(NAME test_threading COMMAND test_threading)

add_executable(test_sync tests/test_sync.c)
target_link_libraries(test_sync PRIVATE syscore_sync)
add_test(NAME test_sync COMMAND test_sync)

add_executable(test_memory tests/test_memory.c)
target_link_libraries(test_memory PRIVATE syscore_memory)
add_test(NAME test_memory COMMAND test_memory)

add_executable(test_semaphore tests/test_semaphore.c)
target_link_libraries(test_semaphore PRIVATE syscore_semaphore syscore_threading)
add_test(NAME test_semaphore COMMAND test_semaphore)

add_executable(test_mq tests/test_mq.c)
target_link_libraries(test_mq PRIVATE syscore_mq)
add_test(NAME test_mq COMMAND test_mq)

add_executable(test_mmap tests/test_mmap.c)
target_link_libraries(test_mmap PRIVATE syscore_mmap)
add_test(NAME test_mmap COMMAND test_mmap)

# Latency Benchmarks
add_executable(thread_latency benchmarks/thread_latency.c)
target_link_libraries(thread_latency PRIVATE syscore_threading)

add_executable(pipe_latency benchmarks/pipe_latency.c)
target_link_libraries(pipe_latency PRIVATE syscore_ipc)

add_executable(semaphore_latency benchmarks/semaphore_latency.c)
target_link_libraries(semaphore_latency PRIVATE syscore_semaphore)

add_executable(mmap_latency benchmarks/mmap_latency.c)
target_link_libraries(mmap_latency PRIVATE syscore_mmap)







