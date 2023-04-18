#pragma once

#include <cstdint>
#include <fstream>
#include <cstring>
#include <string>

std::string RegionSpaceToString(uint8_t RegionSpace)
{
    switch (RegionSpace)
    {
    case 0x00: return "SystemMemory";
    case 0x01: return "SystemIO";
    case 0x02: return "PCIConfig";
    default: return "OEM";
    }
}

extern uint8_t ReadByte(std::ifstream& file);

std::string GetInteger(std::ifstream& file)
{
    uint8_t type = ReadByte(file);

    switch (type)
    {
    default:
        printf("Unknown TermArg integer type 0x%02x\n", type);
        exit(1);
    }
}