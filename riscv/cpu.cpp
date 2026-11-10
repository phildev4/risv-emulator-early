#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory& mem) : mem_(mem) {} //member initilisation 

void CPU::write_register(uint32_t index, uint32_t value) {
	if (index != 0 && index < 32) {
		regs.x[index] = value;
	}

}
void CPU::step() {
	
}
void CPU::addi(uint32_t rd, uint32_t rs1, int imm){	
	write_register(rd, regs.x[rs1] + imm);
	regs.PC = regs.PC + 4;
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
}

void CPU::add(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] + regs.x[rs2]);
}

void CPU::sub(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] - regs.x[rs2]);
}
void CPU::sll(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] << (regs.x[rs2] & 0x1F)); //again bitwise function this time to only take 5 bits of rs2 as max shift of register can be 31
}
void CPU::slt(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	int32_t signed_val_rs2 = static_cast<int32_t>(regs.x[rs2]);
	if( regs.x[rs1] < signed_val_rs2){
		write_register(rd, 1);
	}
	else { write_register(rd, 0); }
}
void CPU::sltu(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	if (regs.x[rs1] < regs.x[rs2]) {
		write_register(rd, 1);
	}
	else { write_register(rd, 0); }
}

void CPU::xor_execute(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] ^ regs.x[rs2]);
}
void CPU::srl(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] >> (regs.x[rs2] & 0x1F));
}
void CPU::sra(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, static_cast<int32_t>(regs.x[rs1]) >> regs.x[rs2] & 0x1F); //this time the static cast replaces the bitwise logic used before
	//AFAIK it somehow figures out how to correctly deal with this and the shift
}
void CPU::or_execute(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] | rs2);
}
void CPU::and_execute(uint32_t rd, uint32_t rs1, uint32_t rs2) {
	write_register(rd, regs.x[rs1] & rs2);

}
void CPU::lui(uint32_t rd, uint32_t imm) { //this case it is a unsinged as it is simply value from [31:12]
	write_register(rd, imm << 12);

}
void CPU::auipc(uint32_t rd, uint32_t imm) {
	write_register(rd, regs.PC + (imm < 12));
}
void CPU::lbu(uint32_t rd, int32_t imm, uint32_t rs1) {

	write_register(rd ,mem_.read8(regs.x[rs1] + imm));
}
void CPU::lb(uint32_t rd, int32_t imm, uint32_t rs2) {
	uint32_t address = regs.x[rs2] + imm;
	uint32_t value = mem_.read8(address);
	if (value & 0x80u) { //sign extension since mem returns unsinged int and we need signed( its is checking for 1 at msb)
		value = value | 0xFFFFFF00u; // extends by 1
	}
	write_register(rd, value);
}
void CPU::lhu(uint32_t rd, int32_t imm, uint32_t rs1) {

	write_register(rd, mem_.read16(regs.x[rs1] + imm));
}
void CPU::lw(uint32_t rd, int32_t imm, uint32_t rs1) {

	write_register(rd, mem_.read32(regs.x[rs1] + imm)); // no sign extension needed since already 4 bytes
}
void CPU::lh(uint32_t rd, int32_t imm, uint32_t rs2) {
	uint32_t address = regs.x[rs2] + imm;
	uint32_t value = mem_.read8(address);
	if (value & 0x8000u) { //sign extension since mem returns unsinged int and we need signed( its is checking for 1 at msb)
		value = value | 0xFFFFFF00u; // extends by 1
	}
	write_register(rd, value);
}
void CPU::sb(uint32_t rs1, int32_t imm, uint32_t rs2) {
	uint32_t address = regs.x[rs1] + imm;
	uint8_t value = regs.x[rs2] & 0xFFu; //only takes lowest 8 bites
	mem_.write8(address, value);
}
void CPU::sh(uint32_t rs1, int32_t imm, uint32_t rs2) {
	uint32_t address = regs.x[rs1] + imm;
	uint16_t value = regs.x[rs2] & 0xFFFFu; //only takes lowest 16 bites
	mem_.write16(address, value);
}
void CPU::sw(uint32_t rs1, int32_t imm, uint32_t rs2) {
	uint32_t address = regs.x[rs1] + imm;
	mem_.write32(address, regs.x[rs2]);
}
void CPU::beq(uint32_t rs1, uint32_t rs2, int32_t imm) {
	if (regs.x[rs1] == regs.x[rs2]) {
		regs.PC = regs.PC + imm;
	}
	else {
		regs.PC = regs.PC + 4;; // just goes PC + 4 as all instruction 4 bytes apart
	}
}
void CPU::bnq(uint32_t rs1, uint32_t rs2, int32_t imm) {
	if (regs.x[rs1] == regs.x[rs2]) {
		regs.PC = regs.PC + 4;;
		
	}
	else {
		regs.PC = regs.PC + imm;
	}
}
void CPU::blt(uint32_t rs1, uint32_t rs2, int32_t imm) {
	if (static_cast<int32_t>(regs.x[rs1]) < static_cast<int32_t>(regs.x[rs2])) {
		regs.PC = regs.PC + imm;
	}
	else { regs.PC = regs.PC + 4;; }
}
void CPU::bge(uint32_t rs1, uint32_t rs2, int32_t imm) {
	if (static_cast<int32_t>(regs.x[rs1]) >= static_cast<int32_t>(regs.x[rs2])) {
		regs.PC = regs.PC + imm;
	}
	else { regs.PC = regs.PC + 4;; }
}
void CPU::bltu(uint32_t rs1, uint32_t rs2, int32_t imm) {
	if (regs.x[rs1] < regs.x[rs2]) {
		regs.PC = regs.PC + imm;
	}
	else { regs.PC = regs.PC + 4;; }
}
void CPU::bgeu(uint32_t rs1, uint32_t rs2, int32_t imm) {
	if (regs.x[rs1] >= regs.x[rs2]) {
		regs.PC = regs.PC + imm;
	}
	else { regs.PC = regs.PC + 4;; }
}
void CPU::jal(uint32_t rd, int32_t imm) {
	write_register(rd, regs.PC + 4);
	regs.PC = regs.PC + imm;
}
void CPU::jalr(uint32_t rd, int32_t imm, uint32_t rs1) {
	uint32_t address = (regs.x[rs1] + imm) & ~1u; //clears lsb 
	write_register(rd, regs.PC + 4);
	regs.PC = address;
}

void CPU::ecall() {
	abort();
}
void CPU::ebreak() {
	abort();
}
