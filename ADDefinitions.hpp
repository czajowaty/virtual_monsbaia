#ifndef ADDEFINITIONS_HPP
#define ADDEFINITIONS_HPP

#include "GpuTypes.hpp"
#include <cstdint>

namespace AD
{

enum VoxelFlags : uint8_t
{
    DoNotDrawBackground = 0,
    Unk_01 = 1,
    Unk_02 = 2,
    DrawMonochromeOpaquePolygons = 3
};

struct Voxel
{
    uint16_t polygonsDescriptorsIndex : 14;
    uint16_t flags_ : 2;

    VoxelFlags flags() const
    { return static_cast<VoxelFlags>(flags_); }
};

struct PolygonDescriptorFlags
{
    union
    {
        uint8_t raw;
        struct
        {
            uint8_t someSize : 4;
            uint8_t nextPolygonOffset : 4;
        } fields;
    } lsb;
    uint8_t semiTransparencyFlag_ : 1;
    uint8_t unk_01 : 6;
    uint8_t lastPolygonFlag_ : 1;

    bool isSemiTransparent() const
    { return semiTransparencyFlag_ != 0; }
    bool isLastPolygon() const
    { return lastPolygonFlag_ != 0; }
};

struct PolygonDescriptor
{
    uint16_t vertex1Index;
    uint16_t vertex2Index;
    uint16_t vertex3Index;
    uint16_t vertex4Index;
    GpuTexCoord texCoord1;
    GpuClut clut;
    union
    {
        uint32_t raw;
        struct
        {
            GpuTexCoord texCoord2;
            GpuTexpage texpage;
        } fields;
    } texCoord2AndTexPage;
    uint16_t normalVectorIndex;
    GpuTexCoord texCoord3;
    GpuTexCoord texCoord4;
    PolygonDescriptorFlags flags;

    GpuTexCoord const& texCoord2() const
    { return texCoord2AndTexPage.fields.texCoord2; }
};

struct Point3D
{
    int16_t x;
    int16_t y;
    int16_t z;
    uint16_t padding;
};

}

#endif // ADDEFINITIONS_HPP
