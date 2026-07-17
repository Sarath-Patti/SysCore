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

