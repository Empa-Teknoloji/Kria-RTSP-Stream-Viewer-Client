/*
 * Network Configuration Example for Kria
 * 
 * To change the network settings, modify these variables in mainwindow.h:
 */

// Example 1: Local development setup
QString m_tcpAddress = "192.168.1.71";
quint16 m_rtspPort = 8554;
quint16 m_tcpPort = 8555;
quint16 m_udpPort = 8556;

// Example 2: Production camera setup
// QString m_tcpAddress = "192.168.1.71";
// quint16 m_rtspPort = 8554;
// quint16 m_tcpPort = 8555;
// quint16 m_udpPort = 8556;

// Example 3: Remote deployment
// QString m_tcpAddress = "10.0.0.100";
// quint16 m_rtspPort = 554;    // Standard RTSP port
// quint16 m_tcpPort = 8555;
// quint16 m_udpPort = 8556;

/*
 * Notes:
 * - m_tcpAddress: IP address of the camera/server
 * - m_rtspPort: Port for RTSP stream (usually 554 or 8554)
 * - m_tcpPort: Port for TCP control server (optional)
 * - m_udpPort: Port for UDP control commands
 * 
 * The RTSP URL is automatically constructed as:
 * rtsp://[m_tcpAddress]:[m_rtspPort]/camera
 * 
 * UDP commands can be sent to:
 * [m_tcpAddress]:[m_udpPort]
 */