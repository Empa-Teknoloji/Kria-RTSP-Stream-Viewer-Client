#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <QDebug>
#include <QLoggingCategory>
#include <QDateTime>
#include <QMouseEvent>

Q_LOGGING_CATEGORY(mainWindow, "kria.mainwindow")

// Custom debug handler for better logging
void customDebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString typeStr;

    switch (type) {
    case QtDebugMsg:
        typeStr = "DEBUG";
        break;
    case QtInfoMsg:
        typeStr = "INFO";
        break;
    case QtWarningMsg:
        typeStr = "WARNING";
        break;
    case QtCriticalMsg:
        typeStr = "CRITICAL";
        break;
    case QtFatalMsg:
        typeStr = "FATAL";
        break;
    }

    QString formattedMessage = QString("%1 [%2] %3: %4")
                                   .arg(timestamp)
                                   .arg(typeStr)
                                   .arg(context.category)
                                   .arg(msg);

    // Output to stderr for all message types
    fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
    fflush(stderr);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_rtspStreamer(new RTSPStreamer(this))
{
    // Install custom debug handler
    qInstallMessageHandler(customDebugHandler);

    qCInfo(mainWindow) << "Starting Kria application";

    // Initialize RTSP URL from address and port
    m_rtspUrl = QString("rtsp://%1:%2/test").arg(m_tcpAddress).arg(m_rtspPort);

    ui->setupUi(this);
    setupUI();


    // Connect signals and slots
    connect(m_rtspStreamer, &RTSPStreamer::newFrameAvailable, this, &MainWindow::updateFrame);
    connect(m_rtspStreamer, &RTSPStreamer::connectionFailed, this, &MainWindow::handleConnectionError);

    // Set up native controller for remote control
    setupNativeController();

    // Set window to fullscreen immediately
    setWindowTitle("RTSP Stream Viewer");
    //showFullScreen();

    // Position the buttons after window is shown
    QTimer::singleShot(100, this, &MainWindow::updateButtonsPosition);

    // Connect to the stream automatically on startup
    QTimer::singleShot(1000, this, &MainWindow::connectToStream); // Increased delay for better startup
}

MainWindow::~MainWindow()
{
    if (m_rtspStreamer->isRunning()) {
        m_rtspStreamer->stopStreaming();
        m_rtspStreamer->wait();
    }

    // Stop native controller if it exists
    if (m_nativeController) {
        m_nativeController->stopController();
    }

    delete ui;
}

void MainWindow::setupUI()
{
    // Create central widget with layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Remove margins for fullscreen

    // Create video display section - full window size
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("QLabel { background-color: black; }");

    // Add video label to main layout
    mainLayout->addWidget(m_videoLabel);

    // Set the central widget
    setCentralWidget(centralWidget);

    // Create the distance map widget (top right corner)
    m_distanceMap = new DistanceMap(this);
    m_distanceMap->setFixedSize(200, 200);
    m_distanceMap->setMapSize(15, 15);
    m_distanceMap->raise(); // Ensure it's on top

    // Optimize distance map for better performance
    m_distanceMap->setAttribute(Qt::WA_OpaquePaintEvent, true);
    m_distanceMap->setAttribute(Qt::WA_NoSystemBackground, true);

    // Create the AUTO/MANUAL toggle button
    m_toggleButton = new QPushButton(this);
    m_toggleButton->setMinimumSize(150, 80);  // Make the button large
    m_toggleButton->setFocusPolicy(Qt::NoFocus); // Don't steal keyboard focus
    updateButtonStyle(); // Set initial style

    // Connect button signal
    connect(m_toggleButton, &QPushButton::clicked, this, &MainWindow::toggleAutoManual);

    // Create arrow buttons
    m_arrowButtons.append(createArrowButton("↑"));  // Up
    m_arrowButtons.append(createArrowButton("→"));  // Right
    m_arrowButtons.append(createArrowButton("↓"));  // Down
    m_arrowButtons.append(createArrowButton("←"));  // Left

    // Initially hide arrow buttons (since we start in AUTO mode)
    updateArrowButtonsVisibility();

    // Focus policy for key events
    setFocusPolicy(Qt::StrongFocus);

    // Show cursor since we have clickable buttons
    setCursor(Qt::ArrowCursor);
}

void MainWindow::updateFrame(const QImage &frame)
{
    if (!frame.isNull()) {
        // Use faster scaling for better performance
        QImage scaledImage = frame.scaled(m_videoLabel->size(),
                                          Qt::KeepAspectRatioByExpanding,
                                          Qt::FastTransformation);

        // Create a pixmap the size of the label
        QPixmap pixmap(m_videoLabel->size());
        pixmap.fill(Qt::black);

        // Create a painter to draw the scaled image centered on the pixmap
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false); // Disable antialiasing for speed
        int x = (pixmap.width() - scaledImage.width()) / 2;
        int y = (pixmap.height() - scaledImage.height()) / 2;
        painter.drawImage(x, y, scaledImage);
        painter.end();

        m_videoLabel->setPixmap(pixmap);
    }
}

void MainWindow::handleConnectionError()
{
    qCWarning(mainWindow) << "RTSP connection failed for URL:" << m_rtspUrl;

    // Display error message directly on video label instead of showing a message box
    QPixmap errorPixmap(m_videoLabel->size());
    errorPixmap.fill(Qt::black);

    QPainter painter(&errorPixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16));

#ifdef OPENCV_ENABLED
    painter.drawText(errorPixmap.rect(), Qt::AlignCenter,
                     "Connection Error: Failed to connect to RTSP stream.\n"
                     "URL: " + m_rtspUrl + "\n"
                                       "Press R to reconnect or Q to quit.");
#else
    painter.drawText(errorPixmap.rect(), Qt::AlignCenter,
                     "OpenCV Not Available\n"
                     "RTSP streaming is disabled.\n"
                     "Press Q to quit.");
#endif
    painter.end();

    m_videoLabel->setPixmap(errorPixmap);

    // Log RTSP connection failure
    qCWarning(mainWindow) << "RTSP connection failed - server may be unavailable";

    // Try to reconnect after 5 seconds
    qCInfo(mainWindow) << "Scheduling reconnection attempt in 5 seconds";
    QTimer::singleShot(5000, this, &MainWindow::connectToStream);
}

void MainWindow::connectToStream()
{
#ifndef OPENCV_ENABLED
    qCCritical(mainWindow) << "OpenCV not available - RTSP streaming disabled";
    QMessageBox::critical(this, "OpenCV Not Available",
                          "OpenCV was not found during compilation. RTSP streaming is disabled. Please install OpenCV and rebuild the application.");
    return;
#endif

    qCInfo(mainWindow) << "Connecting to RTSP stream:" << m_rtspUrl;

    // Set the URL and start the streaming thread
    m_rtspStreamer->setUrl(m_rtspUrl);

    if (!m_rtspStreamer->isRunning()) {
        m_rtspStreamer->start();
    } else {
        qCWarning(mainWindow) << "RTSP streamer already running";
    }

    // Clear any existing text on the video label
    m_videoLabel->setText("");
}

void MainWindow::disconnectFromStream()
{
    // Stop streaming
    if (m_rtspStreamer->isRunning()) {
        m_rtspStreamer->stopStreaming();
        m_rtspStreamer->wait();
    }

    // Clear the image
    m_videoLabel->setPixmap(QPixmap());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // Quit application when Escape is pressed
        close();
    } else if (event->key() == Qt::Key_F) {
        // Toggle fullscreen with F key
        if (isFullScreen()) {
            showNormal();
            setCursor(Qt::ArrowCursor); // Show cursor in windowed mode
        } else {
            showFullScreen();
            setCursor(Qt::BlankCursor); // Hide cursor in fullscreen
        }
    } else if (event->key() == Qt::Key_Q) {
        // Also quit on Q
        close();
    } else if (event->key() == Qt::Key_R) {
        // Reconnect to stream on R
        disconnectFromStream();
        QTimer::singleShot(500, this, &MainWindow::connectToStream);
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateButtonStyle()
{
    QString text = m_isAutoMode ? "AUTO" : "MANUAL";
    QString bgColor = m_isAutoMode ? "rgba(0, 128, 0, 160)" : "rgba(128, 0, 0, 160)"; // Green or Red with transparency

    // Set button style with custom CSS
    m_toggleButton->setStyleSheet(
        QString("QPushButton {"
                "  color: white;"
                "  background-color: %1;"
                "  border: 2px solid white;"
                "  border-radius: 10px;"
                "  font: bold 16pt 'Arial';"
                "  padding: 10px;"
                "}"
                "QPushButton:hover {"
                "  background-color: %2;"
                "}")
            .arg(bgColor)
            .arg(m_isAutoMode ? "rgba(0, 150, 0, 200)" : "rgba(150, 0, 0, 200)")
        );

    m_toggleButton->setText(text);
}

void MainWindow::toggleAutoManual()
{
    // Toggle the mode
    m_isAutoMode = !m_isAutoMode;

    // Update button appearance
    updateButtonStyle();

    // Update arrow buttons visibility
    updateArrowButtonsVisibility();

    // Here you can add code to handle the mode change
    // For example, sending commands to your device or changing behavior
}

void MainWindow::updateButtonsPosition()
{
    // Position toggle button in bottom right with margin
    const int margin = 30;
    int x = width() - m_toggleButton->width() - margin;
    int y = height() - m_toggleButton->height() - margin;

    m_toggleButton->move(x, y);

    // Ensure button is visible and on top
    m_toggleButton->raise();
    m_toggleButton->show();

    // Position distance map in top right corner
    m_distanceMap->move(width() - m_distanceMap->width() - margin, margin);
    m_distanceMap->raise();

    // Position arrow buttons in bottom left corner
    // Creating a diamond/cross pattern
    const int arrowMargin = 20;
    const int buttonSize = m_arrowButtons[0]->width();

    // Up arrow (top position)
    m_arrowButtons[0]->move(margin + buttonSize+10, height() - 2*buttonSize - margin - arrowMargin+20);

    // Right arrow (right position)
    m_arrowButtons[1]->move(margin + 2*buttonSize+50 - arrowMargin, height() - buttonSize - margin - arrowMargin/2);

    // Down arrow (bottom position)
    m_arrowButtons[2]->move(margin + buttonSize+10, height() - buttonSize - margin - 10);

    // Left arrow (left position)
    m_arrowButtons[3]->move(margin, height() - buttonSize - margin - arrowMargin/2);

    // Make sure all arrows are on top
    for (QPushButton* button : m_arrowButtons) {
        button->raise();
    }
}

QPushButton* MainWindow::createArrowButton(const QString& direction)
{
    QPushButton* button = new QPushButton(direction, this);
    button->setMinimumSize(60, 60);
    button->setFocusPolicy(Qt::NoFocus);
    button->setStyleSheet(
        "QPushButton {"
        "  color: white;"
        "  background-color: rgba(50, 50, 50, 160);"
        "  border: 2px solid white;"
        "  border-radius: 30px;"
        "  font: bold 24pt;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(80, 80, 80, 200);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(100, 100, 100, 220);"
        "}"
        );

    // Store the direction as a property
    button->setProperty("direction", direction);

    // Connect button click signal
    connect(button, &QPushButton::clicked, this, &MainWindow::arrowButtonClicked);

    return button;
}

void MainWindow::updateArrowButtonsVisibility()
{
    // Show arrow buttons only in MANUAL mode
    for (QPushButton* button : m_arrowButtons) {
        button->setVisible(!m_isAutoMode);
    }
}

void MainWindow::arrowButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString direction = button->property("direction").toString();

    qCInfo(mainWindow) << "Arrow button clicked:" << direction;

    // Convert arrow symbols to direction names
    QString directionName;
    if (direction == "↑") {
        directionName = "UP";
    } else if (direction == "→") {
        directionName = "RIGHT";
    } else if (direction == "↓") {
        directionName = "DOWN";
    } else if (direction == "←") {
        directionName = "LEFT";
    }

    // Send command to server
    if (!directionName.isEmpty() && m_nativeController) {
        m_nativeController->sendButtonPress(directionName);
        qCInfo(mainWindow) << "Sent button command to server:" << directionName;
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateButtonsPosition();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // Only process left mouse button clicks in AUTO mode
    if (event->button() == Qt::LeftButton && m_isAutoMode) {
        // Get click coordinates relative to the video display
        QPoint clickPos = event->pos();

        // Normalize coordinates based on RTSP stream dimensions
        QPointF normalizedCoord = normalizeCoordinates(clickPos);

        // Convert normalized coordinates to stream pixel coordinates
        QSize streamSize = m_rtspStreamer->getStreamSize();
        int streamX = static_cast<int>(normalizedCoord.x() * streamSize.width());
        int streamY = static_cast<int>(normalizedCoord.y() * streamSize.height());

        qCInfo(mainWindow) << "Mouse click in AUTO mode at screen:" << clickPos.x() << "," << clickPos.y()
                           << "normalized:" << normalizedCoord.x() << "," << normalizedCoord.y()
                           << "stream:" << streamX << "," << streamY;

        // Send normalized touch coordinates to server
        if (m_nativeController) {
            m_nativeController->sendTouchCoordinate(streamX, streamY);
            m_nativeController->sendModeChange(m_isAutoMode);
        }

        // Also trigger local handling with stream coordinates
        handleTouchCoordinate(streamX, streamY);
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::setupNativeController()
{
    m_nativeController = new NativeController(this);

    // Connect signals for local UI updates
    connect(m_nativeController, &NativeController::directionPressed, this, &MainWindow::handleDirectionPress);
    connect(m_nativeController, &NativeController::modeTogglePressed, this, &MainWindow::handleModeToggle);

    // Debug connections
    connect(m_nativeController, &NativeController::controllerStarted,
            []() { qDebug() << "Native controller started as CLIENT"; });
    connect(m_nativeController, &NativeController::serverConnected,
            []() { qDebug() << "Connected to server"; });
    connect(m_nativeController, &NativeController::serverDisconnected,
            []() { qDebug() << "Disconnected from server"; });
    connect(m_nativeController, &NativeController::commandSent,
            [](const QString &command) { qDebug() << "Command sent to server:" << command; });
    connect(m_nativeController, &NativeController::errorOccurred,
            [](const QString &error) { qDebug() << "Controller Error:" << error; });

    // Set network configuration
    m_nativeController->setServerAddress(m_tcpAddress);
    m_nativeController->setUdpPort(m_udpPort);
    m_nativeController->setTcpPort(m_tcpPort);

    // Enable client methods
    m_nativeController->enableKeyboardControl(true);
    m_nativeController->enableGamepadControl(true);
    m_nativeController->enableUdpClient(true);  // Enable UDP client
    m_nativeController->enableTcpClient(false); // Disable TCP client by default

    // Install keyboard shortcuts
    m_nativeController->installGlobalKeyFilter(this);

    // Start the controller
    m_nativeController->startController();

    qCInfo(mainWindow) << "Network configuration (CLIENT MODE):";
    qCInfo(mainWindow) << "  RTSP URL:" << m_rtspUrl;
    qCInfo(mainWindow) << "  Server Address:" << m_tcpAddress;
    qCInfo(mainWindow) << "  Server TCP Port:" << m_tcpPort;
    qCInfo(mainWindow) << "  Server UDP Port:" << m_udpPort;
}

void MainWindow::handleDirectionPress(const QString &direction)
{
    qCInfo(mainWindow) << "Direction pressed:" << direction;

    // Only handle directional buttons in MANUAL mode
    if (m_isAutoMode == false) {
        // Map direction names to arrow symbols
        QString arrowSymbol;
        if (direction == "UP") {
            arrowSymbol = "↑";
        } else if (direction == "DOWN") {
            arrowSymbol = "↓";
        } else if (direction == "LEFT") {
            arrowSymbol = "←";
        } else if (direction == "RIGHT") {
            arrowSymbol = "→";
        }

        // Find and simulate a click on the appropriate arrow button
        for (QPushButton* button : m_arrowButtons) {
            QString buttonDirection = button->property("direction").toString();
            if (buttonDirection == arrowSymbol) {
                // Simulate button click
                button->click();
                qCInfo(mainWindow) << "Simulated click on arrow button:" << arrowSymbol;
                break;
            }
        }
    }
}

void MainWindow::handleModeToggle()
{
    qDebug() << "Mode toggle pressed";
    toggleAutoManual();

    // Send mode change to server
    if (m_nativeController) {
        m_nativeController->sendModeChange(m_isAutoMode);
    }
}

void MainWindow::handleTouchCoordinate(int x, int y)
{
    qDebug() << "Touch coordinate received:" << x << "," << y;

    // Only process touch coordinates if in AUTO mode
    if (m_isAutoMode) {
        // Send touch coordinates to server
        if (m_nativeController) {
            //m_nativeController->sendTouchCoordinate(x, y);
        }

        // Optional: Visualize the touch point on the distance map
        if (m_distanceMap) {
            // You could add a method to highlight the touch point
            //m_distanceMap->highlightPoint(x, y);
        }

        qDebug() << "Sent touch coordinate to server:" << x << "," << y;
    }
}

QPointF MainWindow::normalizeCoordinates(const QPoint &screenCoord) const
{
    // Get the video label rectangle
    QRect videoRect = m_videoLabel->geometry();

    // Get the current frame to determine aspect ratio
    QImage currentFrame = m_rtspStreamer->getCurrentFrame();
    if (currentFrame.isNull()) {
        // If no frame is available, use stream dimensions
        QSize streamSize = m_rtspStreamer->getStreamSize();
        if (streamSize.isEmpty()) {
            // Return normalized coordinates based on screen position if no stream info
            return QPointF(static_cast<double>(screenCoord.x()) / width(),
                           static_cast<double>(screenCoord.y()) / height());
        }
        currentFrame = QImage(streamSize, QImage::Format_RGB888);
    }

    // Calculate the actual video area within the label (considering aspect ratio)
    QSize frameSize = currentFrame.size();
    QSize labelSize = m_videoLabel->size();

    // Calculate scaling to fit the frame in the label while maintaining aspect ratio
    double frameAspect = static_cast<double>(frameSize.width()) / frameSize.height();
    double labelAspect = static_cast<double>(labelSize.width()) / labelSize.height();

    QRect actualVideoRect;
    if (frameAspect > labelAspect) {
        // Frame is wider than label - fit to width
        int scaledHeight = static_cast<int>(labelSize.width() / frameAspect);
        int yOffset = (labelSize.height() - scaledHeight) / 2;
        actualVideoRect = QRect(videoRect.x(), videoRect.y() + yOffset,
                                labelSize.width(), scaledHeight);
    } else {
        // Frame is taller than label - fit to height
        int scaledWidth = static_cast<int>(labelSize.height() * frameAspect);
        int xOffset = (labelSize.width() - scaledWidth) / 2;
        actualVideoRect = QRect(videoRect.x() + xOffset, videoRect.y(),
                                scaledWidth, labelSize.height());
    }

    // Convert screen coordinates to coordinates within the actual video area
    QPoint relativeCoord = screenCoord - actualVideoRect.topLeft();

    // Normalize to 0.0-1.0 range
    double normalizedX = static_cast<double>(relativeCoord.x()) / actualVideoRect.width();
    double normalizedY = static_cast<double>(relativeCoord.y()) / actualVideoRect.height();

    // Clamp to valid range
    normalizedX = qBound(0.0, normalizedX, 1.0);
    normalizedY = qBound(0.0, normalizedY, 1.0);

    return QPointF(normalizedX, normalizedY);
}

void MainWindow::setNetworkConfiguration(const QString &address, quint16 rtspPort, quint16 tcpPort, quint16 udpPort)
{
    qCInfo(mainWindow) << "Updating network configuration:";
    qCInfo(mainWindow) << "  Previous - Address:" << m_tcpAddress << "RTSP:" << m_rtspPort << "TCP:" << m_tcpPort << "UDP:" << m_udpPort;

    m_tcpAddress = address;
    m_rtspPort = rtspPort;
    m_tcpPort = tcpPort;
    m_udpPort = udpPort;

    // Update RTSP URL
    m_rtspUrl = QString("rtsp://%1:%2/test").arg(m_tcpAddress).arg(m_rtspPort);

    qCInfo(mainWindow) << "  Updated - Address:" << m_tcpAddress << "RTSP:" << m_rtspPort << "TCP:" << m_tcpPort << "UDP:" << m_udpPort;
    qCInfo(mainWindow) << "  New RTSP URL:" << m_rtspUrl;

    // Update native controller configuration if it exists
    if (m_nativeController) {
        m_nativeController->setServerAddress(m_tcpAddress);
        m_nativeController->setUdpPort(m_udpPort);
        m_nativeController->setTcpPort(m_tcpPort);

        qCInfo(mainWindow) << "Native controller updated with new server configuration";
    }
}
