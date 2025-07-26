#ifndef NATIVECONTROLLER_H
#define NATIVECONTROLLER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimer>
#include <QKeyEvent>
#include <QShortcut>

#ifdef QT_GAMEPAD_ENABLED
#include <QGamepad>
#endif

class NativeController : public QObject
{
    Q_OBJECT

public:
    explicit NativeController(QObject *parent = nullptr);
    ~NativeController();

    // Start/stop the controller
    void startController();
    void stopController();
    
    // Enable/disable different control methods
    void enableKeyboardControl(bool enable);
    void enableGamepadControl(bool enable);
    void enableUdpClient(bool enable);
    void enableTcpClient(bool enable);
    
    // Set server address and ports
    void setServerAddress(const QString &address);
    void setUdpPort(quint16 port);
    void setTcpPort(quint16 port);
    
    // Send commands to server
    void sendButtonPress(const QString &button);
    void sendTouchCoordinate(int x, int y);
    void sendModeChange(bool autoMode);
    
    // Install event filter for global key events
    void installGlobalKeyFilter(QWidget *widget);

signals:
    // Local control signals (for UI)
    void directionPressed(const QString &direction);
    void modeTogglePressed();
    void touchCoordinateReceived(int x, int y);
    
    // Status signals
    void controllerStarted();
    void controllerStopped();
    void serverConnected();
    void serverDisconnected();
    void commandSent(const QString &command);
    void errorOccurred(const QString &error);

private slots:
    // TCP client control
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);
    void reconnectToServer();
    
    // Gamepad control
    void onGamepadConnected(int deviceId);
    void onGamepadDisconnected(int deviceId);
    void onGamepadButtonPressed(int button);
    void onGamepadAxisChanged(int axis, double value);
    
    // Keyboard shortcuts
    void onKeyboardShortcut();

private:
    // Network components
    QUdpSocket *m_udpSocket;
    QTcpSocket *m_tcpClient;
    QTimer *m_reconnectTimer;
    
    // Gamepad
#ifdef QT_GAMEPAD_ENABLED
    QGamepad *m_gamepad;
#endif
    
    // Keyboard shortcuts
    QList<QShortcut*> m_shortcuts;
    
    // Settings
    QString m_serverAddress;
    quint16 m_udpPort;
    quint16 m_tcpPort;
    bool m_keyboardEnabled;
    bool m_gamepadEnabled;
    bool m_udpEnabled;
    bool m_tcpEnabled;
    bool m_autoReconnect;
    
    // Helper methods
    void setupKeyboardShortcuts(QWidget *parent);
    void setupGamepad();
    void sendUdpCommand(const QString &command);
    void sendTcpCommand(const QString &command);
    void connectToServer();
};

#endif // NATIVECONTROLLER_H