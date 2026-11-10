#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>

constexpr std::array<std::uint32_t, 7> program = {
    0x00000513u, // addi a0, zero, 0
    0x00A00293u, // addi t0, zero, 10
    0x00550533u, // loop: add a0, a0, t0
    0xFFF28293u, // addi t0, t0, -1
    0xFE029CE3u, // bne t0, zero, loop (-8 bytes)
    0x10A02023u, // sw a0, 256(zero)
    0x00000073u, // ecall
};

int main(int argc, char* argv[]) {
    
    std::ofstream output(argv[1], std::ios::binary);
    for (const auto instruction : program) {
        for (unsigned shift = 0; shift < 32; shift += 8) {
            output.put(static_cast<char>((instruction >> shift) & 0xFFu));
        }
    }
    output.close();
    return 0;
}
