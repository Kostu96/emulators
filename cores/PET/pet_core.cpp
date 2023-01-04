#include "pet_core.hpp"

#include <cassert>
#include <cstring>
#include <fstream>

extern "C"
{
    __declspec(dllexport) PETCore* allocator()
    {
        return new PETCore{};
    }

    __declspec(dllexport) void deleter(PETCore* ptr)
    {
        delete ptr;
    }
}

void PETCore::getWindowSettings(WindowSettings& settings)
{
    constexpr u16 CHIP8_WIDTH = 64;
    constexpr u16 CHIP8_HEIGHT = 32;
    constexpr u16 SCREEN_SIZE = CHIP8_WIDTH * CHIP8_HEIGHT;
    constexpr u16 ZOOM = 16;
    constexpr u16 WINDOW_WIDTH = CHIP8_WIDTH * ZOOM;
    constexpr u16 WINDOW_HEIGHT = CHIP8_HEIGHT * ZOOM;

    settings.width = WINDOW_WIDTH;
    settings.height = WINDOW_HEIGHT;
    strcpy_s(settings.title, "chip8");
}

std::span<u8> PETCore::getMemory()
{
    return std::span<u8>{Memory};
}

void PETCore::render(CharVertex* verts)
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

void PETCore::handleKey(int key, int action)
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
    case KEY4: keys[0xF] = action; break;
    case KEYQ: keys[0x4] = action; break;
    case KEYW: keys[0x5] = action; break;
    case KEYE: keys[0x6] = action; break;
    case KEYR: keys[0xE] = action; break;
    case KEYA: keys[0x7] = action; break;
    case KEYS: keys[0x8] = action; break;
    case KEYD: keys[0x9] = action; break;
    case KEYF: keys[0xD] = action; break;
    case KEYZ: keys[0xA] = action; break;
    case KEYX: keys[0x0] = action; break;
    case KEYC: keys[0xB] = action; break;
    case KEYV: keys[0xC] = action; break;
    }
}

void PETCore::loadROM(const char* filename)
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

    std::memcpy(Memory, charset, CHARSET_SIZE);

    std::ifstream fin{ filename, std::ios::binary };
    fin.read((char*)(Memory + 0x200), 0x4A0);
    fin.close();
}

void PETCore::reset()
{
    std::srand(1234567890);

    PC = 0x200;
    SP = 0;

    DT = ST = 0;

    for (u8 i = 0; i < 16; i++)
        keys[i] = false;
}

union Instruction
{
    struct
    {
        u8 n1 : 4;
        u8 n2 : 4;
        u8 n3 : 4;
        u8 n4 : 4;
    } nibbles;
    u16 word;
};

void PETCore::clock()
{
    if (DT) DT--;
    if (ST) ST--;

    Instruction instruction;
    instruction.word = Memory[PC++] << 8;
    instruction.word |= Memory[PC++];

    switch (instruction.nibbles.n4)
    {
    case 0x0:
        if (instruction.word == 0x00E0)
            std::memset(Screen, 0, 8 * 32);
        else if (instruction.word == 0x00EE)
        {
            PC = Stack[--SP];
            PC |= (Stack[--SP] << 8);
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
        if (GPR[instruction.nibbles.n3] == (instruction.word & 0xFF))
            PC += 2;
        break;
    case 0x4:
        if (GPR[instruction.nibbles.n3] != (instruction.word & 0xFF))
            PC += 2;
        break;
    case 0x5:
        if (GPR[instruction.nibbles.n3] == GPR[instruction.nibbles.n2])
            PC += 2;
        break;
    case 0x6:
        GPR[instruction.nibbles.n3] = instruction.word & 0xFF;
        break;
    case 0x7:
        GPR[instruction.nibbles.n3] += instruction.word & 0xFF;
        break;
    case 0x8:
        switch (instruction.nibbles.n1)
        {
        case 0x0:
            GPR[instruction.nibbles.n3] = GPR[instruction.nibbles.n2];
            break;
        case 0x1:
            GPR[instruction.nibbles.n3] |= GPR[instruction.nibbles.n2];
            break;
        case 0x2:
            GPR[instruction.nibbles.n3] &= GPR[instruction.nibbles.n2];
            break;
        case 0x3:
            GPR[instruction.nibbles.n3] ^= GPR[instruction.nibbles.n2];
            break;
        case 0x4: {
            u16 temp = (u16)GPR[instruction.nibbles.n3] + (u16)GPR[instruction.nibbles.n2];
            GPR[0xF] = temp >> 8;
            GPR[instruction.nibbles.n3] = temp & 0xFF;
        } break;
        case 0x5:
            GPR[0xF] = GPR[instruction.nibbles.n3] < GPR[instruction.nibbles.n2] ? 0 : 1;
            break;
        case 0x6:
            GPR[0xF] = GPR[instruction.nibbles.n3] & 1;
            GPR[instruction.nibbles.n3] >>= 1;
            break;
        case 0x7: {
            s16 temp = (s16)GPR[instruction.nibbles.n3] - (s16)GPR[instruction.nibbles.n2];
            GPR[0xF] = temp < 0 ? 1 : 0;
            GPR[instruction.nibbles.n3] = temp & 0xFF;
        } break;
        case 0xE:
            GPR[0xF] = GPR[instruction.nibbles.n3] >> 7;
            GPR[instruction.nibbles.n3] <<= 1;
            break;
        default:
            assert(false && "Unhandled n1!");
        }
        break;
    case 0x9:
        if (GPR[instruction.nibbles.n3] != GPR[instruction.nibbles.n2])
            PC += 2;
        break;
    case 0xA:
        I = instruction.word & 0x0FFF;
        break;
    case 0xC:
        GPR[instruction.nibbles.n3] = (std::rand() % 256) & (instruction.word & 0xFF);
        break;
    case 0xD: {
        u8 x = GPR[instruction.nibbles.n3];
        u8 x_byte = x / 8;
        u8 x_bit = x % 8;
        u8 y = GPR[instruction.nibbles.n2];
        u8 bytes = instruction.nibbles.n1;
        GPR[0xF] = 0;
        for (u8 i = 0; i < bytes; i++)
        {
            u16 index = (y + i) * 8 + x_byte;
            GPR[0xF] = (Screen[index] & Memory[I + i] >> x_bit) | (Screen[index + 1] & Memory[I + i] << (8 - x_bit));
            Screen[index] ^= Memory[I + i] >> x_bit;
            Screen[index + 1] ^= Memory[I + i] << (8 - x_bit);
        }
    } break;
    case 0xE:
        if ((instruction.word & 0xFF) == 0x9E)
        {
            if (keys[GPR[instruction.nibbles.n3]])
                PC += 2;
        }
        else if ((instruction.word & 0xFF) == 0xA1)
        {
            if (!keys[GPR[instruction.nibbles.n3]])
                PC += 2;
        }
        break;
    case 0xF:
        switch (instruction.word & 0xFF)
        {
        case 0x07:
            GPR[instruction.nibbles.n3] = DT;
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
            DT = GPR[instruction.nibbles.n3];
            break;
        case 0x18:
            ST = GPR[instruction.nibbles.n3];
            break;
        case 0x1E:
            I += GPR[instruction.nibbles.n3];
            break;
        case 0x29:
            I = GPR[instruction.nibbles.n3] * 5;
            break;
        case 0x33: {
            u8 n1 = GPR[instruction.nibbles.n3] % 10;
            u8 n2 = (GPR[instruction.nibbles.n3] / 10) % 10;
            u8 n3 = GPR[instruction.nibbles.n3] / 100;
            Memory[I] = n3;
            Memory[I + 1] = n2;
            Memory[I + 2] = n1;
        } break;
        case 0x55:
            for (u8 i = 0; i <= instruction.nibbles.n3; i++)
                Memory[I + i] = GPR[i];
            break;
        case 0x65:
            for (u8 i = 0; i <= instruction.nibbles.n3; i++)
                GPR[i] = Memory[I + i];
            break;
        default:
            assert(false);
        }
        break;
    default:
        assert(false && "Unhandled n4!");
    }
}
