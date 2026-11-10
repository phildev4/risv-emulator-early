    
#include "memory.h"
#include "decoder.h"

std::uint32_t Decoder::opcode(std::uint32_t instruction) {
    return instruction & 0x7Fu;          
}

std::uint32_t Decoder::funct3(std::uint32_t instruction) {
    return (instruction >> 12) & 0x7u;   
}

std::uint32_t Decoder::funct7(std::uint32_t instruction) {
    return (instruction >> 25) & 0x7Fu;  
}

std::uint32_t Decoder::rs2(uint32_t instruction) {
    return (instruction >> 20) & 0x1Fu;
}

std::uint32_t Decoder::rs1(uint32_t instruction) {
    return (instruction >> 15) & 0x1Fu;
}

std::uint32_t Decoder::rd(uint32_t instruction) {
    return (instruction >> 7) & 0x1Fu;
}

std::int32_t Decoder::imm_i(uint32_t instruction) {
    uint32_t imm = (instruction >> 20) & 0xFFFu;

    if (imm & 0x800u) {
        return static_cast<int32_t>(imm) - 0x1000;
    }
    return static_cast<int32_t>(imm); //sign extension to 32 bit int
}

std::int32_t Decoder::imm_s(uint32_t instruction) {
    uint32_t imm =
        (((instruction >> 25) & 0x7Fu) << 5) |
        ((instruction >> 7) & 0x1Fu);

    if (imm & 0x800u) {
        return static_cast<int32_t>(imm) - 0x1000;
    }
    return static_cast<int32_t>(imm);
}

std::int32_t Decoder::imm_u(uint32_t instruction) {
    
    return static_cast<int32_t>(instruction & 0xFFFFF000u);
}


std::int32_t Decoder::imm_j(uint32_t instruction) {
    uint32_t imm =
        ((instruction >> 21) & 0x3FF) << 1 |
        (((instruction >> 20) & 0x1) << 11) |
        ((instruction >> 12) & 0xFF) << 12 |
        (((instruction >> 31) & 0x1) << 20);


    if (imm & 0x100000u) {
          return static_cast<int32_t>(imm) - 0x200000;
        }

    return static_cast<int32_t>(imm);
}

std::int32_t Decoder::imm_b(uint32_t instruction) {
    uint32_t imm =
        ((instruction >> 8) & 0xF) << 1 |
        ((instruction >> 25) & 0x3F) << 5 |
        ((instruction >> 7) & 0x1) << 11 |
        ((instruction >> 31) & 0x1) << 12;
    if (imm & 0x1000u) {
        return static_cast<int32_t>(imm) - 0x2000;
    }
    return static_cast<int32_t>(imm);
}
