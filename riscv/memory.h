#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <fstream>

// The emulator's RAM: a flat array of bytes.
//
// RISC-V is little-endian: the byte at the lowest address holds the lowest
// 8 bits of a 16- or 32-bit value.
//
// Any access that touches a byte outside memory throws std::out_of_range.
class Memory {
public:
    // Creates `size` bytes of memory, all zero.
    explicit Memory(std::size_t size);

    // Copies a raw binary file into memory, starting at `addr`.
    // Throws std::runtime_error if the file can't be read,
    // and std::out_of_range if it doesn't fit.
    void load(const std::filesystem::path& path, std::uint32_t addr = 0);

    std::uint8_t  read8(std::uint32_t addr) const;
    std::uint16_t read16(std::uint32_t addr) const;
    std::uint32_t read32(std::uint32_t addr) const;

    void write8(std::uint32_t addr, std::uint8_t value);
    void write16(std::uint32_t addr, std::uint16_t value);
    void write32(std::uint32_t addr, std::uint32_t value);

private:
    std::vector<std::uint8_t> bytes_;
};
