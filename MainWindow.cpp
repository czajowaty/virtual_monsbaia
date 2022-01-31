#include "MainWindow.hpp"
#include "PsxRamConst.hpp"
#include "PsxVRamConst.hpp"
#include "ui_MainWindow.h"
#include <QDateTime>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <cmath>
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    psxRam_ = std::make_unique<BufferedPsxRam>();
    ui->setupUi(this);
    connect(
                &keyboardControlsTimer_, &QTimer::timeout,
                this, &MainWindow::onKeyboardControlsTimerTimeout);
    keyboardControlsTimer_.start(5);
    keyboardControlsTimerLastExecutionMs_ = QDateTime::currentMSecsSinceEpoch();
    setFocus();
}

MainWindow::~MainWindow()
{
    keyboardControlsTimer_.stop();
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_1:
        ui->sceneRenderOpenGLWidget->toggleDrawOpaques();
        break;
    case Qt::Key_2:
        ui->sceneRenderOpenGLWidget->toggleDrawSemiTransparent();
        break;
    case Qt::Key_R:
        ui->sceneRenderOpenGLWidget->resetCamera();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        return;
    }
}
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        isTrackingMouse_ = true;
        lastMousePosition_ = event->pos();
    }
    else
    { QMainWindow::mousePressEvent(event); }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        isTrackingMouse_ = false;
        event->accept();
    }
    else
    { QMainWindow::mouseReleaseEvent(event); }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (isTrackingMouse_)
    {
        auto mousePositionChange = event->pos() - lastMousePosition_;
        if (mousePositionChange.x() != 0)
        {
            auto xChangeRatio = static_cast<float>(mousePositionChange.x()) / ui->sceneRenderOpenGLWidget->width();
            ui->sceneRenderOpenGLWidget->rotateYaw(M_PI * xChangeRatio);
        }
        if (mousePositionChange.y() != 0)
        {
            auto yChangeRatio = static_cast<float>(mousePositionChange.y()) / ui->sceneRenderOpenGLWidget->height();
            ui->sceneRenderOpenGLWidget->rotatePitch(-M_PI * yChangeRatio);
        }
        lastMousePosition_ = event->pos();
    }
    else
    { QMainWindow::mouseMoveEvent(event); }
}

void MainWindow::on_action_Open_triggered()
{
    auto filePath = QFileDialog::getOpenFileName(this, "Open AD 3D model", {}, "AD 3D models (*.3dm)");
    if (filePath.isNull())
    { return; }
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this,  "Read AD 3D model error",  QString("Could not open file %1.").arg(filePath));
        return;
    }
    auto fileContent = file.readAll();
    int expectedSize = PsxRamConst::SIZE + PsxVRamConst::SIZE;
    if (fileContent.size() != expectedSize)
    {
        QMessageBox::warning(
                    this,
                    "Read AD 3D model error",
                    QString("File size %1 is different than expected %2.")
                    .arg(fileContent.size())
                    .arg(expectedSize));
        return;
    }
    psxRam_->fill(fileContent.data());
    psxVRam_ = QByteArray(fileContent.data() + PsxRamConst::SIZE, PsxVRamConst::SIZE);
    adScene_.read(*psxRam_, psxVRam_);
    ui->sceneRenderOpenGLWidget->loadScene(adScene_);
    ui->sceneRenderOpenGLWidget->resetCamera();
}

bool isKeyPressed(int key)
{ return GetAsyncKeyState(key) < 0; }

void MainWindow::onKeyboardControlsTimerTimeout()
{
    auto timeElapsed = QDateTime::currentMSecsSinceEpoch() - keyboardControlsTimerLastExecutionMs_;
    keyboardControlsTimerLastExecutionMs_ = QDateTime::currentMSecsSinceEpoch();
    if (!hasFocus())
    { return; }
    static constexpr float VECTOR_CHANGE_PER_SEC = 1.0f;
    float timeElapsedFactor = (timeElapsed / 1000.0f);
    float movementSpeed = VECTOR_CHANGE_PER_SEC * timeElapsedFactor * (isKeyPressed(VK_CONTROL) ? 1.0f : 0.1f);
    static constexpr float FOV_CHANGE_PER_SEC = 30.0f;
    float fovChange = FOV_CHANGE_PER_SEC * timeElapsedFactor;
    if (isKeyPressed(Qt::Key_W))
    { ui->sceneRenderOpenGLWidget->moveForward(movementSpeed); }
    else if (isKeyPressed(Qt::Key_I))
    { ui->sceneRenderOpenGLWidget->moveNonRotatedForward(movementSpeed); }
    if (isKeyPressed(Qt::Key_S))
    { ui->sceneRenderOpenGLWidget->moveBackwards(movementSpeed); }
    else if (isKeyPressed(Qt::Key_K))
    { ui->sceneRenderOpenGLWidget->moveNonRotatedBackwards(movementSpeed); }
    if (isKeyPressed(Qt::Key_A))
    { ui->sceneRenderOpenGLWidget->moveLeft(movementSpeed); }
    else if (isKeyPressed(Qt::Key_J))
    { ui->sceneRenderOpenGLWidget->moveNonRotatedLeft(movementSpeed); }
    if (isKeyPressed(Qt::Key_D))
    { ui->sceneRenderOpenGLWidget->moveRight(movementSpeed); }
    else if (isKeyPressed(Qt::Key_L))
    { ui->sceneRenderOpenGLWidget->moveNonRotatedRight(movementSpeed); }
    if (isKeyPressed(Qt::Key_U))
    { ui->sceneRenderOpenGLWidget->moveNonRotatedUp(movementSpeed); }
    if (isKeyPressed(Qt::Key_O))
    { ui->sceneRenderOpenGLWidget->moveNonRotatedDown(movementSpeed); }
    if (isKeyPressed(Qt::Key_Z))
    { ui->sceneRenderOpenGLWidget->decreaseFieldOfView(fovChange); }
    if (isKeyPressed(Qt::Key_X))
    { ui->sceneRenderOpenGLWidget->increaseFieldOfView(fovChange); }
}
