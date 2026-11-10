cmake_minimum_required(VERSION 3.25)

project(riscv LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Language standard: C++23 everywhere (Catch2 too, so both sides agree).
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# AddressSanitizer in Debug builds: catches out-of-bounds access, use after
# free and leaks at runtime, standing in for Rust's safety checks.
# ---------------------------------------------------------------------------
option(RISCV_ASAN "Build Debug configurations with AddressSanitizer" ON)

if(MSVC)
    if(RISCV_ASAN)
        # ASan doesn't work together with these MSVC Debug defaults:
        # runtime checks (/RTC1) and incremental linking.
        string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO"
               CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
        add_compile_options($<$<CONFIG:Debug>:/fsanitize=address>)

        # The ASan runtime is a DLL that sits next to cl.exe. It gets copied
        # next to each program (see riscv_setup_target) so the programs start
        # from Visual Studio, the command line and the test runner alike.
        get_filename_component(_cl_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        file(GLOB RISCV_ASAN_RUNTIME "${_cl_dir}/clang_rt.asan*.dll")
        if(NOT RISCV_ASAN_RUNTIME)
            message(WARNING "ASan runtime DLL not found next to ${CMAKE_CXX_COMPILER}. "
                            "Install the 'C++ AddressSanitizer' component in the Visual Studio "
                            "Installer, or configure with -DRISCV_ASAN=OFF.")
        endif()
    else()
        # Visual Studio Hot Reload (from the original template). It can't be
        # combined with ASan, so it's only used when ASan is off.
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
            "$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>")
    endif()
elseif(RISCV_ASAN)
    # GCC and Clang (e.g. under WSL) also get the undefined-behavior sanitizer.
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>
                        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
endif()

# ---------------------------------------------------------------------------
# Strict warnings, for our own code only (not for Catch2).
# ---------------------------------------------------------------------------
add_library(riscv_warnings INTERFACE)
if(MSVC)
    target_compile_options(riscv_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(riscv_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow)
endif()

# Call this on every target we define.
function(riscv_setup_target target)
    target_link_libraries(${target} PRIVATE riscv_warnings)

    get_target_property(_type ${target} TYPE)
    if(_type STREQUAL "EXECUTABLE" AND RISCV_ASAN_RUNTIME)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${RISCV_ASAN_RUNTIME} $<TARGET_FILE_DIR:${target}>
            VERBATIM)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Catch2, the unit test framework. Downloaded once when CMake configures.
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(Catch2
    URL    https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.1.tar.gz
    SYSTEM # treat its headers as external, so our warning flags skip them
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

enable_testing()

# ---------------------------------------------------------------------------
# Sub-projects
# ---------------------------------------------------------------------------
add_subdirectory(riscv)   # the emulator library and the riscv program
add_subdirectory(samples) # generate a raw RISC-V sample program
add_subdirectory(tests)  # unit tests
