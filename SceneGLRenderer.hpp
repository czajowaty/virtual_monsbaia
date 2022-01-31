#ifndef SCENEGLRENDERER_HPP
#define SCENEGLRENDERER_HPP

#include "ADScene.hpp"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>

struct VertexPlain
{
    QVector3D pos;
};


struct VertexTextured
{
    QVector3D pos;
    QVector2D texturePos;
    QVector3D texpage;
    QVector2D clut;
};

class SceneGLRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

    using Vertex = VertexTextured;

public:
    SceneGLRenderer(QWidget* parent = nullptr);
    ~SceneGLRenderer();

    bool isSceneLoaded() const
    { return vramTexture_ != nullptr; }
    void loadScene(ADScene const& adScene);
    QVector3D const& cameraPosition() const
    { return cameraPosition_; }
    QVector3D const& cameraFront() const
    { return cameraFront_; }
    float cameraYaw() const
    { return cameraYaw_; }
    float cameraPitch() const
    { return cameraPitch_; }
    void resetCamera();
    void increaseFieldOfView(float change);
    void decreaseFieldOfView(float change);
    void moveNonRotatedForward(float magnitude);
    void moveForward(float magnitude);
    void moveNonRotatedBackwards(float magnitude);
    void moveBackwards(float magnitude);
    void moveNonRotatedLeft(float magnitude);
    void moveLeft(float magnitude);
    void moveNonRotatedRight(float magnitude);
    void moveRight(float magnitude);
    void moveNonRotatedUp(float magnitude);
    void moveNonRotatedDown(float magnitude);
    void rotateYaw(float magnitude);
    void rotatePitch(float magnitude);
    void toggleDrawOpaques()
    { setDrawOpaques(!drawOpaques_); }
    void setDrawOpaques(bool enabled);
    void toggleDrawSemiTransparent()
    { setDrawSemiTransparent(!drawSemiTransparent_); }
    void setDrawSemiTransparent(bool enabled);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void clear();
    Vertex toVertex(
            AD::PolygonDescriptor const& polygonDescriptor,
            AD::Point3D const& adVertex,
            GpuTexCoord const& texCoord);
    void prepareBuffers(
            QVector<Vertex> const& vertices,
            QVector<GLuint> const& opaquePolygonsIndices,
            QVector<GLuint> const& semiTransparentPolygonsIndices);
    QVector3D cameraRight() const;
    void calculateCameraFront();
    void updateViewMatrix();
    void calculateProjectionMatrix();

    float aspectRatio_;
    QMatrix4x4 pMatrix_;
    QOpenGLShaderProgram shaderProgram_;
    QOpenGLVertexArrayObject opaquePolygonsVao_;
    QOpenGLVertexArrayObject semiTransparentPolygonsVao_;
    QOpenGLBuffer vbo_;
    QOpenGLBuffer opaquePolygonsEbo_;
    QOpenGLBuffer semiTransparentPolygonsEbo_;
    std::unique_ptr<QOpenGLTexture> vramTexture_;
    uint32_t opaquePolygonsIndicesNumber_;
    uint32_t semiTransparentPolygonsIndicesNumber_;
    QMatrix4x4 projectionMatrix_;
    int projectionMatrixLocation_;
    QMatrix4x4 viewMatrix_;
    int viewMatrixLocation_;
    float fieldOfView_;
    QVector3D cameraPosition_;
    QVector3D cameraFront_;
    QVector3D cameraUp_;
    float cameraYaw_;
    float cameraPitch_;
    bool drawOpaques_;
    bool drawSemiTransparent_;
};

#endif // SCENEGLRENDERER_HPP
