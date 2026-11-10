# This host utility writes pre-encoded RV32I instructions, so no cross compiler
# is needed to build the sample.
add_executable(build_sum build_sum.cpp)
riscv_setup_target(build_sum)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sum.bin"
    COMMAND build_sum "${CMAKE_CURRENT_BINARY_DIR}/sum.bin"
    DEPENDS build_sum
    COMMENT "Building the RISC-V sum sample"
    VERBATIM
)
add_custom_target(sample_program ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/sum.bin")
add_dependencies(riscv sample_program)
