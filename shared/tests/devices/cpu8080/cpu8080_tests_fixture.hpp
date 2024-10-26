#pragma once
#include "shared/source/devices/cpu8080/cpu8080.hpp"

#include <gtest/gtest.h>

struct CPU8080Tests :
    public testing::Test
{
    static constexpr u16 ROM_SIZE = 0x10;
    static constexpr u16 RAM_SIZE = 0x10;

    u8 rom[ROM_SIZE]{};
    u8 ram[RAM_SIZE]{};

    CPU8080 cpu;

    CPU8080Tests()
    {
        cpu.mapReadMemoryCallback([&](u16 address)
            {
                if (address < ROM_SIZE) return rom[address];
                if (address >= 0x8000) return ram[address - 0x8000];
                return u8{};
            });

        cpu.mapWriteMemoryCallback([&](u16 address, u8 data)
            {
                if (address >= 0x8000) ram[address - 0x8000] = data;
            });
    }

    void SetUp() override
    {
        std::memset(rom, 0, ROM_SIZE);
        std::memset(ram, 0, RAM_SIZE);
        cpu.reset();
        u8 clocks = 3;
        while (clocks--)
            cpu.clock();

        ASSERT_EQ(cpu.getCyclesLeft(), 0);
    }

    void TearDown() override
    {
        EXPECT_EQ(cpu.getCyclesLeft(), 0);
    }

    CPU8080::State captureCPUState() const
    {
        return cpu.getState();
    }

    void compareCPUStates(const CPU8080::State& state1, const CPU8080::State& state2)
    {
        EXPECT_EQ(state1.PC, state2.PC);
        EXPECT_EQ(state1.SP, state2.SP);
        EXPECT_EQ(state1.AF, state2.AF);
        EXPECT_EQ(state1.BC, state2.BC);
        EXPECT_EQ(state1.DE, state2.DE);
        EXPECT_EQ(state1.HL, state2.HL);
    }
};
