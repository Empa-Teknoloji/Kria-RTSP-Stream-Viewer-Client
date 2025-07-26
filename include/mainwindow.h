#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVector>
#include "rtspstreamer.h"
#include "distancemap.h"
#include "nativecontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Handle key press events (Esc to exit fullscreen)
    void keyPressEvent(QKeyEvent *event) override;
    
    // Handle resize events to reposition the buttons
    void resizeEvent(QResizeEvent *event) override;
    
    // Handle mouse press events for touch coordinates
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateFrame(const QImage &frame);
    void handleConnectionError();
    void connectToStream();
    void disconnectFromStream();
    void toggleAutoManual();
    void updateButtonsPosition();
    void arrowButtonClicked();

private:
    Ui::MainWindow *ui;
    RTSPStreamer *m_rtspStreamer;
    QLabel *m_videoLabel;
    QPushButton *m_toggleButton;
    QVector<QPushButton*> m_arrowButtons;
    DistanceMap *m_distanceMap;
    NativeController *m_nativeController;
    QString m_tcpAddress = "192.168.10.102";  // Use localhost for testing
    quint16 m_rtspPort = 554;
    quint16 m_tcpPort = 8080;
    quint16 m_udpPort = 8081;
    QString m_rtspUrl="rtsp://192.168.10.102:554/test";
    bool m_isAutoMode = true; // Start in AUTO mode
    
    void setupUI();
    void setupNativeController();
    void updateButtonStyle();
    void updateArrowButtonsVisibility();
    QPushButton* createArrowButton(const QString& direction);
    
    // Network configuration
    void setNetworkConfiguration(const QString &address, quint16 rtspPort, quint16 tcpPort, quint16 udpPort);
    
    // Native controller message handlers
    void handleDirectionPress(const QString &direction);
    void handleModeToggle();
    void handleTouchCoordinate(int x, int y);
    
    // Coordinate normalization
    QPointF normalizeCoordinates(const QPoint &screenCoord) const;
};
#endif // MAINWINDOW_H
