#include "PsxVRamConst.hpp"
#include "SceneGLRenderer.hpp"
#include <QOpenGLPixelTransferOptions>
#include <cmath>

static constexpr float RAD = M_PI / 180.0f;

static constexpr float toRad(float angle)
{ return angle * RAD; }

SceneGLRenderer::SceneGLRenderer(QWidget* parent)
    : QOpenGLWidget(parent),
      aspectRatio_{1.0f},
      vbo_(QOpenGLBuffer::VertexBuffer),
      opaquePolygonsEbo_(QOpenGLBuffer::IndexBuffer),
      semiTransparentPolygonsEbo_(QOpenGLBuffer::IndexBuffer),
      opaquePolygonsIndicesNumber_{0},
      semiTransparentPolygonsIndicesNumber_{0},
      cameraPosition_(0.0f, 0.5f, -1.5f),
      cameraFront_(0.0f, 0.0f, -1.0f),
      cameraUp_(0.0f, 1.0f, 0.0f),
      cameraYaw_{toRad(90.0f)},
      cameraPitch_{toRad(-20.0f)},
      drawOpaques_{true},
      drawSemiTransparent_{true}
{ resetCamera(); }

SceneGLRenderer::~SceneGLRenderer()
{
    makeCurrent();
    clear();
    doneCurrent();
}

void SceneGLRenderer::clear()
{
    if (vbo_.isCreated())
    { vbo_.destroy(); }
    if (opaquePolygonsEbo_.isCreated())
    { opaquePolygonsEbo_.destroy(); }
    if (opaquePolygonsVao_.isCreated())
    { opaquePolygonsVao_.destroy(); }
    if (semiTransparentPolygonsVao_.isCreated())
    { semiTransparentPolygonsVao_.destroy(); }
    if (vramTexture_ != nullptr)
    {
        if (vramTexture_->isCreated())
        { vramTexture_->destroy(); }
        vramTexture_.reset();
    }
}

AD::Point3D operator+(AD::Point3D const& one, AD::Point3D const& other)
{
    AD::Point3D result;
    result.x = one.x + other.x;
    result.y = one.y + other.y;
    result.z = one.z + other.z;
    return result;
}

QVector3D toVector3D(AD::Point3D const& adVertex)
{
    static constexpr int FRACTIONAL_SIZE = 12;
    static constexpr float DENOMINATOR = 1 << FRACTIONAL_SIZE;
    return QVector3D(-adVertex.x / DENOMINATOR, -adVertex.z / DENOMINATOR, -adVertex.y / DENOMINATOR);
}

void SceneGLRenderer::loadScene(ADScene const& adScene)
{
    auto addPolygonIndices = [](QVector<GLuint>& indices, GLuint firstVertexIndex) {
        indices.append(firstVertexIndex + 0);
        indices.append(firstVertexIndex + 1);
        indices.append(firstVertexIndex + 2);
        indices.append(firstVertexIndex + 2);
        indices.append(firstVertexIndex + 1);
        indices.append(firstVertexIndex + 3);
    };

    clear();
    QVector<Vertex> vertices;
    QVector<GLuint> opaquePolygonsIndices;
    QVector<GLuint> semiTransparentPolygonsIndices;
    AD::Point3D voxelTranslation;
    voxelTranslation.y = -0x20 * adScene.height();
    voxelTranslation.z = 0;
    GLuint nextVertexIndex = 0;
    for (auto voxelY = 0u; voxelY < adScene.height(); ++voxelY)
    {
        auto const* voxelIt = adScene.yAxisVoxels(voxelY);
        voxelTranslation.x = -0x20 * adScene.width();
        for (auto voxelX = 0u; voxelX < adScene.width(); ++voxelX, ++voxelIt)
        {
            auto const& voxel = *voxelIt;
            auto const* polygonDescriptorIt = voxel.polygonsDescriptors.begin();
            while (polygonDescriptorIt != voxel.polygonsDescriptors.end())
            {
                auto const& polygonDescriptor = *polygonDescriptorIt;
                auto const& adVertex1 = adScene.adVertex(polygonDescriptor.vertex1Index);
                auto const& adVertex2 = adScene.adVertex(polygonDescriptor.vertex2Index);
                auto const& adVertex3 = adScene.adVertex(polygonDescriptor.vertex3Index);
                auto const& adVertex4 = adScene.adVertex(polygonDescriptor.vertex4Index);
                vertices.append(
                            toVertex(polygonDescriptor, adVertex1 + voxelTranslation, polygonDescriptor.texCoord1));
                vertices.append(
                            toVertex(polygonDescriptor, adVertex2 + voxelTranslation, polygonDescriptor.texCoord2()));
                vertices.append(
                            toVertex(polygonDescriptor, adVertex3 + voxelTranslation, polygonDescriptor.texCoord3));
                vertices.append(
                            toVertex(polygonDescriptor, adVertex4 + voxelTranslation, polygonDescriptor.texCoord4));

                addPolygonIndices(
                            polygonDescriptor.flags.isSemiTransparent() ?
                                semiTransparentPolygonsIndices :
                                opaquePolygonsIndices,
                            nextVertexIndex);
                ++polygonDescriptorIt;
                nextVertexIndex += 4;
            }
            voxelTranslation.x += 0x40;
        }
        voxelTranslation.y += 0x40;
    }
    prepareBuffers(vertices, opaquePolygonsIndices, semiTransparentPolygonsIndices);
    makeCurrent();
    vramTexture_ = std::make_unique<QOpenGLTexture>(QOpenGLTexture::TargetRectangle);
    vramTexture_->setFormat(QOpenGLTexture::RG8U);
    vramTexture_->setSize(PsxVRamConst::PIXELS_PER_LINE, PsxVRamConst::HEIGHT);
    vramTexture_->setMinificationFilter(QOpenGLTexture::Linear);
    vramTexture_->setMagnificationFilter(QOpenGLTexture::Linear);
    vramTexture_->allocateStorage(QOpenGLTexture::RG, QOpenGLTexture::UInt8);
    QOpenGLPixelTransferOptions transferOptions;
    transferOptions.setAlignment(1);
    vramTexture_->setData(
                0,
                QOpenGLTexture::RG_Integer,
                QOpenGLTexture::UInt8,
                adScene.rawVRam().constData(),
                &transferOptions);
    shaderProgram_.bind();
    shaderProgram_.setUniformValue("vramSampler", 0);
    shaderProgram_.release();
    doneCurrent();
    update();
}

SceneGLRenderer::Vertex SceneGLRenderer::toVertex(
        AD::PolygonDescriptor const& polygonDescriptor,
        AD::Point3D const& adVertex,
        GpuTexCoord const& texCoord)
{
    SceneGLRenderer::Vertex vertex;
    vertex.pos = toVector3D(adVertex);
    auto const& gpuTexpage = polygonDescriptor.texCoord2AndTexPage.fields.texpage;
    auto const& gpuClut = polygonDescriptor.clut;
    vertex.texturePos = QVector2D(texCoord.x, texCoord.y);
    vertex.texpage = QVector3D(gpuTexpage.x, gpuTexpage.y, gpuTexpage.texpageBpp);
    vertex.clut = QVector2D(gpuClut.x, gpuClut.y);
    return vertex;
}

void SceneGLRenderer::prepareBuffers(
        QVector<Vertex> const& vertices,
        QVector<GLuint> const& opaquePolygonsIndices,
        QVector<GLuint> const& semiTransparentPolygonsIndices)
{
    makeCurrent();
    shaderProgram_.bind();
    vbo_.create();
    vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo_.bind();
    vbo_.allocate(vertices.constData(), vertices.count() * sizeof(Vertex));
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&opaquePolygonsVao_);
        shaderProgram_.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, pos), 3, sizeof(Vertex));
        shaderProgram_.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, texturePos), 2, sizeof(Vertex));
        shaderProgram_.setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, texpage), 3, sizeof(Vertex));
        shaderProgram_.setAttributeBuffer(3, GL_FLOAT, offsetof(Vertex, clut), 2, sizeof(Vertex));
        shaderProgram_.enableAttributeArray(0);
        shaderProgram_.enableAttributeArray(1);
        shaderProgram_.enableAttributeArray(2);
        shaderProgram_.enableAttributeArray(3);
        opaquePolygonsEbo_.create();
        opaquePolygonsEbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
        opaquePolygonsEbo_.bind();
        opaquePolygonsIndicesNumber_ = opaquePolygonsIndices.count();
        opaquePolygonsEbo_.allocate(opaquePolygonsIndices.constData(), opaquePolygonsIndicesNumber_ * sizeof(GLuint));
    }
    opaquePolygonsEbo_.release();
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&semiTransparentPolygonsVao_);
        shaderProgram_.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, pos), 3, sizeof(Vertex));
        shaderProgram_.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, texturePos), 2, sizeof(Vertex));
        shaderProgram_.setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, texpage), 3, sizeof(Vertex));
        shaderProgram_.setAttributeBuffer(3, GL_FLOAT, offsetof(Vertex, clut), 2, sizeof(Vertex));
        shaderProgram_.enableAttributeArray(0);
        shaderProgram_.enableAttributeArray(1);
        shaderProgram_.enableAttributeArray(2);
        shaderProgram_.enableAttributeArray(3);
        semiTransparentPolygonsEbo_.create();
        semiTransparentPolygonsEbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
        semiTransparentPolygonsEbo_.bind();
        semiTransparentPolygonsIndicesNumber_ = semiTransparentPolygonsIndices.count();
        semiTransparentPolygonsEbo_.allocate(
                    semiTransparentPolygonsIndices.constData(),
                    semiTransparentPolygonsIndicesNumber_ * sizeof(GLuint));
    }
    semiTransparentPolygonsEbo_.release();
    vbo_.release();
    shaderProgram_.release();
    doneCurrent();
}

void SceneGLRenderer::resetCamera()
{
    fieldOfView_ = 45.0f;
    calculateProjectionMatrix();
    cameraPosition_ = QVector3D(0.0f, 0.5f, -1.5f),
    cameraFront_ = QVector3D(0.0f, 0.0f, -1.0f),
    cameraUp_ = QVector3D(0.0f, 1.0f, 0.0f),
    cameraYaw_ = toRad(90.0f),
    cameraPitch_ = toRad(-20.0f);
    calculateCameraFront();
    updateViewMatrix();
}

void SceneGLRenderer::increaseFieldOfView(float change)
{
    static constexpr float MAX_FIELD_OF_VIEW = 150.0f;
    if (fieldOfView_ >= MAX_FIELD_OF_VIEW)
    { return; }
    fieldOfView_ += change;
    if (fieldOfView_ > MAX_FIELD_OF_VIEW)
    { fieldOfView_ = MAX_FIELD_OF_VIEW; }
    calculateProjectionMatrix();
    update();
}

void SceneGLRenderer::decreaseFieldOfView(float change)
{
    static constexpr float MIN_FIELD_OF_VIEW = 2.0f;
    if (fieldOfView_ <= MIN_FIELD_OF_VIEW)
    { return; }
    fieldOfView_ -= change;
    if (fieldOfView_ < MIN_FIELD_OF_VIEW)
    { fieldOfView_ = MIN_FIELD_OF_VIEW; }
    calculateProjectionMatrix();
    update();
}

void SceneGLRenderer::moveNonRotatedForward(float magnitude)
{
    cameraPosition_ += QVector3D(0.0f, 0.0f, 1.0f * magnitude);
    updateViewMatrix();
}

void SceneGLRenderer::moveForward(float magnitude)
{
    cameraPosition_ += cameraFront_ * magnitude;
    updateViewMatrix();
}

void SceneGLRenderer::moveNonRotatedBackwards(float magnitude)
{
    cameraPosition_ += QVector3D(0.0f, 0.0f, -1.0f * magnitude);
    updateViewMatrix();
}

void SceneGLRenderer::moveBackwards(float magnitude)
{
    cameraPosition_ -= cameraFront_ * magnitude;
    updateViewMatrix();
}

void SceneGLRenderer::moveNonRotatedLeft(float magnitude)
{
    cameraPosition_ += QVector3D(1.0f * magnitude, 0.0f, 0.0f);
    updateViewMatrix();
}

void SceneGLRenderer::moveLeft(float magnitude)
{
    cameraPosition_ -= cameraRight() * magnitude;
    updateViewMatrix();
}

void SceneGLRenderer::moveNonRotatedRight(float magnitude)
{
    cameraPosition_ += QVector3D(-1.0f * magnitude, 0.0f, 0.0f);
    updateViewMatrix();
}

void SceneGLRenderer::moveRight(float magnitude)
{
    cameraPosition_ += cameraRight() * magnitude;
    updateViewMatrix();
}

void SceneGLRenderer::moveNonRotatedUp(float magnitude)
{
    cameraPosition_ += QVector3D(0.0f, 1.0f * magnitude, 0.0f);
    updateViewMatrix();
}

void SceneGLRenderer::moveNonRotatedDown(float magnitude)
{
    cameraPosition_ += QVector3D(0.0f, -1.0f * magnitude, 0.0f);
    updateViewMatrix();
}

QVector3D SceneGLRenderer::cameraRight() const
{
    auto rightVector = QVector3D::crossProduct(cameraFront_, cameraUp_);
    rightVector.normalize();
    return rightVector;
}

void SceneGLRenderer::rotateYaw(float yawChange)
{
    cameraYaw_ += yawChange;
    calculateCameraFront();
    updateViewMatrix();
}

void SceneGLRenderer::rotatePitch(float pitchChange)
{
    static constexpr float MIN_PITCH = toRad(-89.0f);
    static constexpr float MAX_PITCH = toRad(89.0f);
    cameraPitch_ += pitchChange;
    if (cameraPitch_ < MIN_PITCH)
    { cameraPitch_ = MIN_PITCH; }
    else if (cameraPitch_ > MAX_PITCH)
    { cameraPitch_ = MAX_PITCH; }
    calculateCameraFront();
    updateViewMatrix();
}

void SceneGLRenderer::calculateCameraFront()
{
    cameraFront_.setX(cos(cameraYaw_) * cos(cameraPitch_));
    cameraFront_.setY(sin(cameraPitch_));
    cameraFront_.setZ(sin(cameraYaw_) * cos(cameraPitch_));
    cameraFront_.normalize();
}

void SceneGLRenderer::updateViewMatrix()
{
    viewMatrix_.setToIdentity();
    viewMatrix_.lookAt(cameraPosition_, cameraPosition_ + cameraFront_, cameraUp_);
    update();
}

void SceneGLRenderer::setDrawOpaques(bool enabled)
{
    drawOpaques_ = enabled;
    update();
}
void SceneGLRenderer::setDrawSemiTransparent(bool enabled)
{
    drawSemiTransparent_ = enabled;
    update();
}

void SceneGLRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragmentShader.fsh");
    shaderProgram_.link();
    shaderProgram_.bind();
    projectionMatrixLocation_ = shaderProgram_.uniformLocation("projectionMatrix");
    viewMatrixLocation_ = shaderProgram_.uniformLocation("viewMatrix");
    shaderProgram_.release();
}

void SceneGLRenderer::resizeGL(int w, int h)
{
    aspectRatio_ = static_cast<float>(w) / h;
    calculateProjectionMatrix();
}

void SceneGLRenderer::calculateProjectionMatrix()
{
    projectionMatrix_.setToIdentity();
    projectionMatrix_.perspective(fieldOfView_, aspectRatio_, 0.001f, 100.0f);
}

void SceneGLRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!isSceneLoaded())
    { return; }
    vramTexture_->bind(0);
    shaderProgram_.bind();
    shaderProgram_.setUniformValue(projectionMatrixLocation_, projectionMatrix_);
    shaderProgram_.setUniformValue(viewMatrixLocation_, viewMatrix_);
    if (drawOpaques_)
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&opaquePolygonsVao_);
        glDrawElements(GL_TRIANGLES, opaquePolygonsIndicesNumber_, GL_UNSIGNED_INT, nullptr);
    }
    if (drawSemiTransparent_)
    {
        glEnable(GL_BLEND);
        QOpenGLVertexArrayObject::Binder vaoBinder(&semiTransparentPolygonsVao_);
        glDrawElements(GL_TRIANGLES, semiTransparentPolygonsIndicesNumber_, GL_UNSIGNED_INT, nullptr);
        glDisable(GL_BLEND);
    }
    vramTexture_->release(0);
    shaderProgram_.release();
}
