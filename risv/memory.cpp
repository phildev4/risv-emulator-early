#include "memory.h"
#include <iostream>
#include <stdexcept>


Memory::Memory( std::size_t size): bytes_(size) {

}

void Memory::load(const std::filesystem::path& path,
                   std::uint32_t addr) {
    std::fstream fs(path, std::ios::out | std::ios::binary | std::ios::app | std::ios::in);
    auto size = std::filesystem::file_size(path);

    if (!fs) throw std::runtime_error("File doesnt exist");
    if (size > bytes_.size() - addr ) throw std::out_of_range("too large");
    fs.read(reinterpret_cast<char*>(bytes_.data() + addr), static_cast<std::streamsize>(size));
}

std::uint8_t Memory::read8( std::uint32_t addr) const {
    return bytes_.at(addr);
}

std::uint16_t Memory::read16( std::uint32_t addr) const {
    return (read8(addr) | read8(addr + 1) << 8);
}

std::uint32_t Memory::read32(std::uint32_t addr) const {
    return (read16(addr) | read16(addr + 2) << 16);

}

void Memory::write8( std::uint32_t addr,
                     std::uint8_t value) {
    bytes_.at(addr) = value;

}

void Memory::write16( std::uint32_t addr,
                      std::uint16_t value) {
    Memory::write8(addr, static_cast<std::uint8_t>(value));
    Memory::write8(addr+1, static_cast<std::uint8_t>(value >> 8));

}

void Memory::write32( std::uint32_t addr,
                      std::uint32_t value) {
    Memory::write16(addr, static_cast<std::uint16_t>(value));
    Memory::write16(addr + 2, static_cast<std::uint16_t>(value >> 16));



}
