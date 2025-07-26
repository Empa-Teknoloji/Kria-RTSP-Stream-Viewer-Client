#ifndef DISTANCEMAP_H
#define DISTANCEMAP_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QColor>
#include <QVector>
#include <QPointF>

// Structure to represent a point in the radar display
struct RadarPoint {
    float distance;     // Distance in meters
    float angle;        // Angle in degrees (0-180)
    float velocity;     // Optional: velocity for animation
    QColor color;       // Color of the point
};

class DistanceMap : public QWidget
{
    Q_OBJECT

public:
    explicit DistanceMap(QWidget *parent = nullptr);
    ~DistanceMap();

    // Sets the size of the widget
    void setMapSize(int width, int height);
    
    // Add a new point to the radar
    void addRadarPoint(float distance, float angle, QColor color = Qt::red);
    
    // Clear all points
    void clearPoints();
    
    // Simulates radar data for testing
    void generateSimulatedData();

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    
private:
    QVector<RadarPoint> m_points;
    QTimer* m_simulationTimer;
    int m_mapWidth = 100;
    int m_mapHeight = 100;
    float m_maxDistance = 10.0f;  // Maximum distance in meters
    int m_animationTimerId;
    
    // Converts radar coordinates to widget coordinates
    QPointF radarToWidget(float distance, float angle);
    
    // Converts a distance value to a color
    QColor distanceToColor(float distance);
    
    // Updates the positions of the moving points
    void updatePointPositions();
};

#endif // DISTANCEMAP_H
