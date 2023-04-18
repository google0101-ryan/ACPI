#include <cstdint>
#include <fstream>
#include <cstring>
#include <vector>
#include <tuple>
#include <util.h>

std::vector<std::pair<uint64_t, uint64_t>> pkgs;

int global_tabs = 0;

void PrintTabs()
{
    for (int i = 0; i < global_tabs; i++)
        printf("\t");
}

uint8_t ReadByte(std::ifstream& file)
{
    for (auto& i : pkgs)
        i.first++;
    
    if (pkgs.back().first >= pkgs.back().second)
    {
        global_tabs--;
        PrintTabs();
        printf("}\n");
        pkgs.pop_back();
    }
    
    uint8_t byte = 0;
    file.read((char*)&byte, 1);
    return byte;
}

struct Header
{
    char TableSig[4];
    uint32_t TableLength;
    uint8_t SpecCompliance;
    uint8_t Checksum;
    char OemID[6];
    char OemTableID[8];
    uint32_t OemRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

int opcodes_read = 0;

void ReadPkgLength(std::ifstream& file)
{
    uint8_t leadByte = ReadByte(file);

    uint64_t len = leadByte & 0x1F;

    if (((leadByte >> 6) & 0x3) != 0)
    {
        for (int i = 0; i < ((leadByte >> 6) & 0x3); i++)
        {
            len |= ReadByte(file) << (i*8)+4;
        }
    }

    pkgs.push_back(std::make_pair(opcodes_read, len));
}

std::string ReadNameString(std::ifstream& file)
{
    char leadChar = ReadByte(file);

    std::string name;

    if (leadChar == '^') // PrefixPath
        name.push_back('^');
    else if (leadChar == '\\')
        name.push_back('\\');
    else
        name.push_back(leadChar);
    
    uint8_t maybeType = ReadByte(file);

    if (maybeType == 0x2E || maybeType == 0x2F)
    {
        printf("TODO: MultiNamePath/DualNamePath");
        exit(1);
    }
    else if (maybeType == 0);
    else
    {
        name.push_back(maybeType);
        for (int i = 0; i < 2; i++)
            name.push_back(ReadByte(file));
    }

    return name;
}

void ParseScopeOp(std::ifstream& file)
{
    ReadPkgLength(file);

    auto name = ReadNameString(file);

    PrintTabs();
    printf("Scope (%s)\n", name.c_str());
    PrintTabs();
    printf("{\n");
    global_tabs++;
}

void ParseRegion(std::ifstream& file)
{
    auto name = ReadNameString(file);

    uint8_t regionSpace = ReadByte(file);

    auto RegionOffset = GetInteger(file);
    auto RegionLen = GetInteger(file);

    PrintTabs();
    printf("OperationRegion (%s, %s, %s, %s)\n", name.c_str(), RegionSpaceToString(regionSpace).c_str(), RegionOffset.c_str(), RegionLen.c_str());

    exit(1);
}

void SecondaryOp(std::ifstream& file)
{
    uint8_t opcode = ReadByte(file);
    opcodes_read++;

    switch (opcode)
    {
    case 0x80:
        ParseRegion(file);
        break;
    default:
        printf("Unknown secondary op 0x%02x\n", opcode);
        exit(1);
    }
}

int main()
{
    std::ifstream file("dsdt.aml", std::ios::binary);

    Header hdr;
    file.read((char*)&hdr, sizeof(Header));

    char tableType[5] = {0};
    strncpy(tableType, hdr.TableSig, 4);
    char oemID[7] = {0};
    strncpy(oemID, hdr.OemID, 6);
    char oemCreatorID[9] = {0};
    strncpy(oemCreatorID, hdr.OemTableID, 8);

    // Following the header, we begin our opcode stream
    printf("DefinitionBlock (\"\", \"%s\", %d, \"%s\", \"%s\", 0x%08x)\n{\n", tableType, hdr.SpecCompliance, oemID, oemCreatorID, hdr.OemRevision);
    global_tabs++;
    pkgs.push_back(std::make_pair(sizeof(hdr), hdr.TableLength));

    while (!pkgs.empty())
    {
        opcodes_read = 0;
        uint8_t opcode = ReadByte(file);
        opcodes_read++;

        switch (opcode)
        {
        case 0x10:
            ParseScopeOp(file);
            break;
        case 0x5b:
            SecondaryOp(file);
            break;
        default:
            printf("Unknown ACPI opcode 0x%02x\n", opcode);
            exit(1);
        }
    }
}