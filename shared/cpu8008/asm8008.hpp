#pragma once
#include "../common.hpp"

#include <vector>

namespace ASM8008
{

	void assemble(const char* filename, std::vector<u8>& output);
	void disassemble(const u8* code, size_t code_size, std::vector<DisassemblyLine>& output);

}
