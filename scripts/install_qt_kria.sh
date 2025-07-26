#!/bin/bash

# Qt Installation Script for Kria Development
# This script installs Qt development environment for the Kria RTSP streaming application

set -e  # Exit on any error

echo "=========================================="
echo "Qt Installation Script for Kria Development"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to detect the OS
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$NAME
        VERSION=$VERSION_ID
    else
        print_error "Cannot detect OS. /etc/os-release not found."
        exit 1
    fi
    
    print_status "Detected OS: $OS $VERSION"
}

# Function to install Qt on Ubuntu/Debian
install_qt_ubuntu() {
    print_status "Installing Qt development packages for Ubuntu/Debian..."
    
    # Update package list
    sudo apt update
    
    # Install Qt6 (preferred) or Qt5 as fallback
    if apt list --installed | grep -q qt6-base-dev 2>/dev/null || apt search qt6-base-dev 2>/dev/null | grep -q qt6-base-dev; then
        print_status "Installing Qt6 development packages..."
        sudo apt install -y \
            qt6-base-dev \
            qt6-base-dev-tools \
            libqt6widgets6 \
            libqt6network6 \
            libqt6gui6 \
            libqt6core6 \
            qt6-tools-dev \
            qt6-tools-dev-tools \
            cmake \
            build-essential \
            pkg-config
        
        # Optional: Qt6 Gamepad support
        if apt search libqt6gamepad6-dev 2>/dev/null | grep -q libqt6gamepad6-dev; then
            print_status "Installing Qt6 Gamepad support..."
            sudo apt install -y libqt6gamepad6-dev
        else
            print_warning "Qt6 Gamepad not available, skipping..."
        fi
        
        QT_VERSION="Qt6"
    else
        print_status "Qt6 not available, installing Qt5..."
        sudo apt install -y \
            qt5-default \
            qtbase5-dev \
            qttools5-dev \
            qttools5-dev-tools \
            libqt5widgets5 \
            libqt5network5 \
            libqt5gui5 \
            libqt5core5a \
            cmake \
            build-essential \
            pkg-config
        
        # Optional: Qt5 Gamepad support
        if apt search libqt5gamepad5-dev 2>/dev/null | grep -q libqt5gamepad5-dev; then
            print_status "Installing Qt5 Gamepad support..."
            sudo apt install -y libqt5gamepad5-dev
        else
            print_warning "Qt5 Gamepad not available, skipping..."
        fi
        
        QT_VERSION="Qt5"
    fi
}

# Function to install Qt on CentOS/RHEL/Fedora
install_qt_redhat() {
    print_status "Installing Qt development packages for Red Hat based systems..."
    
    if command -v dnf &> /dev/null; then
        PKG_MANAGER="dnf"
    elif command -v yum &> /dev/null; then
        PKG_MANAGER="yum"
    else
        print_error "Neither dnf nor yum found. Cannot install packages."
        exit 1
    fi
    
    # Install development tools
    sudo $PKG_MANAGER groupinstall -y "Development Tools"
    sudo $PKG_MANAGER install -y cmake pkg-config
    
    # Try Qt6 first, then Qt5
    if $PKG_MANAGER list available | grep -q qt6-qtbase-devel; then
        print_status "Installing Qt6 development packages..."
        sudo $PKG_MANAGER install -y \
            qt6-qtbase-devel \
            qt6-qttools-devel \
            qt6-qtbase \
            cmake \
            gcc-c++
        
        # Optional: Qt6 Gamepad support
        if $PKG_MANAGER list available | grep -q qt6-qtgamepad-devel; then
            print_status "Installing Qt6 Gamepad support..."
            sudo $PKG_MANAGER install -y qt6-qtgamepad-devel
        fi
        
        QT_VERSION="Qt6"
    else
        print_status "Installing Qt5 development packages..."
        sudo $PKG_MANAGER install -y \
            qt5-qtbase-devel \
            qt5-qttools-devel \
            qt5-qtbase \
            cmake \
            gcc-c++
        
        # Optional: Qt5 Gamepad support
        if $PKG_MANAGER list available | grep -q qt5-qtgamepad-devel; then
            print_status "Installing Qt5 Gamepad support..."
            sudo $PKG_MANAGER install -y qt5-qtgamepad-devel
        fi
        
        QT_VERSION="Qt5"
    fi
}

# Function to install OpenCV (optional but recommended)
install_opencv() {
    print_status "Installing OpenCV for RTSP streaming support..."
    
    if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
        # Try OpenCV 4 first, then OpenCV 3
        if apt search libopencv-dev 2>/dev/null | grep -q libopencv-dev; then
            sudo apt install -y libopencv-dev libopencv-contrib-dev
            print_status "OpenCV installed successfully"
        else
            print_warning "OpenCV not available in package manager"
            print_status "You may need to build OpenCV from source"
        fi
        
    elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]] || [[ "$OS" == *"Fedora"* ]]; then
        if $PKG_MANAGER list available | grep -q opencv-devel; then
            sudo $PKG_MANAGER install -y opencv-devel
            print_status "OpenCV installed successfully"
        else
            print_warning "OpenCV not available in package manager"
            print_status "You may need to build OpenCV from source"
        fi
    fi
}

# Function to verify installation
verify_installation() {
    print_status "Verifying Qt installation..."
    
    # Check for cmake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake not found. Installation may have failed."
        return 1
    fi
    
    # Check for Qt tools
    if command -v qmake &> /dev/null; then
        print_status "qmake found: $(qmake --version)"
    elif command -v qmake-qt5 &> /dev/null; then
        print_status "qmake-qt5 found: $(qmake-qt5 --version)"
    elif command -v qmake-qt6 &> /dev/null; then
        print_status "qmake-qt6 found: $(qmake-qt6 --version)"
    else
        print_warning "qmake not found in PATH"
    fi
    
    # Check for moc
    if command -v moc &> /dev/null; then
        print_status "moc found"
    elif command -v moc-qt5 &> /dev/null; then
        print_status "moc-qt5 found"
    elif command -v moc-qt6 &> /dev/null; then
        print_status "moc-qt6 found"
    else
        print_warning "moc not found in PATH"
    fi
    
    print_status "Installation verification completed"
}

# Function to create build script
create_build_script() {
    print_status "Creating build script for the Kria application..."
    
    cat > build_kria.sh << 'EOF'
#!/bin/bash

# Build script for Kria Qt application
set -e

echo "Building Kria Qt application..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_OPENCV=ON

# Build the application
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: ./kria"
EOF

    chmod +x build_kria.sh
    print_status "Build script created: build_kria.sh"
}

# Function to create environment setup script
create_environment_script() {
    print_status "Creating environment setup script..."
    
    cat > setup_env.sh << 'EOF'
#!/bin/bash

# Environment setup for Kria Qt development
echo "Setting up Qt development environment..."

# Add Qt tools to PATH if not already present
if [[ "$QT_VERSION" == "Qt6" ]]; then
    # Try to find Qt6 installation paths
    QT6_PATHS=("/usr/lib/qt6/bin" "/usr/lib/x86_64-linux-gnu/qt6/bin" "/opt/qt6/bin")
    for path in "${QT6_PATHS[@]}"; do
        if [[ -d "$path" && ":$PATH:" != *":$path:"* ]]; then
            export PATH="$path:$PATH"
            echo "Added Qt6 tools to PATH: $path"
            break
        fi
    done
else
    # Try to find Qt5 installation paths
    QT5_PATHS=("/usr/lib/qt5/bin" "/usr/lib/x86_64-linux-gnu/qt5/bin" "/opt/qt5/bin")
    for path in "${QT5_PATHS[@]}"; do
        if [[ -d "$path" && ":$PATH:" != *":$path:"* ]]; then
            export PATH="$path:$PATH"
            echo "Added Qt5 tools to PATH: $path"
            break
        fi
    done
fi

# Set Qt environment variables
export QT_QPA_PLATFORM_PLUGIN_PATH="/usr/lib/x86_64-linux-gnu/qt5/plugins"
export QT_DEBUG_PLUGINS=0

echo "Environment setup completed"
echo "You can now build the application using: ./build_kria.sh"
EOF

    chmod +x setup_env.sh
    print_status "Environment script created: setup_env.sh"
}

# Main installation function
main() {
    print_status "Starting Qt installation for Kria development..."
    
    # Check if running as root
    if [[ $EUID -eq 0 ]]; then
        print_error "This script should not be run as root directly."
        print_status "It will use sudo when necessary."
        exit 1
    fi
    
    # Detect OS
    detect_os
    
    # Install Qt based on OS
    case "$OS" in
        *"Ubuntu"*|*"Debian"*)
            install_qt_ubuntu
            ;;
        *"CentOS"*|*"Red Hat"*|*"Fedora"*)
            install_qt_redhat
            ;;
        *)
            print_error "Unsupported operating system: $OS"
            print_status "Please install Qt development packages manually:"
            print_status "- Qt5 or Qt6 development libraries"
            print_status "- CMake build system"
            print_status "- C++ compiler (gcc/g++)"
            exit 1
            ;;
    esac
    
    # Install OpenCV (optional)
    read -p "Install OpenCV for RTSP streaming support? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_opencv
    else
        print_warning "Skipping OpenCV installation. RTSP streaming will be disabled."
    fi
    
    # Verify installation
    verify_installation
    
    # Create build and environment scripts
    create_build_script
    create_environment_script
    
    print_status "Installation completed successfully!"
    print_status "Qt Version: $QT_VERSION"
    print_status ""
    print_status "Next steps:"
    print_status "1. Run: source ./setup_env.sh"
    print_status "2. Build: ./build_kria.sh"
    print_status "3. Run: ./build/kria"
    print_status ""
    print_status "Note: Make sure to configure the network settings in the code"
    print_status "      (IP address, ports) before building."
}

# Run main function
main "$@"