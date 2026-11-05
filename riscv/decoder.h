#include <cstdint>

class Decoder {
public:
	std::uint32_t opcode(std::uint32_t instruction);
	std::uint32_t funct3(std::uint32_t instruction);
	std::uint32_t funct7(std::uint32_t instruction);

	std::uint32_t rs2(std::uint32_t instruction);
	std::uint32_t rs1(uint32_t instruction);
	std::uint32_t rd(uint32_t instruction);

	std::int32_t imm_i(uint32_t instruction);
	std::int32_t imm_s(uint32_t instruction);
	std::int32_t imm_u(uint32_t instruction);
	std::int32_t imm_b(uint32_t instruction);
	std::int32_t imm_j(uint32_t instruction);

};