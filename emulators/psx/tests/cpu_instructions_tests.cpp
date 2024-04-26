#include "../cpu.hpp"

#include <gtest/gtest.h>

namespace PSX {

	struct CPUTests :
		public testing::Test
	{
		CPU cpu;

		static constexpr size_t MEMORY_SIZE = 0x10;
		u32 memory[MEMORY_SIZE];

		CPU::CPUStatus cpuStatusBefore;
		CPU::COP0Status cop0StatusBefore;
		u32 memoryBefore[MEMORY_SIZE];

		CPUTests() {
			cpu.mapRead32MemoryCallback([&](u32 address) { return memory[address >> 2]; });
			cpu.mapWrite32MemoryCallback([&](u32 address, u32 data) { memory[address >> 2] = data; });
		}

		void SetUp() override {
			cpu.reset();
			cpu.m_cpuStatus.PC = 0;
		}

		void makeSnapshot() {
			cpuStatusBefore = cpu.getCPUStatus();
			cop0StatusBefore = cpu.getCOP0Status();
			memcpy(memoryBefore, memory, MEMORY_SIZE);
		}

		void checkSnapshot() {
			EXPECT_EQ(memcmp(&cpuStatusBefore, &cpu.getCPUStatus(), sizeof(CPU::CPUStatus)), 0);
			EXPECT_EQ(memcmp(&cop0StatusBefore, &cpu.getCOP0Status(), sizeof(CPU::COP0Status)), 0);
			EXPECT_EQ(memcmp(memoryBefore, memory, MEMORY_SIZE * sizeof(u32)), 0);
		}
	};

	TEST_F(CPUTests, LUITest)
	{
		memory[0] = 0x3C080013; // LUI $8, 0x13

		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock();

		cpuStatusBefore.regs[8] = 0x13 << 16;
		cpuStatusBefore.PC += 8;

		checkSnapshot();
	}

	TEST_F(CPUTests, ORITest)
	{
		memory[0] = 0x3508243F; // ORI $8, $8, 0x243F

		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock();

		cpuStatusBefore.regs[8] |= 0x243F;
		cpuStatusBefore.PC += 8;

		checkSnapshot();
	}

	TEST_F(CPUTests, SWTestWithPositiveOffset)
	{
		cpu.m_cpuStatus.regs[1] = 0x2;
		cpu.m_cpuStatus.regs[8] = 0xDEADBEEF;
		memory[0] = 0xAC280006; // SW $8, 0x6($1)

		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock();

		cpuStatusBefore.PC += 8;
		memoryBefore[0x2] = 0xDEADBEEF;

		checkSnapshot();
	}

	TEST_F(CPUTests, SWTestWithNegativeOffset)
	{
		cpu.m_cpuStatus.regs[1] = 0x9;
		cpu.m_cpuStatus.regs[8] = 0xDEADBEEF;
		memory[0] = 0xAC28FFFF; // SW $8, 0xFFFF($1)

		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock();

		cpuStatusBefore.PC += 8;
		memoryBefore[0x2] = 0xDEADBEEF;

		checkSnapshot();
	}

	TEST_F(CPUTests, SSLTest)
	{
		cpu.m_cpuStatus.regs[8] = 0xDEADBEEF;
		memory[0] = 0x00084080; // SSL $8, $8, 0x2
		
		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock();

		cpuStatusBefore.regs[8] = 0x7AB6FBBC;
		cpuStatusBefore.PC += 8;

		checkSnapshot();
	}

	TEST_F(CPUTests, ADDIUTest) // TODO: test overflow conditions and negative values
	{
		memory[0] = 0x24080B88; // ADDIU $8, $zero, 0xB88

		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock();

		cpuStatusBefore.regs[8] = 0xB88;
		cpuStatusBefore.PC += 8;

		checkSnapshot();
	}
	
	TEST_F(CPUTests, JTest)
	{
		memory[0] = 0x08000002; // J 0x2
		memory[1] = 0x24080123; // ADDIU $8, $zero, 0x123
		memory[2] = 0x24090456; // ADDIU $9, $zero, 0x456

		makeSnapshot();

		cpu.clock(); // go over first dummy nop
		cpu.clock(); // execute jump
		cpu.clock(); // execute branch delay slot
		cpu.clock(); // execute instruction after jump

		cpuStatusBefore.regs[8] = 0x123;
		cpuStatusBefore.regs[9] = 0x456;
		cpuStatusBefore.PC = 0x10;

		checkSnapshot();
	}
} // namespace PSX
