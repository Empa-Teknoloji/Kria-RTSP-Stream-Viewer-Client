cmake_minimum_required(VERSION 3.16)

project(kria VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# Try to find Qt Gamepad (optional)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Gamepad)
if(Qt${QT_VERSION_MAJOR}Gamepad_FOUND)
    message(STATUS "Found Qt Gamepad support")
    add_definitions(-DQT_GAMEPAD_ENABLED)
else()
    message(WARNING "Qt Gamepad not found. Gamepad functionality will be disabled.")
endif()

# Make OpenCV optional
option(USE_OPENCV "Enable OpenCV support for RTSP streaming" ON)

if(USE_OPENCV)
    # Try to find OpenCV package
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        message(STATUS "Found OpenCV ${OpenCV_VERSION}")
        message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
        include_directories(${OpenCV_INCLUDE_DIRS})
        add_definitions(-DOPENCV_ENABLED)
    else()
        message(WARNING "OpenCV not found. RTSP functionality will be disabled.")
        # Disable OpenCV if not found
        set(USE_OPENCV OFF)
        
        # Try to find opencv4 for Debian/Ubuntu systems if standard OpenCV is not found
        find_package(opencv4 QUIET)
        if(opencv4_FOUND)
            message(STATUS "Found OpenCV4 ${opencv4_VERSION}")
            include_directories(${opencv4_INCLUDE_DIRS})
            set(OpenCV_LIBS ${opencv4_LIBS})
            add_definitions(-DOPENCV_ENABLED)
            set(USE_OPENCV ON)
        endif()
    endif()
endif()

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        ui/mainwindow.ui
        src/rtspstreamer.cpp
        include/rtspstreamer.h
        src/distancemap.cpp
        include/distancemap.h
        src/nativecontroller.cpp
        include/nativecontroller.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(kria
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET kria APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(kria SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(kria
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Add include directory
target_include_directories(kria PRIVATE include)

# Link with Qt libraries
target_link_libraries(kria PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Link with Qt Gamepad if available
if(Qt${QT_VERSION_MAJOR}Gamepad_FOUND)
    target_link_libraries(kria PRIVATE Qt${QT_VERSION_MAJOR}::Gamepad)
endif()

# Link with OpenCV if available
if(USE_OPENCV AND OpenCV_FOUND)
    target_link_libraries(kria PRIVATE ${OpenCV_LIBS})
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.kria)
endif()
set_target_properties(kria PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS kria
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(kria)
endif()
