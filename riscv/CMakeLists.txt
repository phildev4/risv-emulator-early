# The emulator itself, built as a library so the program and the tests share it.
# Add new emulator source files (cpu.cpp, decoder.cpp, ...) to this list.
add_library(riscv_core STATIC
    memory.cpp
    memory.h
    decoder.cpp
    decoder.h
    cpu.cpp
    cpu.h
)
target_include_directories(riscv_core PUBLIC ${PROJECT_SOURCE_DIR})
riscv_setup_target(riscv_core)

# The program you run.
add_executable(riscv
    main.cpp
)
target_link_libraries(riscv PRIVATE riscv_core)
riscv_setup_target(riscv)
