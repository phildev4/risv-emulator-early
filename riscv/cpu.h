#pragma once
#include <cstdint>

class CPU {

public:
	
	
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

	void write_register(uint32_t index, uint32_t value);

private:
	struct Registers {
		uint32_t x[32] = {};
		uint32_t PC = 0;
	};
	Registers regs;
};

