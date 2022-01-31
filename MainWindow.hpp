#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ADScene.hpp"
#include "BufferedPsxRam.hpp"
#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

private slots:
    void on_action_Open_triggered();
    void onKeyboardControlsTimerTimeout();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<BufferedPsxRam> psxRam_;
    QByteArray psxVRam_;
    ADScene adScene_;
    QTimer keyboardControlsTimer_;
    qint64 keyboardControlsTimerLastExecutionMs_;
    bool isTrackingMouse_{false};
    QPoint lastMousePosition_;
};
#endif // MAINWINDOW_HPP
