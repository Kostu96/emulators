#pragma once
#include "type_aliases.hpp"

#include <sstream>

struct DisassemblyLine
{
    u16 address;
    char buffer[30];
};

struct ErrorMessage
{
    u32 line;
    const char* message;
};

void printBytes(std::stringstream& ss, const u8* code, size_t& addr, u8 count, ...);

u8 digitCharToValue(char c);
