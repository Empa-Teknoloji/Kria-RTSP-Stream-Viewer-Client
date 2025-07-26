# Kria - RTSP Stream Viewer Client

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/yourusername/kria)
[![Qt Version](https://img.shields.io/badge/Qt-5.15%2B%20%7C%206.x-blue.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A high-performance Qt-based RTSP stream viewer that acts as a **client**, sending button presses and touch coordinates to a remote server. Perfect for remote control applications, robotics projects, and IoT device management.

## Features

### Core Functionality
- **RTSP Video Streaming**: Connects to RTSP camera feeds with optimized performance
- **Fullscreen Display**: Immersive viewing experience with minimal UI
- **Dual Control Modes**: AUTO mode for touch coordinates, MANUAL mode for directional control
- **Distance Map**: Real-time radar-style visualization widget

### CLIENT Control Options
- **Keyboard Controls**: Sends button commands to server via UDP/TCP
- **Gamepad Support**: Xbox/PlayStation controller commands sent to server
- **Mouse Clicks**: Touch coordinates sent to server in AUTO mode
- **UDP Client**: Lightweight UDP command transmission
- **TCP Client**: Optional persistent TCP connection to server

## Control Methods

### Keyboard Shortcuts
- **Arrow Keys / WASD**: Directional movement (in MANUAL mode)
- **Space**: Toggle AUTO/MANUAL mode
- **F**: Toggle fullscreen
- **R**: Reconnect to RTSP stream
- **Q/Esc**: Quit application

### Gamepad Controls
- **A Button**: Toggle AUTO/MANUAL mode
- **B Button**: Up movement
- **X Button**: Down movement  
- **Y Button**: Left movement
- **Left Bumper**: Right movement
- **Left Stick**: Directional movement

### Network Controls

#### Commands Sent by Kria CLIENT
Kria sends these commands to the server at 192.168.1.71:8556 (UDP) or :8555 (TCP):

```
# Button commands (MANUAL mode)
BUTTON:UP
BUTTON:DOWN
BUTTON:LEFT
BUTTON:RIGHT

# Mode changes
MODE:AUTO
MODE:MANUAL

# Touch coordinates (AUTO mode)
TOUCH:x:y
```

#### TCP Client (Port 8555, Optional)
Optional persistent TCP connection to server with acknowledgments.

## 📁 Project Structure

```
kria/
├── src/                    # Source files (.cpp)
├── include/                # Header files (.h)
├── ui/                     # Qt UI files (.ui)
├── scripts/                # Build and installation scripts
├── tests/                  # Test files and utilities
├── docs/                   # Documentation
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
└── .gitignore             # Git ignore rules
```

## 🚀 Quick Start

### Prerequisites
- **Qt 5.15+** or **Qt 6.x**
- **OpenCV 4.x** (optional, for RTSP support)
- **CMake 3.16+**
- **C++17** compatible compiler

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/kria.git
   cd kria
   ```

2. **Build the project**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Run the application**
   ```bash
   ./kria
   ```

### Build Options
```bash
# Disable OpenCV support (if you don't need RTSP)
cmake -DUSE_OPENCV=OFF ..

# Custom installation directory
cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..
make install

# Use convenience scripts
./scripts/build.sh              # Standard build
./scripts/build_optimized.sh    # Optimized build
```

## Configuration

### Network Settings
Default network configuration (CLIENT sends to):
- **Server Address**: `192.168.1.71`
- **RTSP URL**: `rtsp://192.168.1.71:8554/camera` (for video stream)
- **Server UDP Port**: 8556 (commands sent here)
- **Server TCP Port**: 8555 (optional persistent connection)

To change the server configuration, modify the variables in `include/mainwindow.h`:
```cpp
QString m_tcpAddress = "your.server.ip.here";  // Server IP
quint16 m_rtspPort = 8554;                     // RTSP stream port
quint16 m_tcpPort = 8555;                      // Server TCP port
quint16 m_udpPort = 8556;                      // Server UDP port
```

Alternatively, copy `include/network_config_example.h` to `include/network_config.h` and customize your settings there.

### Optimizations Applied
- **RTSP Streaming**: Reduced buffer size, optimized frame rate, thread priority
- **UI Rendering**: Fast scaling, disabled antialiasing for performance
- **Memory Management**: Efficient image handling, optimized paint events
- **Error Handling**: Comprehensive logging with timestamps

## Testing

Use the included test server to receive commands from Kria:
```bash
# Start test server to receive commands
python3 tests/test_server.py

# Start only UDP server
python3 tests/test_server.py --udp-only

# Start only TCP server  
python3 tests/test_server.py --tcp-only

# Show help
python3 tests/test_server.py --help

# Debug UDP connections
python3 tests/debug_udp.py

# Test controller functionality
python3 tests/test_controller.py
```

Then run Kria and use keyboard/mouse/gamepad - you'll see commands received by the server.

## Troubleshooting

### RTSP Connection Issues
- Verify camera IP address and port
- Check network connectivity
- Ensure OpenCV is compiled with RTSP support
- Review application logs for detailed error messages

### Commands Not Being Sent
- Verify server is running on 192.168.1.71:8556 (UDP) or :8555 (TCP)
- Test with the included test_server.py script
- Check application logs for connection status  
- Ensure network connectivity to server
- Verify firewall allows outbound connections

### Performance Issues
- Reduce RTSP stream resolution at source
- Adjust frame rate in `src/rtspstreamer.cpp`
- Monitor CPU and memory usage
- Consider hardware acceleration options

## 🔧 Development

### Project Structure Details
- **`src/`**: All implementation files (.cpp)
- **`include/`**: All header files (.h) 
- **`ui/`**: Qt Designer UI files (.ui)
- **`scripts/`**: Build automation and installation scripts
- **`tests/`**: Test utilities and mock servers
- **`docs/`**: Additional documentation

### Adding New Control Methods
1. Extend `NativeController` class in `include/nativecontroller.h`
2. Add signal connections in `src/mainwindow.cpp::setupNativeController()`
3. Implement command processing in `src/nativecontroller.cpp::processCommand()`

### Customizing UI
- Modify button positions in `src/mainwindow.cpp::updateButtonsPosition()`
- Adjust colors and styles in button creation methods
- Update distance map configuration in `src/mainwindow.cpp::setupUI()`
- Edit UI layout in `ui/mainwindow.ui` with Qt Designer

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 🐛 Issues

If you encounter any problems or have suggestions, please [open an issue](https://github.com/yourusername/kria/issues) on GitHub.

## 📞 Support

- **Documentation**: Check the [Wiki](https://github.com/yourusername/kria/wiki) for detailed documentation
- **Issues**: Report bugs via [GitHub Issues](https://github.com/yourusername/kria/issues)
- **Discussions**: Join the [GitHub Discussions](https://github.com/yourusername/kria/discussions) for questions and ideas

## ⭐ Star History

If you find this project useful, please consider giving it a star on GitHub!