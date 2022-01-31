#ifndef GPUTYPES_HPP
#define GPUTYPES_HPP

#include <cstdint>

enum class GpuTexpageBpp : uint16_t
{
    BPP_4 = 0,
    BPP_8 = 1,
    BPP_15 = 2,
    BPP_15_3 = 3
};

struct GpuTexpage
{
    uint16_t x : 4;
    uint16_t y : 1;
    uint16_t semiTransparency : 2;
    uint16_t texpageBpp : 2;
    uint16_t unused_1 : 2;
    uint16_t textureDisable : 1;
    uint16_t unused_2 : 4;

    GpuTexpageBpp bpp() const
    { return static_cast<GpuTexpageBpp>(texpageBpp); }
};

struct GpuTexCoord
{
    uint8_t x;
    uint8_t y;
};

struct GpuClut
{
    uint16_t x : 6;
    uint16_t y : 9;
    uint16_t zero : 1;
};

#endif // GPUTYPES_HPP
