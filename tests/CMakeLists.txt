# Unit tests. Add a test_<something>.cpp file per part of the emulator.
add_executable(risv_tests
    test_memory.cpp
)
target_link_libraries(risv_tests PRIVATE risv_core Catch2::Catch2WithMain)
risv_setup_target(risv_tests)

# Registers every TEST_CASE with CTest, so each one shows up separately in
# Visual Studio's Test Explorer and in `ctest`.
include(Catch)
catch_discover_tests(risv_tests)
