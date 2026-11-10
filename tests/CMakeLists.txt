# Unit tests. Add a test_<something>.cpp file per part of the emulator.
add_executable(riscv_tests
    test_memory.cpp
    test_cpu.cpp
)
target_link_libraries(riscv_tests PRIVATE riscv_core Catch2::Catch2WithMain)
riscv_setup_target(riscv_tests)

# Registers every TEST_CASE with CTest, so each one shows up separately in
# Visual Studio's Test Explorer and in `ctest`.
include(Catch)
catch_discover_tests(riscv_tests)
