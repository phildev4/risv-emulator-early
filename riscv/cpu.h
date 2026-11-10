#pragma once
#include <cstdint>
#include "riscv/memory.h"

class CPU {

public:
	explicit CPU(Memory& mem);
	void step();
	/// <summary>
	///  stores content of rs1 + constant IMM at rd
	/// </summary>
	/// <param name="rd"></param>
	/// <param name="rs1"></param>
	/// <param name="imm"></param>
	void addi(uint32_t rd, uint32_t rs1, int imm);
	/// <summary>
	/// Compares  rs1 with constant imm
	/// </summary>
	/// <param name="rd"></param> 1 if rs1 is less than imm else -
	/// <param name="rs1"></param> 
	/// <param name="imm"></param>
	void slti(uint32_t rd, uint32_t rs1, int imm);
	void sltiu(uint32_t rd, uint32_t rs1, int imm);
	void xori(uint32_t rd, uint32_t rs1, int imm);
	void ori(uint32_t rd, uint32_t rs1, int imm);
	void andi(uint32_t rd, uint32_t rs1, int imm);

	void slli(uint32_t rd, uint32_t rs1, uint32_t shamt);
	void srli(uint32_t rd, uint32_t rs1, uint32_t shamt);
	void srai(uint32_t rd, uint32_t rs1, uint32_t shamt);

	//R type instructions format (destination register, reg 1 , reg2)
	void add(uint32_t rd, uint32_t rs1, uint32_t rs2);
	void sub(uint32_t rd, uint32_t rs1, uint32_t rs2);
	/// <summary>
	/// Shifts left by 5 smallest bits of rs2
	/// </summary>
	/// <param name="rd">destination register</param>
	/// <param name="rs1">What gets shifted</param>
	/// <param name="rs2">Shift by(5 bits max) </param>
	void sll(uint32_t rd, uint32_t rs1, uint32_t rs2);
	/// <summary>
	/// Set rd to 1 if rs1 is less than (signed)rs2  else 0
	/// </summary>
	/// <param name="rd"></param>
	/// <param name="rs1"></param>
	/// <param name="rs2"></param>
	void slt(uint32_t rd, uint32_t rs1, uint32_t rs2);
	/// <summary>
	/// same as slt but rs2 is treated as unsinged
	/// </summary>
	/// <param name="rd"></param>
	/// <param name="rs1"></param>
	/// <param name="rs2"></param>
	void sltu(uint32_t rd, uint32_t rs1, uint32_t rs2);
	void xor_execute(uint32_t rd, uint32_t rs1, uint32_t rs2);
	/// <summary>
	/// Shift right locial R type instruction
	/// </summary>
	/// <param name="rd"></param>
	/// <param name="rs1"></param>
	/// <param name="rs2"></param>
	void srl(uint32_t rd, uint32_t rs1, uint32_t rs2);
	/// <summary>
	/// Shift right analytical so taking the MSB as sign bit
	/// </summary>
	/// <param name="rd"></param>
	/// <param name="rs1"></param>
	/// <param name="rs2"></param>
	void sra(uint32_t rd, uint32_t rs1, uint32_t rs2);
	void or_execute(uint32_t rd, uint32_t rs1, uint32_t rs2);
	void and_execute(uint32_t rd, uint32_t rs1, uint32_t rs2);
	void lui(uint32_t rd, uint32_t imm);
	void auipc(uint32_t rd, uint32_t imm);
	/// <summary>
	/// load 8 bit to memory and write to rd
	/// </summary>
	/// <param name="rd"></param>
	/// <param name="imm"></param>
	void lb(uint32_t rd, int32_t imm, uint32_t rs1); // read 1 byte from mem
	void lh(uint32_t rd, int32_t imm, uint32_t rs1); //read 2 byte from mem
	void lw(uint32_t rd, int32_t imm, uint32_t rs1);//read 4 byte from mem
	void lbu(uint32_t rd, int32_t imm, uint32_t rs1);
	void lhu(uint32_t rd, int32_t imm, uint32_t rs1);
	void sb(uint32_t rs2, int32_t imm, uint32_t rs1); //stores lowest 8 bytes to mem
	void sh(uint32_t rs2, int32_t imm, uint32_t rs1); //stores lowest 16 bytes to mem
	void sw(uint32_t rs2, int32_t imm, uint32_t rs1); //stores lowest 32 bytes to mem

	void beq(uint32_t rs1, uint32_t rs2, int32_t imm); //branch if equal
	void bnq(uint32_t rs1, uint32_t rs2, int32_t imm); //branch if not equal
	void blt(uint32_t rs1, uint32_t rs2, int32_t imm); //branch if rs1 < rs2(signed)
	void bltu(uint32_t rs1, uint32_t rs2, int32_t imm); //branch if rs1 < rs2 (unsinged)
	void bge(uint32_t rs1, uint32_t rs2, int32_t imm); //branch greate or equal rs1 rs2 (signed)
	void bgeu(uint32_t rs1, uint32_t rs2, int32_t imm); //branch greate or equal rs1 rs2 (unsinged)
	
	void jal(uint32_t rd, int32_t imm);
	void jalr(uint32_t rd, int32_t imm, uint32_t rs1);

	void ecall();
	void ebreak();


	void write_register(uint32_t index, uint32_t value);

private:
	struct Registers {
		uint32_t x[32] = {};
		uint32_t PC = 0;
	};
	Registers regs;
	Memory& mem_;
};

