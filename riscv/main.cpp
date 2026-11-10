#include "riscv/cpu.h"
#include "riscv/memory.h"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: riscv <program.bin>\n";
        return 1;
    }

    try {
        Memory mem(0x100000);
        mem.load(argv[1]);
        CPU cpu(mem);
        constexpr unsigned max_steps = 1'000'000;
        for (unsigned steps = 1; steps <= max_steps; ++steps) {
            if (!cpu.step()) {
                std::cout << "stopped after " << steps << " instructions.\n"
                          << "a0 (x10) = " << cpu.read_register(10) << '\n';
                return 0;
            }
        }
        std::cerr << "Error: never reached stop\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
    }
    return 1;
}
