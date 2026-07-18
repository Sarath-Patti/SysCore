cmake_minimum_required(VERSION 3.15)
project(SysCore VERSION 0.1.0 LANGUAGES C)

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



