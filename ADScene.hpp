#ifndef ADSCENE_HPP
#define ADSCENE_HPP

#include "ADDefinitions.hpp"
#include "BufferedPsxRam.hpp"
#include <QVector>
#include <memory>

class ADScene
{
public:
    struct Voxel
    {
        bool drawVoxelPolygon;
        QVector<AD::PolygonDescriptor> polygonsDescriptors;
    };

    ADScene();

    uint32_t width() const
    { return (maxVoxelX_ - minVoxelX_) + 1; }
    uint32_t height() const
    { return (maxVoxelY_ -minVoxelY_) + 1; }
    Voxel const* yAxisVoxels(uint16_t y) const;
    AD::Point3D const& adVertex(uint16_t vertexIndex) const;
    void read(BufferedPsxRam const& psxRam, QByteArray const& psxVRam);
    QByteArray const& rawVRam() const
    { return rawVRam_; }

private:
    int voxelIndex(uint16_t x, uint16_t y) const
    { return x + (y << log2VoxelsWidth_); }
    void clear();
    uint16_t readVoxels(BufferedPsxRam const& psxRam);
    void readVoxel(BufferedPsxRam const& psxRam, AD::Voxel const& adVoxel, Voxel& voxel, uint16_t& maxVertexIndex);
    AD::PolygonDescriptor const* moveToNextDrawablePolygon(AD::PolygonDescriptor const* polygonDescriptorIt);
    AD::PolygonDescriptor const* moveToNextPolygon(AD::PolygonDescriptor const* polygonDescriptorIt);
    uint16_t maxPolygonVertexIndex(AD::PolygonDescriptor const* polygonDescriptorIt) const
    {
        return qMax(
                    qMax(polygonDescriptorIt->vertex1Index, polygonDescriptorIt->vertex2Index),
                    qMax(polygonDescriptorIt->vertex3Index, polygonDescriptorIt->vertex4Index));
    }
    void readVertices(BufferedPsxRam const& psxRam, uint16_t maxVertexIndex);

    int16_t minVoxelX_;
    int16_t maxVoxelX_;
    int16_t minVoxelY_;
    int16_t maxVoxelY_;
    uint8_t log2VoxelsWidth_;
    uint8_t log2VoxelsHeight_;
    std::unique_ptr<Voxel[]> voxels_;
    std::unique_ptr<AD::Point3D[]> adVertices_;
    uint32_t adVerticesNumber_;
    QByteArray rawVRam_;
};

#endif // ADSCENE_HPP
