#include "risv/memory.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>

// These tests are the specification for Memory. They all fail until you
// implement risv/memory.cpp.

TEST_CASE("new memory is all zeros") {
    Memory mem(16);
    CHECK(mem.read32(0) == 0u);
    CHECK(mem.read32(12) == 0u);
}

TEST_CASE("write32 stores the lowest byte first (little-endian)") {
    Memory mem(16);
    mem.write32(0, 0x12345678u);
    CHECK(mem.read8(0) == 0x78);
    CHECK(mem.read8(1) == 0x56);
    CHECK(mem.read8(2) == 0x34);
    CHECK(mem.read8(3) == 0x12);
}

TEST_CASE("read32 builds a word from four bytes, lowest address = lowest bits") {
    Memory mem(16);
    mem.write8(4, 0xEF);
    mem.write8(5, 0xBE);
    mem.write8(6, 0xAD);
    mem.write8(7, 0xDE);
    CHECK(mem.read32(4) == 0xDEADBEEFu);
}

TEST_CASE("16-bit reads and writes are little-endian too") {
    Memory mem(16);
    mem.write16(2, 0xABCD);
    CHECK(mem.read8(2) == 0xCD);
    CHECK(mem.read8(3) == 0xAB);
    CHECK(mem.read16(2) == 0xABCD);
}

TEST_CASE("a write only changes its own bytes") {
    Memory mem(16);
    mem.write32(0, 0xFFFFFFFFu);
    mem.write32(8, 0xFFFFFFFFu);
    mem.write32(4, 0x01020304u);
    CHECK(mem.read32(0) == 0xFFFFFFFFu);
    CHECK(mem.read32(4) == 0x01020304u);
    CHECK(mem.read32(8) == 0xFFFFFFFFu);
}

TEST_CASE("accesses outside memory throw std::out_of_range") {
    Memory mem(16);
    CHECK_THROWS_AS(mem.read8(16), std::out_of_range);
    CHECK_THROWS_AS(mem.write8(16, 0), std::out_of_range);
    // A value that starts inside memory but ends outside is also out of range.
    CHECK_THROWS_AS(mem.read16(15), std::out_of_range);
    CHECK_THROWS_AS(mem.read32(14), std::out_of_range);
    CHECK_THROWS_AS(mem.write32(13, 0), std::out_of_range);
}

TEST_CASE("load copies a file's bytes into memory at the given address") {
    const auto path = std::filesystem::temp_directory_path() / "risv_test_program.bin";
    {
        // 0x00000013 is `addi x0, x0, 0`, the RISC-V "nop" instruction.
        const std::array<char, 4> nop{0x13, 0x00, 0x00, 0x00};
        std::ofstream out(path, std::ios::binary);
        out.write(nop.data(), static_cast<std::streamsize>(nop.size()));
    }

    Memory mem(16);
    mem.load(path, 8);
    std::filesystem::remove(path);

    CHECK(mem.read32(8) == 0x00000013u);
    CHECK(mem.read32(4) == 0u);  // bytes before the load address are untouched
}

TEST_CASE("load reports problems") {
    Memory mem(4);

    SECTION("a missing file throws std::runtime_error") {
        CHECK_THROWS_AS(mem.load("this_file_does_not_exist.bin"), std::runtime_error);
    }

    SECTION("a file that doesn't fit throws std::out_of_range") {
        const auto path = std::filesystem::temp_directory_path() / "risv_test_too_big.bin";
        {
            const std::array<char, 8> eight_bytes{};
            std::ofstream out(path, std::ios::binary);
            out.write(eight_bytes.data(), static_cast<std::streamsize>(eight_bytes.size()));
        }
        CHECK_THROWS_AS(mem.load(path), std::out_of_range);
        std::filesystem::remove(path);
    }
}
