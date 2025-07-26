#ifndef RTSPSTREAMER_H
#define RTSPSTREAMER_H

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// Check if OpenCV is enabled at compile time
#ifdef OPENCV_ENABLED
#include <opencv2/opencv.hpp>
#endif

class RTSPStreamer : public QThread
{
    Q_OBJECT

public:
    explicit RTSPStreamer(QObject *parent = nullptr);
    ~RTSPStreamer();

    void setUrl(const QString &url);
    void stopStreaming();
    bool isStreaming() const;
    QImage getCurrentFrame() const;
    void setLowLatencyMode(bool enabled);
    
    // Get stream dimensions
    QSize getStreamSize() const;
    int getStreamWidth() const;
    int getStreamHeight() const;

protected:
    void run() override;

signals:
    void newFrameAvailable(const QImage &frame);
    void connectionFailed();

private:
    QString m_rtspUrl;
    QImage m_currentFrame;
    mutable QMutex m_mutex; // Made mutable to allow modification in const methods
    bool m_stopped;
    bool m_opencvEnabled;
    bool m_lowLatencyMode;
    QSize m_streamSize;
    
#ifdef OPENCV_ENABLED
    cv::VideoCapture m_videoCapture;
    // Convert OpenCV Mat to QImage
    QImage matToQImage(const cv::Mat &mat) const;
#endif
};

#endif // RTSPSTREAMER_H
