#include "ADScene.hpp"

ADScene::ADScene()
{ clear(); }

void ADScene::clear()
{
    minVoxelX_ = 0;
    maxVoxelX_ = -1;
    minVoxelY_ = 0;
    maxVoxelY_ = -1;
    log2VoxelsWidth_ = 0;
    log2VoxelsHeight_ = 0;
    voxels_.reset();
    adVertices_.reset();
    adVerticesNumber_ = 0;
}

ADScene::Voxel const* ADScene::yAxisVoxels(uint16_t y) const
{ return voxels_.get() + voxelIndex(0, y); }

AD::Point3D const& ADScene::adVertex(uint16_t vertexIndex) const
{ return adVertices_[vertexIndex]; }

void ADScene::read(BufferedPsxRam const& psxRam, QByteArray const& psxVRam)
{
    clear();
    auto maxVertexIndex = readVoxels(psxRam);
    readVertices(psxRam, maxVertexIndex);
    rawVRam_ = psxVRam;
}

uint16_t ADScene::readVoxels(BufferedPsxRam const& psxRam)
{
    log2VoxelsWidth_ = psxRam.readWord(0x80083350) & 0x1f;
    log2VoxelsHeight_ = psxRam.readWord(0x80083352) & 0x1f;
    minVoxelX_ = 0x0;
    maxVoxelX_ = psxRam.readSWord(0x80083354);
    minVoxelY_ = 0x0;
    maxVoxelY_ = psxRam.readSWord(0x80083356);
    auto voxelsNumber = (maxVoxelY_ - minVoxelY_ + 1) << log2VoxelsWidth_;
    voxels_ = std::make_unique<Voxel[]>(voxelsNumber);
    auto const* adVoxels = psxRam.readAsPointer<AD::Voxel>(psxRam.readAddress(0x8008333c));
    uint16_t maxVertexIndex = 0;
    for (uint16_t voxelY = minVoxelY_; voxelY <= maxVoxelY_; ++voxelY)
    {
        auto const* adVoxelIt = adVoxels + (voxelY << log2VoxelsWidth_) + minVoxelX_;
        auto* voxelIt = voxels_.get() + ((voxelY - minVoxelY_) << log2VoxelsWidth_);
        for (uint16_t voxelX = minVoxelX_; voxelX <= maxVoxelX_; ++voxelX, ++adVoxelIt, ++voxelIt)
        { readVoxel(psxRam, *adVoxelIt, *voxelIt, maxVertexIndex); }
    }
    return maxVertexIndex;
}

void ADScene::readVoxel(BufferedPsxRam const& psxRam, AD::Voxel const& adVoxel, Voxel& voxel, uint16_t& maxVertexIndex)
{
    if (adVoxel.polygonsDescriptorsIndex == 0)
    { return; }
    voxel.drawVoxelPolygon = adVoxel.flags() != AD::VoxelFlags::DoNotDrawBackground;
    auto const* polygonsDescriptorsPtrArray = psxRam.readAsPointer<PsxRamAddress>(psxRam.readAddress(0x80083340));
    auto polygonDescriptorItAddress = polygonsDescriptorsPtrArray[adVoxel.polygonsDescriptorsIndex];
    auto const* polygonDescriptorIt = psxRam.readAsPointer<AD::PolygonDescriptor>(polygonDescriptorItAddress);
    while (polygonDescriptorIt != nullptr)
    {
        polygonDescriptorIt = moveToNextDrawablePolygon(polygonDescriptorIt);
        if (polygonDescriptorIt == nullptr)
        { continue; }
        voxel.polygonsDescriptors.append(*polygonDescriptorIt);
        maxVertexIndex = qMax(maxVertexIndex, maxPolygonVertexIndex(polygonDescriptorIt));
        polygonDescriptorIt = moveToNextPolygon(polygonDescriptorIt);
    }
}

AD::PolygonDescriptor const* ADScene::moveToNextDrawablePolygon(AD::PolygonDescriptor const* polygonDescriptorIt)
{
    while (polygonDescriptorIt->texCoord2AndTexPage.raw == 0)
    {
        if (polygonDescriptorIt->flags.lsb.raw == 1 && polygonDescriptorIt->flags.isLastPolygon())
        { return nullptr; }
        ++polygonDescriptorIt;
    }
    return polygonDescriptorIt;
}

AD::PolygonDescriptor const* ADScene::moveToNextPolygon(AD::PolygonDescriptor const* polygonDescriptorIt)
{
    if (
            polygonDescriptorIt->flags.lsb.fields.someSize == 1 &&
            polygonDescriptorIt->flags.isLastPolygon() &&
            polygonDescriptorIt->flags.lsb.fields.nextPolygonOffset == 0)
    { return nullptr; }
    return polygonDescriptorIt + 1;
}

void ADScene::readVertices(BufferedPsxRam const& psxRam, uint16_t maxVertexIndex)
{
    auto verticesAddress = psxRam.readAddress(0x80083344);
    adVerticesNumber_ = maxVertexIndex + 1;
    adVertices_ = std::make_unique<AD::Point3D[]>(adVerticesNumber_);
    psxRam.readRegion(
                {verticesAddress, static_cast<uint32_t>(sizeof(AD::Point3D)) * adVerticesNumber_},
                reinterpret_cast<uint8_t*>(adVertices_.get()));
}
