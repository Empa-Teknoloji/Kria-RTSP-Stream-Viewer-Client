#include "rtspstreamer.h"

RTSPStreamer::RTSPStreamer(QObject *parent) : QThread(parent), m_stopped(false), m_lowLatencyMode(true), m_streamSize(0, 0)
{
#ifdef OPENCV_ENABLED
    m_opencvEnabled = true;
#else
    m_opencvEnabled = false;
#endif
    
    // Set thread priority for better performance
    setPriority(QThread::HighPriority);
}

RTSPStreamer::~RTSPStreamer()
{
    stopStreaming();
    wait();
}

void RTSPStreamer::setUrl(const QString &url)
{
    m_mutex.lock();
    m_rtspUrl = url;
    m_mutex.unlock();
}

void RTSPStreamer::stopStreaming()
{
    m_mutex.lock();
    m_stopped = true;
    m_mutex.unlock();
}

bool RTSPStreamer::isStreaming() const
{
    return !m_stopped && isRunning();
}

QImage RTSPStreamer::getCurrentFrame() const
{
    // Use direct mutex lock/unlock instead of QMutexLocker for const methods
    m_mutex.lock();
    QImage frame = m_currentFrame;
    m_mutex.unlock();
    return frame;
}

void RTSPStreamer::setLowLatencyMode(bool enabled)
{
    m_mutex.lock();
    m_lowLatencyMode = enabled;
    m_mutex.unlock();
}

QSize RTSPStreamer::getStreamSize() const
{
    m_mutex.lock();
    QSize size = m_streamSize;
    m_mutex.unlock();
    return size;
}

int RTSPStreamer::getStreamWidth() const
{
    return getStreamSize().width();
}

int RTSPStreamer::getStreamHeight() const
{
    return getStreamSize().height();
}

void RTSPStreamer::run()
{
    m_mutex.lock();
    m_stopped = false;
    QString rtspUrl = m_rtspUrl;
    m_mutex.unlock();

    if (!m_opencvEnabled) {
        emit connectionFailed();
        return;
    }

#ifdef OPENCV_ENABLED
    // Open the RTSP stream with basic settings
    m_videoCapture.open(rtspUrl.toStdString());

    // Check if the stream is opened successfully
    if (!m_videoCapture.isOpened()) {
        emit connectionFailed();
        return;
    }

    // Basic buffer optimization for low latency
    m_videoCapture.set(cv::CAP_PROP_BUFFERSIZE, 1);

    // Get stream dimensions and store them
    int width = static_cast<int>(m_videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(m_videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
    
    m_mutex.lock();
    m_streamSize = QSize(width, height);
    m_mutex.unlock();

    // Main capture loop
    while (!m_stopped) {
        cv::Mat frame;
        if (!m_videoCapture.read(frame)) {
            // If we couldn't read the frame, try to reconnect
            m_videoCapture.release();
            if (!m_videoCapture.open(rtspUrl.toStdString())) {
                emit connectionFailed();
                break;
            }
            continue;
        }

        if (frame.empty())
            continue;

        // Convert the frame to QImage
        QImage qimg = matToQImage(frame);

        m_mutex.lock();
        m_currentFrame = qimg;
        m_mutex.unlock();

        // Emit signal with the new frame
        emit newFrameAvailable(qimg);

        // Minimal delay for low latency - let OpenCV handle timing
        msleep(1);
    }

    // Close the video capture when done
    m_videoCapture.release();
#endif
}

#ifdef OPENCV_ENABLED
QImage RTSPStreamer::matToQImage(const cv::Mat &mat) const
{
    // Handle different image formats
    if (mat.type() == CV_8UC3) {
        // Convert BGR to RGB
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                      rgbMat.step, QImage::Format_RGB888).copy();
    } else if (mat.type() == CV_8UC1) {
        // Grayscale image
        return QImage(mat.data, mat.cols, mat.rows, 
                      mat.step, QImage::Format_Grayscale8).copy();
    } else {
        // Default case - convert to RGB
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                      rgbMat.step, QImage::Format_RGB888).copy();
    }
}
#endif
