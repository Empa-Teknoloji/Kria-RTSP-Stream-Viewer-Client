#include "nativecontroller.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QWidget>

NativeController::NativeController(QObject *parent)
    : QObject(parent)
    , m_udpSocket(nullptr)
    , m_tcpClient(nullptr)
    , m_reconnectTimer(nullptr)
#ifdef QT_GAMEPAD_ENABLED
    , m_gamepad(nullptr)
#endif
    , m_serverAddress("192.168.1.71")
    , m_udpPort(8556)
    , m_tcpPort(8555)
    , m_keyboardEnabled(true)
    , m_gamepadEnabled(true)
    , m_udpEnabled(true)
    , m_tcpEnabled(false)
    , m_autoReconnect(true)
{
    // Initialize UDP socket for sending commands
    m_udpSocket = new QUdpSocket(this);
    
    // Initialize TCP client for persistent connection
    m_tcpClient = new QTcpSocket(this);
    connect(m_tcpClient, &QTcpSocket::connected, this, &NativeController::onTcpConnected);
    connect(m_tcpClient, &QTcpSocket::disconnected, this, &NativeController::onTcpDisconnected);
    connect(m_tcpClient, &QTcpSocket::errorOccurred, this, &NativeController::onTcpError);
    
    // Setup reconnect timer
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(5000); // Try reconnect every 5 seconds
    connect(m_reconnectTimer, &QTimer::timeout, this, &NativeController::reconnectToServer);
    
    // Setup gamepad
    setupGamepad();
}

NativeController::~NativeController()
{
    stopController();
}

void NativeController::startController()
{
    qDebug() << "Starting Native Controller as CLIENT";
    qDebug() << "  Server Address:" << m_serverAddress;
    qDebug() << "  UDP Port:" << m_udpPort;
    qDebug() << "  TCP Port:" << m_tcpPort;
    
    if (m_tcpEnabled) {
        connectToServer();
    }
    
    emit controllerStarted();
}

void NativeController::stopController()
{
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
    }
    
    if (m_tcpClient && m_tcpClient->state() == QTcpSocket::ConnectedState) {
        m_tcpClient->disconnectFromHost();
    }
    
    emit controllerStopped();
}

void NativeController::enableKeyboardControl(bool enable)
{
    m_keyboardEnabled = enable;
}

void NativeController::enableGamepadControl(bool enable)
{
    m_gamepadEnabled = enable;
#ifdef QT_GAMEPAD_ENABLED
    if (m_gamepad) {
        m_gamepad->setEnabled(enable);
    }
#else
    Q_UNUSED(enable)
    qDebug() << "Gamepad support not available - compiled without Qt Gamepad";
#endif
}

void NativeController::enableUdpClient(bool enable)
{
    m_udpEnabled = enable;
}

void NativeController::enableTcpClient(bool enable)
{
    m_tcpEnabled = enable;
    if (enable && m_tcpClient->state() == QTcpSocket::UnconnectedState) {
        connectToServer();
    } else if (!enable && m_tcpClient->state() == QTcpSocket::ConnectedState) {
        m_tcpClient->disconnectFromHost();
    }
}

void NativeController::setServerAddress(const QString &address)
{
    m_serverAddress = address;
    qDebug() << "Server address set to:" << m_serverAddress;
}

void NativeController::setUdpPort(quint16 port)
{
    m_udpPort = port;
}

void NativeController::setTcpPort(quint16 port)
{
    m_tcpPort = port;
}

void NativeController::sendButtonPress(const QString &button)
{
    QString command = QString("BUTTON:%1").arg(button);
    
    if (m_udpEnabled) {
        sendUdpCommand(command);
    }
    
    if (m_tcpEnabled) {
        sendTcpCommand(command);
    }
    
    emit commandSent(command);
    qDebug() << "Sent button press:" << button;
}

void NativeController::sendTouchCoordinate(int x, int y)
{
    QString command = QString("TOUCH:%1:%2").arg(x).arg(y);
    
    if (m_udpEnabled) {
        sendUdpCommand(command);
    }
    
    if (m_tcpEnabled) {
        sendTcpCommand(command);
    }
    
    emit commandSent(command);
    qDebug() << "Sent touch coordinate:" << x << "," << y;
}

void NativeController::sendModeChange(bool autoMode)
{
    QString command = QString("MODE:%1").arg(autoMode ? "AUTO" : "MANUAL");
    
    if (m_udpEnabled) {
        sendUdpCommand(command);
    }
    
    if (m_tcpEnabled) {
        sendTcpCommand(command);
    }
    
    emit commandSent(command);
    qDebug() << "Sent mode change:" << (autoMode ? "AUTO" : "MANUAL");
}

void NativeController::installGlobalKeyFilter(QWidget *widget)
{
    if (m_keyboardEnabled) {
        setupKeyboardShortcuts(widget);
    }
}

void NativeController::onTcpConnected()
{
    qDebug() << "Connected to TCP server at" << m_serverAddress << ":" << m_tcpPort;
    m_reconnectTimer->stop();
    emit serverConnected();
}

void NativeController::onTcpDisconnected()
{
    qDebug() << "Disconnected from TCP server";
    emit serverDisconnected();
    
    if (m_autoReconnect && m_tcpEnabled) {
        m_reconnectTimer->start();
    }
}

void NativeController::onTcpError(QAbstractSocket::SocketError error)
{
    QString errorMsg = m_tcpClient->errorString();
    qDebug() << "TCP error:" << errorMsg;
    emit errorOccurred(errorMsg);
    
    if (m_autoReconnect && m_tcpEnabled && !m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

void NativeController::reconnectToServer()
{
    if (m_tcpClient->state() == QTcpSocket::UnconnectedState) {
        qDebug() << "Attempting to reconnect to server...";
        connectToServer();
    }
}

void NativeController::onGamepadConnected(int deviceId)
{
    Q_UNUSED(deviceId)
    qDebug() << "Gamepad connected";
}

void NativeController::onGamepadDisconnected(int deviceId)
{
    Q_UNUSED(deviceId)
    qDebug() << "Gamepad disconnected";
}

void NativeController::onGamepadButtonPressed(int button)
{
    if (!m_gamepadEnabled) return;
    
    QString direction;
    switch (button) {
        case 0: // A button
            emit modeTogglePressed();
            return;
        case 1: // B button
            direction = "UP";
            break;
        case 2: // X button  
            direction = "DOWN";
            break;
        case 3: // Y button
            direction = "LEFT";
            break;
        case 4: // Left bumper
            direction = "RIGHT";
            break;
        default:
            return;
    }
    
    // Send to server and emit local signal
    sendButtonPress(direction);
    emit directionPressed(direction);
}

void NativeController::onGamepadAxisChanged(int axis, double value)
{
    if (!m_gamepadEnabled || qAbs(value) < 0.5) return;
    
    QString direction;
    if (axis == 0) { // Left stick X
        direction = value > 0 ? "RIGHT" : "LEFT";
    } else if (axis == 1) { // Left stick Y
        direction = value > 0 ? "DOWN" : "UP";
    } else {
        return;
    }
    
    // Send to server and emit local signal
    sendButtonPress(direction);
    emit directionPressed(direction);
}

void NativeController::onKeyboardShortcut()
{
    if (!m_keyboardEnabled) return;
    
    QShortcut *shortcut = qobject_cast<QShortcut*>(sender());
    if (!shortcut) return;
    
    QString key = shortcut->key().toString();
    
    if (key == "Space") {
        emit modeTogglePressed();
    } else if (key == "Up" || key == "W") {
        sendButtonPress("UP");
        emit directionPressed("UP");
    } else if (key == "Down" || key == "S") {
        sendButtonPress("DOWN");
        emit directionPressed("DOWN");
    } else if (key == "Left" || key == "A") {
        sendButtonPress("LEFT");
        emit directionPressed("LEFT");
    } else if (key == "Right" || key == "D") {
        sendButtonPress("RIGHT");
        emit directionPressed("RIGHT");
    }
}

void NativeController::setupKeyboardShortcuts(QWidget *parent)
{
    // Clear existing shortcuts
    qDeleteAll(m_shortcuts);
    m_shortcuts.clear();
    
    // Define shortcuts
    QStringList keys = {"Space", "Up", "Down", "Left", "Right", "W", "A", "S", "D"};
    
    for (const QString &key : keys) {
        QShortcut *shortcut = new QShortcut(QKeySequence(key), parent);
        connect(shortcut, &QShortcut::activated, this, &NativeController::onKeyboardShortcut);
        m_shortcuts.append(shortcut);
    }
}

void NativeController::setupGamepad()
{
#ifdef QT_GAMEPAD_ENABLED
    // Check if gamepad is available
    auto gamepads = QGamepad::availableGamepads();
    if (!gamepads.isEmpty()) {
        m_gamepad = new QGamepad(gamepads.first(), this);
        
        // Connect gamepad signals
        connect(m_gamepad, &QGamepad::connected, this, &NativeController::onGamepadConnected);
        connect(m_gamepad, &QGamepad::disconnected, this, &NativeController::onGamepadDisconnected);
        connect(m_gamepad, &QGamepad::buttonAChanged, this, [this](bool pressed) {
            if (pressed) onGamepadButtonPressed(0);
        });
        connect(m_gamepad, &QGamepad::buttonBChanged, this, [this](bool pressed) {
            if (pressed) onGamepadButtonPressed(1);
        });
        connect(m_gamepad, &QGamepad::buttonXChanged, this, [this](bool pressed) {
            if (pressed) onGamepadButtonPressed(2);
        });
        connect(m_gamepad, &QGamepad::buttonYChanged, this, [this](bool pressed) {
            if (pressed) onGamepadButtonPressed(3);
        });
        connect(m_gamepad, &QGamepad::buttonL1Changed, this, [this](bool pressed) {
            if (pressed) onGamepadButtonPressed(4);
        });
        
        // Connect axis changes
        connect(m_gamepad, &QGamepad::axisLeftXChanged, this, [this](double value) {
            onGamepadAxisChanged(0, value);
        });
        connect(m_gamepad, &QGamepad::axisLeftYChanged, this, [this](double value) {
            onGamepadAxisChanged(1, value);
        });
        
        m_gamepad->setEnabled(m_gamepadEnabled);
        qDebug() << "Gamepad initialized successfully";
    } else {
        qDebug() << "No gamepads found";
    }
#else
    qDebug() << "Gamepad support not available - compiled without Qt Gamepad";
#endif
}

void NativeController::sendUdpCommand(const QString &command)
{
    if (!m_udpSocket || m_serverAddress.isEmpty()) {
        qDebug() << "Cannot send UDP command: socket not ready or no server address";
        return;
    }
    
    QByteArray data = command.toUtf8();
    qint64 bytesWritten = m_udpSocket->writeDatagram(data, QHostAddress(m_serverAddress), m_udpPort);
    
    if (bytesWritten == -1) {
        qDebug() << "Failed to send UDP command:" << m_udpSocket->errorString();
    } else {
        qDebug() << "UDP command sent:" << command << "to" << m_serverAddress << ":" << m_udpPort;
    }
}

void NativeController::sendTcpCommand(const QString &command)
{
    if (!m_tcpClient || m_tcpClient->state() != QTcpSocket::ConnectedState) {
        qDebug() << "Cannot send TCP command: not connected to server";
        return;
    }
    
    QString formattedCommand = command + "\n";
    QByteArray data = formattedCommand.toUtf8();
    qint64 bytesWritten = m_tcpClient->write(data);
    
    if (bytesWritten == -1) {
        qDebug() << "Failed to send TCP command:" << m_tcpClient->errorString();
    } else {
        qDebug() << "TCP command sent:" << command;
    }
}

void NativeController::connectToServer()
{
    if (m_serverAddress.isEmpty()) {
        qDebug() << "Cannot connect: no server address set";
        return;
    }
    
    if (m_tcpClient->state() == QTcpSocket::ConnectedState) {
        qDebug() << "Already connected to server";
        return;
    }
    
    qDebug() << "Connecting to TCP server at" << m_serverAddress << ":" << m_tcpPort;
    m_tcpClient->connectToHost(m_serverAddress, m_tcpPort);
}