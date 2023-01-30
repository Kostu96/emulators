#include "chip8_core.hpp"
#include "disasm_chip8.hpp"
#include "chip8_instruction.hpp"

#include <cassert>
#include <cstring>
#include <fstream>

extern "C"
{
    __declspec(dllexport) CHIP8Core* allocator()
    {
        return new CHIP8Core{};
    }

    __declspec(dllexport) void deleter(CHIP8Core* ptr)
    {
        delete ptr;
    }
}

void CHIP8Core::render(CharVertex* verts) const
{
    for (u16 row = 0; row < 32; row++)
        for (u16 col = 0; col < 8; col++)
        {
            u16 screenIndex = row * 8 + col;
            for (u16 bit = 0; bit < 8; bit++)
            {
                u16 vertIndex = row * 64 + col * 8 + bit;
                bool on = Screen[screenIndex] & (1 << (7 - bit));
                verts[vertIndex].color = on ? 0xFFFFFFFF : 0;
            }
        }
}

void CHIP8Core::handleKey(int key, int action)
{
    constexpr int KEY1 = 49;
    constexpr int KEY2 = 50;
    constexpr int KEY3 = 51;
    constexpr int KEY4 = 52;
    constexpr int KEYQ = 81;
    constexpr int KEYW = 87;
    constexpr int KEYE = 69;
    constexpr int KEYR = 82;
    constexpr int KEYA = 65;
    constexpr int KEYS = 83;
    constexpr int KEYD = 68;
    constexpr int KEYF = 70;
    constexpr int KEYZ = 90;
    constexpr int KEYX = 88;
    constexpr int KEYC = 67;
    constexpr int KEYV = 86;

    switch (key)
    {
    case KEY1: keys[0x1] = action; break;
    case KEY2: keys[0x2] = action; break;
    case KEY3: keys[0x3] = action; break;
    case KEY4: keys[0xC] = action; break;
    case KEYQ: keys[0x4] = action; break;
    case KEYW: keys[0x5] = action; break;
    case KEYE: keys[0x6] = action; break;
    case KEYR: keys[0xD] = action; break;
    case KEYA: keys[0x7] = action; break;
    case KEYS: keys[0x8] = action; break;
    case KEYD: keys[0x9] = action; break;
    case KEYF: keys[0xE] = action; break;
    case KEYZ: keys[0xA] = action; break;
    case KEYX: keys[0x0] = action; break;
    case KEYC: keys[0xB] = action; break;
    case KEYV: keys[0xF] = action; break;
    }
}

void CHIP8Core::loadROM(const char* filename)
{
    constexpr u8 CHARSET_SIZE = 80;
    const u8 charset[CHARSET_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    std::memcpy(m_memory, charset, CHARSET_SIZE);

    std::ifstream fin{ filename, std::ios::binary };
    fin.read((char*)(m_memory + 0x200), 0x1000 - 0x200);
    fin.close();

    disassemble(m_memory + 0x200, 0x1000 - 0x200, m_disassembly);
}

void CHIP8Core::reset()
{
    std::srand(1234567890);

    std::memset(Screen, 0, 8 * 32);

    PC = 0x200;
    SP = 0;

    DT = ST = 0;

    for (u8 i = 0; i < 16; i++)
        keys[i] = false;

    m_elspsedTime = 0.0;

    updateState();
}

void CHIP8Core::update(double dt)
{
    m_elspsedTime += dt;

    if (m_elspsedTime > 16.67)
    {
        m_elspsedTime -= 16.67;
        if (DT) DT--;
        if (ST) ST--;
    }

    Instruction instruction;
    instruction.h2 = m_memory[PC++];
    instruction.h1 = m_memory[PC++];

    switch (instruction.n4)
    {
    case 0x0:
        if (instruction.word == 0x00E0)
            std::memset(Screen, 0, 8 * 32);
        else if (instruction.word == 0x00EE)
        {
            PC = (Stack[SP - 2] << 8);
            PC |= Stack[SP - 1];
            SP -= 2;
        }
        else
            assert(false);
        break;
    case 0x1:
        PC = instruction.word & 0x0FFF;
        break;
    case 0x2:
        Stack[SP++] = PC >> 8;
        Stack[SP++] = PC & 0xFF;
        PC = instruction.word & 0x0FFF;
        break;
    case 0x3:
        if (GPR[instruction.n3] == instruction.h1)
            PC += 2;
        break;
    case 0x4:
        if (GPR[instruction.n3] != instruction.h1)
            PC += 2;
        break;
    case 0x5:
        if (GPR[instruction.n3] == GPR[instruction.n2])
            PC += 2;
        break;
    case 0x6:
        GPR[instruction.n3] = instruction.h1;
        break;
    case 0x7:
        GPR[instruction.n3] += instruction.h1;
        break;
    case 0x8:
        switch (instruction.n1)
        {
        case 0x0:
            GPR[instruction.n3] = GPR[instruction.n2];
            break;
        case 0x1:
            GPR[instruction.n3] |= GPR[instruction.n2];
            break;
        case 0x2:
            GPR[instruction.n3] &= GPR[instruction.n2];
            break;
        case 0x3:
            GPR[instruction.n3] ^= GPR[instruction.n2];
            break;
        case 0x4: {
            u16 temp = (u16)GPR[instruction.n3] + (u16)GPR[instruction.n2];
            GPR[0xF] = temp >> 8;
            GPR[instruction.n3] = temp & 0xFF;
        } break;
        case 0x5: {
            s16 temp = (s16)GPR[instruction.n3] - (s16)GPR[instruction.n2];
            GPR[instruction.n3] = temp & 0xFF;
            GPR[0xF] = temp < 0 ? 0 : 1;
        } break;
        case 0x6: {
            u8 temp = GPR[instruction.n3] & 1;
            GPR[instruction.n3] >>= 1;
            GPR[0xF] = temp;
        } break;
        case 0x7: {
            s16 temp = (s16)GPR[instruction.n2] - (s16)GPR[instruction.n3];
            GPR[instruction.n3] = temp & 0xFF;
            GPR[0xF] = temp < 0 ? 0 : 1;
        } break;
        case 0xE: {
            u8 temp = GPR[instruction.n3] >> 7;
            GPR[instruction.n3] <<= 1;
            GPR[0xF] = temp;
        } break;
        default:
            assert(false && "Unhandled 0x8xxx!");
        }
        break;
    case 0x9:
        if (GPR[instruction.n3] != GPR[instruction.n2])
            PC += 2;
        break;
    case 0xA:
        I = instruction.word & 0x0FFF;
        break;
    case 0xC:
        GPR[instruction.n3] = (std::rand() % 256) & instruction.h1;
        break;
    case 0xD: {
        u8 x = GPR[instruction.n3];
        u8 x_byte = x / 8;
        u8 x_bit = x % 8;
        u8 y = GPR[instruction.n2];
        u8 bytes = instruction.n1;
        GPR[0xF] = 0;
        for (u8 i = 0; i < bytes; i++)
        {
            u16 index = (y + i) * 8 + x_byte;
            GPR[0xF] = (Screen[index] & m_memory[I + i] >> x_bit) | (Screen[index + 1] & m_memory[I + i] << (8 - x_bit));
            Screen[index] ^= m_memory[I + i] >> x_bit;
            Screen[index + 1] ^= m_memory[I + i] << (8 - x_bit);

            for (u16 bit = 0; bit < 8; bit++)
            {
                //u16 vertIndex = row * 64 + col * 8 + bit;
                bool on = Screen[index] & (1 << (7 - bit));
                m_renderPoint(x, y + i, on ? 0xFFFFFFFF : 0);
                on = Screen[index + 1] & (1 << (7 - bit));
                m_renderPoint(8 + x, y + i, on ? 0xFFFFFFFF : 0);
            }
        }
    } break;
    case 0xE:
        if (instruction.h1 == 0x9E)
        {
            if (keys[GPR[instruction.n3]])
                PC += 2;
        }
        else if (instruction.h1 == 0xA1)
        {
            if (!keys[GPR[instruction.n3]])
                PC += 2;
        }
        break;
    case 0xF:
        switch (instruction.h1)
        {
        case 0x07:
            GPR[instruction.n3] = DT;
            break;
        case 0x0A: {
            bool found = false;
            for (u8 i = 0; i < 16; i++)
                if (keys[i]) {
                    found = true;
                    break;
                }

            if (!found)
                PC -= 2;
        } break;
        case 0x15:
            DT = GPR[instruction.n3];
            break;
        case 0x18:
            ST = GPR[instruction.n3];
            break;
        case 0x1E:
            I += GPR[instruction.n3];
            break;
        case 0x29:
            I = GPR[instruction.n3] * 5;
            break;
        case 0x33: {
            u8 n1 = GPR[instruction.n3] % 10;
            u8 n2 = (GPR[instruction.n3] / 10) % 10;
            u8 n3 = GPR[instruction.n3] / 100;
            m_memory[I] = n3;
            m_memory[I + 1] = n2;
            m_memory[I + 2] = n1;
        } break;
        case 0x55:
            for (u8 i = 0; i <= instruction.n3; i++)
                m_memory[I + i] = GPR[i];
            break;
        case 0x65:
            for (u8 i = 0; i <= instruction.n3; i++)
                GPR[i] = m_memory[I + i];
            break;
        default:
            assert(false && "Unhandled 0xFxxx!");
        }
        break;
    default:
        assert(false && "Unhandled opcode!");
    }

    updateState();
}

CHIP8Core::CHIP8Core() :
    m_emulatorSettings{ CHIP8_WIDTH, CHIP8_HEIGHT,
                        WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP-8" }
{
    m_state.push_back({ 0, 2, "V0:", true });
    m_state.push_back({ 0, 2, "V1:" });
    m_state.push_back({ 0, 2, "V2:", true });
    m_state.push_back({ 0, 2, "V3:" });
    m_state.push_back({ 0, 2, "V4:", true });
    m_state.push_back({ 0, 2, "V5:" });
    m_state.push_back({ 0, 2, "V6:", true });
    m_state.push_back({ 0, 2, "V7:" });
    m_state.push_back({ 0, 2, "V8:", true });
    m_state.push_back({ 0, 2, "V9:" });
    m_state.push_back({ 0, 2, "VA:", true });
    m_state.push_back({ 0, 2, "VB:" });
    m_state.push_back({ 0, 2, "VC:", true });
    m_state.push_back({ 0, 2, "VD:" });
    m_state.push_back({ 0, 2, "VE:", true });
    m_state.push_back({ 0, 2, "VF:", false, true });
    m_state.push_back({ 0, 3, "PC:" });
    m_state.push_back({ 0, 3, "I:", true });
    m_state.push_back({ 0, 1, "SP:" });
    m_state.push_back({ 0, 2, "DT:", true });
    m_state.push_back({ 0, 2, "ST:" });
}

void CHIP8Core::updateState()
{
    m_state[0].value = GPR[0x0];
    m_state[1].value = GPR[0x1];
    m_state[2].value = GPR[0x2];
    m_state[3].value = GPR[0x3];
    m_state[4].value = GPR[0x4];
    m_state[5].value = GPR[0x5];
    m_state[6].value = GPR[0x6];
    m_state[7].value = GPR[0x7];
    m_state[8].value = GPR[0x8];
    m_state[9].value = GPR[0x9];
    m_state[10].value = GPR[0xA];
    m_state[11].value = GPR[0xB];
    m_state[12].value = GPR[0xC];
    m_state[13].value = GPR[0xD];
    m_state[14].value = GPR[0xE];
    m_state[15].value = GPR[0xF];
    m_state[16].value = PC;
    m_state[17].value = I;
    m_state[18].value = SP;
    m_state[19].value = DT;
    m_state[20].value = ST;
}
