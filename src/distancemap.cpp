#include "distancemap.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QtCore/qcoreevent.h>
#include <cmath>

// Only define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DistanceMap::DistanceMap(QWidget *parent) : QWidget(parent)
{
    // Initialize with default size
    setFixedSize(240, 200);
    
    // Set up transparent background
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Create timer for simulating data updates
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout, this, &DistanceMap::generateSimulatedData);
    m_simulationTimer->start(2000);  // Update every 2 seconds
    
    // Start animation timer for moving points
    m_animationTimerId = startTimer(50); // 20 fps animation
}

DistanceMap::~DistanceMap()
{
    m_simulationTimer->stop();
    killTimer(m_animationTimerId);
}

void DistanceMap::setMapSize(int width, int height)
{
    m_mapWidth = width;
    m_mapHeight = height;
    update();
}

void DistanceMap::addRadarPoint(float distance, float angle, QColor color)
{
    RadarPoint point;
    point.distance = distance;
    point.angle = angle;
    point.color = color;
    // Use integer bounded and convert to float
    point.velocity = QRandomGenerator::global()->bounded(10, 50) / 100.0f;  // Random speed 0.1-0.5
    
    m_points.append(point);
    update();
}

void DistanceMap::clearPoints()
{
    m_points.clear();
    update();
}

void DistanceMap::updatePointPositions()
{
    for (int i = 0; i < m_points.size(); ++i) {
        // Add some random movement to the points
        float randomMove = (QRandomGenerator::global()->bounded(20) - 10) / 100.0f;  // -0.1 to 0.1
        int direction = QRandomGenerator::global()->bounded(2) ? -1 : 1;
        
        m_points[i].distance += (randomMove + 
                                m_points[i].velocity * direction);
        
        m_points[i].angle += (QRandomGenerator::global()->bounded(40) - 10) / 10.0f;  // -1.0 to 1.0
        
        // Keep within bounds
        if (m_points[i].distance < 0.5f) m_points[i].distance = 0.5f;
        if (m_points[i].distance > m_maxDistance) m_points[i].distance = m_maxDistance;
        
        if (m_points[i].angle < 0.0f) m_points[i].angle = 0.0f;
        if (m_points[i].angle > 180.0f) m_points[i].angle = 180.0f;
    }
    
    update();
}

void DistanceMap::generateSimulatedData()
{
    // Clear existing points and generate new ones
    clearPoints();
    
    // Generate 5-10 random points
    int numPoints = QRandomGenerator::global()->bounded(5, 11);
    
    for (int i = 0; i < numPoints; i++) {
        // Generate random distance between 0.5 and max distance
        int distInt = QRandomGenerator::global()->bounded(5, static_cast<int>(m_maxDistance * 10));
        float distance = distInt / 10.0f;  // Convert to float with one decimal precision
        
        // Generate random angle between 0 and 180
        int angleInt = QRandomGenerator::global()->bounded(0, 181);  // 0 to 180
        float angle = static_cast<float>(angleInt);
        
        QColor color;
        if (distance < 3.0f) {
            // Red for close objects
            color = Qt::red;
        } else if (distance < 6.0f) {
            // Yellow for medium-distance objects
            color = QColor(255, 215, 0);
        } else {
            // Green for far objects
            color = Qt::green;
        }
        
        addRadarPoint(distance, angle, color);
    }
}

QPointF DistanceMap::radarToWidget(float distance, float angle)
{
    // Convert polar coordinates (distance, angle) to widget coordinates
    // The radar is centered at the bottom center of the widget
    float centerX = width() / 2.0f;
    float centerY = height() - 10;  // Slight offset from bottom
    
    // Scale distance to fit within the widget
    float scaledDistance = (distance / m_maxDistance) * (height() - 20);
    
    // Convert angle from degrees to radians
    // With rotation: 0° is right (east), 90° is up (north), 180° is left (west)
    float radians = (180 - angle) * M_PI / 180.0f;
    
    // Calculate x and y positions (Note: we invert Y because widget coordinates go down)
    float x = centerX + scaledDistance * cos(radians);
    float y = centerY - scaledDistance * sin(radians);
    
    return QPointF(x, y);
}

QColor DistanceMap::distanceToColor(float distance)
{
    // Distance color mapping: Green (far) -> Yellow -> Red (close)
    float normalizedDistance = 1.0f - (distance / m_maxDistance);
    
    if (normalizedDistance < 0.5f) {
        // Green to Yellow (far to medium)
        int red = 255 * (normalizedDistance * 2);
        return QColor(red, 255, 0, 200);
    } else {
        // Yellow to Red (medium to close)
        int green = 255 * (2 - normalizedDistance * 2);
        return QColor(255, green, 0, 200);
    }
}

void DistanceMap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Set a semi-transparent black background
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
    
    // Draw border
    painter.setPen(QPen(QColor(255, 255, 255, 200), 2));
    painter.drawRect(QRectF(0, 0, width() - 1, height() - 1));
    
    // Add title text
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(QRectF(0, 0, width(), 20), 
                    Qt::AlignCenter, "Distance Radar");
    
    // Set the center point of the radar
    float centerX = width() / 2.0f;
    float centerY = height() - 10;
    
    // Draw the half circles at 2-meter intervals
    painter.setPen(QPen(QColor(100, 100, 100, 150), 1));
    for (float dist = 2.0f; dist <= m_maxDistance; dist += 2.0f) {
        float radius = (dist / m_maxDistance) * (height() - 20);
        
        // Draw half circle (180 degrees)
        painter.drawArc(QRectF(centerX - radius, centerY - radius, 
                              radius * 2, radius * 2), 
                       0, 180 * 16);  // Qt uses 1/16th degree units
        
        // Add distance label
        QString label = QString::number(dist) + "m";
        QPointF labelPos = radarToWidget(dist, 90); // Position at top
        labelPos.setY(labelPos.y() - 15); // Offset slightly above the arc
        
        painter.drawText(labelPos, label);
    }
    
    // Draw the angle lines
    for (int angle = 0; angle <= 180; angle += 30) {
        QPointF start = radarToWidget(0, angle);
        QPointF end = radarToWidget(m_maxDistance, angle);
        
        painter.drawLine(start, end);
        
        // Add angle label
        QPointF labelPos = radarToWidget(m_maxDistance * 0.9, angle);
        
        // Adjust angle labels to match new orientation
        // 0° is right (east), 90° is up (north), 180° is left (west)
        painter.drawText(QRectF(labelPos.x() - 15, labelPos.y() - 15, 30, 20), 
                       Qt::AlignCenter, QString::number(angle) + "°");
    }
    
    // Draw each point on the radar
    for (const RadarPoint &point : m_points) {
        QPointF pos = radarToWidget(point.distance, point.angle);
        
        // Draw a circle for the point
        painter.setPen(Qt::NoPen);
        painter.setBrush(point.color);
        painter.drawEllipse(pos, 5, 5);
        
        // Optionally draw a small "tail" to show motion
        painter.setPen(QPen(point.color, 1));
        float tailAngle = (180 - point.angle) * M_PI / 180.0f;
        QPointF tailEnd(pos.x() - 10 * cos(tailAngle), 
                        pos.y() + 10 * sin(tailAngle));
        painter.drawLine(pos, tailEnd);
    }
}

void DistanceMap::timerEvent(QTimerEvent *event)
{
    if (event && event->timerId() == m_animationTimerId) {
        updatePointPositions();
    }
}
