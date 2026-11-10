#include "riscv/cpu.h"
#include "riscv/memory.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <initializer_list>
#include <stdexcept>

namespace {
    constexpr uint32_t nop = 0x00000013u; // addi x0, x0, 0
    constexpr uint32_t capture_pc_in_x3 = 0x00000197u; // auipc x3, 0

    uint32_t register_value(CPU& cpu, Memory& mem, uint32_t index) {
        mem.write32(240, 0xCCDDCCDDu);
        cpu.sw(index, 240, 0); // store register at x0 + 240
        return mem.read32(240);
    }

    constexpr uint32_t branch(uint32_t funct3, uint32_t rs1, uint32_t rs2,
        int32_t offset) {
        const uint32_t imm = static_cast<uint32_t>(offset);
        return (((imm >> 12) & 1u) << 31)
            | (((imm >> 5) & 0x3Fu) << 25)
            | (rs2 << 20) | (rs1 << 15) | (funct3 << 12)
            | (((imm >> 1) & 0xFu) << 8)
            | (((imm >> 11) & 1u) << 7) | 0x63u;
    }

    constexpr uint32_t jump(uint32_t rd, int32_t offset) {
        const uint32_t imm = static_cast<uint32_t>(offset);
        return (((imm >> 20) & 1u) << 31)
            | (((imm >> 1) & 0x3FFu) << 21)
            | (((imm >> 11) & 1u) << 20)
            | (((imm >> 12) & 0xFFu) << 12)
            | (rd << 7) | 0x6Fu;
    }

    static_assert(branch(0, 1, 2, 8) == 0x00208463u, "beq x1, x2, +8");
    static_assert(branch(0, 1, 2, -8) == 0xFE208CE3u, "beq x1, x2, -8");
    static_assert(jump(5, 8) == 0x008002EFu, "jal x5, +8");
    static_assert(jump(5, -8) == 0xFF9FF2EFu, "jal x5, -8");
} // namespace

TEST_CASE("a new CPU starts with zero registers") {
    Memory mem(256);
    CPU cpu(mem);

    for (uint32_t reg = 0; reg < 32; ++reg) {
        CAPTURE(reg);
        CHECK(register_value(cpu, mem, reg) == 0u);
    }
}

TEST_CASE("write_register updates the selected register but cannot change x0") {
    Memory mem(256);
    CPU cpu(mem);

    for (uint32_t reg = 1; reg < 32; ++reg) {
        cpu.write_register(reg, 0xA5000000u + reg);
    }
    cpu.write_register(0, 0xFFFFFFFFu);

    CHECK(register_value(cpu, mem, 0) == 0u);
    for (uint32_t reg = 1; reg < 32; ++reg) {
        CAPTURE(reg);
        CHECK(register_value(cpu, mem, reg) == 0xA5000000u + reg);
    }
}

TEST_CASE("registers can be inspected without changing CPU state") {
    Memory mem(256);
    CPU cpu(mem);
    cpu.write_register(10, 55);
    CHECK(cpu.read_register(10) == 55u);
    CHECK(cpu.read_register(0) == 0u);
    CHECK_THROWS_AS(cpu.read_register(32), std::out_of_range);
}

TEST_CASE("step returns true for arithmetic and false at ECALL or EBREAK") {
    for (uint32_t halt : {0x00000073u, 0x00100073u}) {
        CAPTURE(halt);
        Memory mem(256);
        CPU cpu(mem);
        mem.write32(0, 0x03700513u); // addi a0, zero, 55
        mem.write32(4, halt);
        mem.write32(8, 0x00100513u); // must not execute after halting
        REQUIRE(cpu.step());
        CHECK_FALSE(cpu.step());
        CHECK(cpu.read_register(10) == 55u);
        CHECK_FALSE(cpu.step()); // stays at the halt instruction
    }
}

TEST_CASE("unsupported opcodes report an error instead of aborting the host") {
    Memory mem(256);
    CPU cpu(mem);
    mem.write32(0, 0xFFFFFFFFu);
    CHECK_THROWS_AS(cpu.step(), std::runtime_error);
}

TEST_CASE("immediate arithmetic and logic produce the expected 32-bit result") {
    struct Example {
        const char* name;
        void (CPU::* execute)(uint32_t, uint32_t, int);
        uint32_t source;
        int immediate;
        uint32_t expected;
    };
    const Example examples[] = {
        {"addi positive", &CPU::addi, 10u, 5, 15u},
        {"addi negative", &CPU::addi, 10u, -15, 0xFFFFFFFBu},
        {"addi unsigned wrap", &CPU::addi, 0xFFFFFFFFu, 1, 0u},
        {"addi signed wrap", &CPU::addi, 0x7FFFFFFFu, 1, 0x80000000u},
        {"addi minimum immediate", &CPU::addi, 0u, -2048, 0xFFFFF800u},
        {"addi maximum immediate", &CPU::addi, 0u, 2047, 2047u},
        {"slti negative source", &CPU::slti, 0xFFFFFFFFu, 0, 1u},
        {"slti equality", &CPU::slti, 5u, 5, 0u},
        {"slti negative immediate", &CPU::slti, 0u, -1, 0u},
        {"slti signed minimum", &CPU::slti, 0x80000000u, -1, 1u},
        {"sltiu unsigned source", &CPU::sltiu, 0xFFFFFFFFu, 1, 0u},
        {"sltiu less", &CPU::sltiu, 4u, 5, 1u},
        {"sltiu sign-extends immediate", &CPU::sltiu, 0x80000000u, -1, 1u},
        {"sltiu equality", &CPU::sltiu, 0xFFFFFFFFu, -1, 0u},
        {"xori mask", &CPU::xori, 0x12345678u, 0xFF, 0x12345687u},
        {"xori negative immediate", &CPU::xori, 0x12345678u, -1, 0xEDCBA987u},
        {"ori mask", &CPU::ori, 0x12345678u, 0xFF, 0x123456FFu},
        {"ori negative immediate", &CPU::ori, 0u, -1, 0xFFFFFFFFu},
        {"andi mask", &CPU::andi, 0x12345678u, 0xFF, 0x78u},
        {"andi negative immediate", &CPU::andi, 0x12345678u, -1, 0x12345678u},
        {"andi clears destination", &CPU::andi, 0xFFFFFFFFu, 0, 0u},
    };

    for (const auto& example : examples) {
        // Test a distinct destination, overwriting the source, and ignoring x0.
        for (uint32_t rd : {2u, 1u, 0u}) {
            CAPTURE(example.name, rd);
            Memory mem(256);
            CPU cpu(mem);
            cpu.write_register(1, example.source);
            cpu.write_register(2, 0xA5A5A5A5u);
            cpu.write_register(31, 0x12345678u);

            (cpu.*example.execute)(rd, 1, example.immediate);

            CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : example.expected));
            CHECK(register_value(cpu, mem, 0) == 0u);
            CHECK(register_value(cpu, mem, 31) == 0x12345678u);
            if (rd != 1) CHECK(register_value(cpu, mem, 1) == example.source);
            if (rd != 2) CHECK(register_value(cpu, mem, 2) == 0xA5A5A5A5u);
        }
    }
}

TEST_CASE("immediate shifts distinguish logical and arithmetic right shifts") {
    struct Example {
        const char* name;
        void (CPU::* execute)(uint32_t, uint32_t, uint32_t);
        uint32_t source;
        uint32_t amount;
        uint32_t expected;
    };
    const Example examples[] = {
        {"slli zero", &CPU::slli, 0x80000001u, 0u, 0x80000001u},
        {"slli discards high bits", &CPU::slli, 0x80000001u, 1u, 2u},
        {"slli 31", &CPU::slli, 1u, 31u, 0x80000000u},
        {"srli zero", &CPU::srli, 0x80000001u, 0u, 0x80000001u},
        {"srli zero-fills", &CPU::srli, 0x80000000u, 1u, 0x40000000u},
        {"srli 31", &CPU::srli, 0xFFFFFFFFu, 31u, 1u},
        {"srai zero", &CPU::srai, 0x80000001u, 0u, 0x80000001u},
        {"srai sign-fills", &CPU::srai, 0x80000000u, 1u, 0xC0000000u},
        {"srai 31", &CPU::srai, 0x80000000u, 31u, 0xFFFFFFFFu},
        {"srai positive", &CPU::srai, 0x7FFFFFFFu, 1u, 0x3FFFFFFFu},
    };
    for (const auto& example : examples) {
        for (uint32_t rd : {2u, 1u, 0u}) {
            CAPTURE(example.name, rd);
            Memory mem(256);
            CPU cpu(mem);
            cpu.write_register(1, example.source);
            cpu.write_register(2, 0xA5A5A5A5u);

            (cpu.*example.execute)(rd, 1, example.amount);

            CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : example.expected));
            CHECK(register_value(cpu, mem, 0) == 0u);
            if (rd != 1) CHECK(register_value(cpu, mem, 1) == example.source);
            if (rd != 2) CHECK(register_value(cpu, mem, 2) == 0xA5A5A5A5u);
        }
    }
}

TEST_CASE("register arithmetic and logic support overlapping operands") {
    struct Example {
        const char* name;
        void (CPU::* execute)(uint32_t, uint32_t, uint32_t);
        uint32_t left;
        uint32_t right;
        uint32_t expected;
    };
    const Example examples[] = {
        {"add", &CPU::add, 10u, 20u, 30u},
        {"add unsigned wrap", &CPU::add, 0xFFFFFFFFu, 1u, 0u},
        {"add signed wrap", &CPU::add, 0x7FFFFFFFu, 1u, 0x80000000u},
        {"sub", &CPU::sub, 20u, 10u, 10u},
        {"sub unsigned wrap", &CPU::sub, 0u, 1u, 0xFFFFFFFFu},
        {"sub signed wrap", &CPU::sub, 0x80000000u, 1u, 0x7FFFFFFFu},
        {"slt signed", &CPU::slt, 0xFFFFFFFFu, 1u, 1u},
        {"slt reversed", &CPU::slt, 1u, 0xFFFFFFFFu, 0u},
        {"slt equality", &CPU::slt, 5u, 5u, 0u},
        {"sltu unsigned", &CPU::sltu, 0xFFFFFFFFu, 1u, 0u},
        {"sltu reversed", &CPU::sltu, 1u, 0xFFFFFFFFu, 1u},
        {"sltu equality", &CPU::sltu, 5u, 5u, 0u},
        {"xor", &CPU::xor_execute, 0xF0F0F0F0u, 0x0FF00FF0u, 0xFF00FF00u},
        {"or", &CPU::or_execute, 0xF0F0F0F0u, 0x0FF00FF0u, 0xFFF0FFF0u},
        {"and", &CPU::and_execute, 0xF0F0F0F0u, 0x0FF00FF0u, 0x00F000F0u},
        {"sll", &CPU::sll, 0x80000001u, 1u, 2u},
        {"sll masks to five bits", &CPU::sll, 1u, 33u, 2u},
        {"sll masked zero", &CPU::sll, 0x80000001u, 32u, 0x80000001u},
        {"sll 31", &CPU::sll, 1u, 31u, 0x80000000u},
        {"srl", &CPU::srl, 0x80000000u, 1u, 0x40000000u},
        {"srl masks to five bits", &CPU::srl, 0x80000000u, 33u, 0x40000000u},
        {"srl masked zero", &CPU::srl, 0x80000001u, 32u, 0x80000001u},
        {"srl 31", &CPU::srl, 0xFFFFFFFFu, 31u, 1u},
        {"sra", &CPU::sra, 0x80000000u, 1u, 0xC0000000u},
        {"sra masks to five bits", &CPU::sra, 0x80000000u, 33u, 0xC0000000u},
        {"sra masked zero", &CPU::sra, 0x80000001u, 32u, 0x80000001u},
        {"sra 31", &CPU::sra, 0x80000000u, 31u, 0xFFFFFFFFu},
        {"sra positive", &CPU::sra, 0x7FFFFFFFu, 1u, 0x3FFFFFFFu},
    };
    for (const auto& example : examples) {
        for (uint32_t rd : {3u, 1u, 2u, 0u}) {
            CAPTURE(example.name, rd);
            Memory mem(256);
            CPU cpu(mem);
            cpu.write_register(1, example.left);
            cpu.write_register(2, example.right);
            cpu.write_register(3, 0xA5A5A5A5u);

            (cpu.*example.execute)(rd, 1, 2);

            CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : example.expected));
            CHECK(register_value(cpu, mem, 0) == 0u);
            if (rd != 1) CHECK(register_value(cpu, mem, 1) == example.left);
            if (rd != 2) CHECK(register_value(cpu, mem, 2) == example.right);
            if (rd != 3) CHECK(register_value(cpu, mem, 3) == 0xA5A5A5A5u);
        }
    }
}

TEST_CASE("loads use base plus signed offset and extend to 32 bits") {
    struct Example {
        const char* name;
        void (CPU::* execute)(uint32_t, int32_t, uint32_t);
        uint32_t word;
        uint32_t expected;
    };
    const Example examples[] = {
        {"lb negative", &CPU::lb, 0x89AB80FEu, 0xFFFFFFFEu},
        {"lb positive", &CPU::lb, 0x1234567Fu, 0x7Fu},
        {"lh negative", &CPU::lh, 0x89AB80FEu, 0xFFFF80FEu},
        {"lh positive", &CPU::lh, 0x1234567Fu, 0x567Fu},
        {"lw", &CPU::lw, 0x89AB80FEu, 0x89AB80FEu},
        {"lbu", &CPU::lbu, 0x89AB80FEu, 0xFEu},
        {"lhu", &CPU::lhu, 0x89AB80FEu, 0x80FEu},
    };
    for (const auto& example : examples) {
        for (int32_t offset : {-4, 4}) {
            for (uint32_t rd : {2u, 1u, 0u}) {
                CAPTURE(example.name, offset, rd);
                Memory mem(256);
                CPU cpu(mem);
                mem.write32(64, example.word);
                const auto base = static_cast<uint32_t>(64 - offset);
                cpu.write_register(1, base);
                cpu.write_register(2, 0xA5A5A5A5u);

                (cpu.*example.execute)(rd, offset, 1);

                CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : example.expected));
                CHECK(register_value(cpu, mem, 0) == 0u);
                CHECK(mem.read32(64) == example.word);
                if (rd != 1) CHECK(register_value(cpu, mem, 1) == base);
                if (rd != 2) CHECK(register_value(cpu, mem, 2) == 0xA5A5A5A5u);
            }
        }
    }
}

TEST_CASE("stores write only the requested low bytes at base plus signed offset") {
    struct Example {
        const char* name;
        void (CPU::* execute)(uint32_t, int32_t, uint32_t);
        uint32_t expected;
    };
    const Example examples[] = {
        {"sb writes one byte", &CPU::sb, 0xFFFFFFD4u},
        {"sh writes two bytes", &CPU::sh, 0xFFFFC3D4u},
        {"sw writes four bytes", &CPU::sw, 0xA1B2C3D4u},
    };
    for (const auto& example : examples) {
        for (int32_t offset : {-8, 8}) {
            CAPTURE(example.name, offset);
            Memory mem(256);
            CPU cpu(mem);
            mem.write32(68, 0xFFFFFFFFu);
            mem.write32(72, 0xFFFFFFFFu);
            mem.write32(76, 0xFFFFFFFFu);
            const auto base = static_cast<uint32_t>(72 - offset);
            cpu.write_register(1, base);
            cpu.write_register(2, 0xA1B2C3D4u);

            (cpu.*example.execute)(2, offset, 1);

            CHECK(mem.read32(72) == example.expected);
            CHECK(mem.read32(68) == 0xFFFFFFFFu);
            CHECK(mem.read32(76) == 0xFFFFFFFFu);
            CHECK(register_value(cpu, mem, 1) == base);
            CHECK(register_value(cpu, mem, 2) == 0xA1B2C3D4u);
            CHECK(register_value(cpu, mem, 0) == 0u);
        }
    }
}

TEST_CASE("step fetches, decodes, executes, and advances by four bytes") {
    Memory mem(256);
    CPU cpu(mem);
    mem.write32(0, 0x04000093u); // addi x1, x0, 64
    mem.write32(4, 0xFF900113u); // addi x2, x0, -7
    mem.write32(8, 0x002081B3u); // add  x3, x1, x2
    mem.write32(12, 0x0030A223u); // sw   x3, 4(x1)
    mem.write32(16, 0x0040A203u); // lw   x4, 4(x1)
    mem.write32(20, 0x00000297u); // auipc x5, 0: save this instruction's PC
    mem.write32(24, 0x00100313u); // addi x6, x0, 1: must not run yet

    for (int instruction = 0; instruction < 6; ++instruction) {
        cpu.step();
    }

    CHECK(mem.read32(68) == 57u);
    CHECK(register_value(cpu, mem, 1) == 64u);
    CHECK(register_value(cpu, mem, 2) == 0xFFFFFFF9u);
    CHECK(register_value(cpu, mem, 3) == 57u);
    CHECK(register_value(cpu, mem, 4) == 57u);
    CHECK(register_value(cpu, mem, 5) == 20u);
    CHECK(register_value(cpu, mem, 6) == 0u);
    CHECK(register_value(cpu, mem, 0) == 0u);
}

TEST_CASE("lui and auipc use the upper immediate and the current instruction PC") {
    for (uint32_t opcode : {0x37u, 0x17u}) { // LUI, AUIPC
        for (uint32_t upper : {0u, 0x12345u, 0xFFFFFu}) {
            for (uint32_t rd : {3u, 0u}) {
                CAPTURE(opcode, upper, rd);
                Memory mem(256);
                CPU cpu(mem);
                cpu.write_register(3, 0xA5A5A5A5u);
                mem.write32(0, nop);
                mem.write32(4, (upper << 12) | (rd << 7) | opcode);
                mem.write32(8, 0x00000217u); // auipc x4, 0
                cpu.step();
                cpu.step();
                cpu.step();

                const uint32_t expected = (upper << 12) + (opcode == 0x17u ? 4u : 0u);
                CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : expected));
                CHECK(register_value(cpu, mem, 4) == 8u);
                CHECK(register_value(cpu, mem, 0) == 0u);
                if (rd == 0) CHECK(register_value(cpu, mem, 3) == 0xA5A5A5A5u);
            }
        }
    }
}

TEST_CASE("branches handle taken, untaken, signed, and unsigned comparisons") {
    struct Example {
        const char* name;
        uint32_t funct3;
        uint32_t left;
        uint32_t right;
        bool taken;
    };
    const Example examples[] = {
        {"beq equal", 0u, 5u, 5u, true},
        {"beq different", 0u, 5u, 6u, false},
        // BNE is the instruction named bnq in the supplied CPU header.
        {"bne different", 1u, 5u, 6u, true},
        {"bne equal", 1u, 5u, 5u, false},
        {"blt signed", 4u, 0xFFFFFFFFu, 1u, true},
        {"blt greater", 4u, 1u, 0xFFFFFFFFu, false},
        {"blt equal", 4u, 5u, 5u, false},
        {"bge signed", 5u, 1u, 0xFFFFFFFFu, true},
        {"bge less", 5u, 0xFFFFFFFFu, 1u, false},
        {"bge equal", 5u, 5u, 5u, true},
        {"bltu unsigned", 6u, 0xFFFFFFFFu, 1u, false},
        {"bltu less", 6u, 1u, 0xFFFFFFFFu, true},
        {"bltu equal", 6u, 5u, 5u, false},
        {"bgeu unsigned", 7u, 0xFFFFFFFFu, 1u, true},
        {"bgeu less", 7u, 1u, 0xFFFFFFFFu, false},
        {"bgeu equal", 7u, 5u, 5u, true},
    };
    for (const auto& example : examples) {
        for (int32_t offset : {-8, 8}) {
            CAPTURE(example.name, offset);
            Memory mem(256);
            CPU cpu(mem);
            cpu.write_register(1, example.left);
            cpu.write_register(2, example.right);
            mem.write32(0, capture_pc_in_x3);
            mem.write32(4, nop);
            mem.write32(8, branch(example.funct3, 1, 2, offset));
            mem.write32(12, capture_pc_in_x3);
            mem.write32(16, capture_pc_in_x3);
            cpu.step();
            cpu.step();
            cpu.write_register(3, 0xDEADBEEFu);

            cpu.step(); // branch
            cpu.step(); // capture the PC at the selected destination in x3

            const auto expected = example.taken ? static_cast<uint32_t>(8 + offset) : 12u;
            CHECK(register_value(cpu, mem, 3) == expected);
            CHECK(register_value(cpu, mem, 1) == example.left);
            CHECK(register_value(cpu, mem, 2) == example.right);
            CHECK(register_value(cpu, mem, 0) == 0u);
        }
    }
}

TEST_CASE("jal jumps relative to its own PC and saves PC plus four") {
    for (int32_t offset : {-8, 8}) {
        for (uint32_t rd : {5u, 0u}) {
            CAPTURE(offset, rd);
            Memory mem(256);
            CPU cpu(mem);
            mem.write32(0, capture_pc_in_x3);
            mem.write32(4, nop);
            mem.write32(8, jump(rd, offset));
            mem.write32(16, capture_pc_in_x3);
            cpu.step();
            cpu.step();
            cpu.write_register(3, 0xDEADBEEFu);

            cpu.step(); // jal
            cpu.step(); // capture the destination PC

            CHECK(register_value(cpu, mem, 3) == static_cast<uint32_t>(8 + offset));
            CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : 12u));
            CHECK(register_value(cpu, mem, 0) == 0u);
        }
    }
}

TEST_CASE("jalr uses the old base register, clears target bit zero, and saves PC plus four") {
    for (int32_t offset : {-4, 4}) {
        for (uint32_t rd : {5u, 1u, 0u}) {
            CAPTURE(offset, rd);
            Memory mem(256);
            CPU cpu(mem);
            const auto base = static_cast<uint32_t>(17 - offset);
            cpu.write_register(1, base);
            mem.write32(0, nop);
            mem.write32(4, nop);
            const uint32_t immediate = static_cast<uint32_t>(offset) & 0xFFFu;
            mem.write32(8, (immediate << 20) | (1u << 15) | (rd << 7) | 0x67u);
            mem.write32(16, capture_pc_in_x3);
            cpu.write_register(3, 0xDEADBEEFu);
            cpu.step();
            cpu.step();

            cpu.step(); // jalr
            cpu.step(); // capture the destination PC

            CHECK(register_value(cpu, mem, 3) == 16u); // (base + offset) & ~1u
            CHECK(register_value(cpu, mem, rd) == (rd == 0 ? 0u : 12u));
            CHECK(register_value(cpu, mem, 0) == 0u);
            if (rd != 1) CHECK(register_value(cpu, mem, 1) == base);
        }
    }
}
