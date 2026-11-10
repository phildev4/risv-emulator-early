#include "cpu.h"
#include "memory.h"

void CPU::write_register(uint32_t index, uint32_t value) {
	if (index != 0 && index < 32) {
		regs.x[index] = value;
	}

}
void CPU::step() {
	regs.PC = regs.PC + 4;
}
void CPU::addi(uint32_t rd, uint32_t rs1, int imm){	
	write_register(rd, regs.x[rs1] + imm);
	step();
}
void CPU::slti(uint32_t rd, uint32_t rs1, int imm) {
	if (imm > static_cast<int32_t>(regs.x[rs1])) { //convert rs1 from unsigned to signed as slti is unsigned operatoin
		write_register(rd, 1);
	}
	else { write_register(rd, 0); }
}
void CPU::sltiu(uint32_t rd, uint32_t rs1, int imm) {
	if (static_cast<uint32_t>(imm) > (regs.x[rs1])) { //convert imm from signed to unsigned as sltiu is unsigned operatoin
		write_register(rd, 1);
	}
	else { write_register(rd, 0); }
}
void CPU::xori(uint32_t rd, uint32_t rs1, int imm) {
	write_register(rd, regs.x[rs1] ^ static_cast<uint32_t>(imm)); 
}
void CPU::ori(uint32_t rd, uint32_t rs1, int imm) {
	write_register(rd, regs.x[rs1] | static_cast<uint32_t>(imm)); 
}
void CPU::andi(uint32_t rd, uint32_t rs1, int imm) { 
	write_register(rd, regs.x[rs1] & static_cast<uint32_t>(imm));
	
}
void CPU::slli(uint32_t rd, uint32_t rs1, uint32_t shamt) { //shift left
	write_register(rd, regs.x[rs1] << shamt);
}
void CPU::srli(uint32_t rd, uint32_t rs1, uint32_t shamt) { //shift right
	write_register(rd, regs.x[rs1] >> shamt);

}
void CPU::srai(uint32_t rd, uint32_t rs1, uint32_t shamt) {
	uint32_t value = regs.x[rs1];
	uint32_t result = regs.x[rs1] >> shamt;
	if ((value & 0x80000000u) && shamt > 0) { //checks if MSB is 1 to see sign abd shamt to avoid undefined behaviour of shifting by 0
		result = result | 0xFFFFFFFFu << (32 - shamt); //again really annoying bitwise operations
		//apparently just using static_cast<int32_t> on value would be sufficent well oh well
		// 0xFFFFFFFFu if just 32 bit number with all 1
		// so the rigtward shift just creates the number of 1s we needed combined with bitmask OR to create 1s
	}
}s



