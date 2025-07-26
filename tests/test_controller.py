#!/usr/bin/env python3
"""
Simple test script for the Kria Native Controller
Tests UDP commands and basic functionality
"""

import socket
import time
import sys

def test_udp_controller(host='192.168.1.71', port=8556):
    """Test the UDP controller interface"""
    
    print(f"Testing Kria UDP Controller at {host}:{port}")
    print(f"Make sure Kria is running and configured for address: {host}")
    print("="*60)
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(5.0)  # 5 second timeout
    
    # Test commands
    commands = [
        "UP",
        "DOWN", 
        "LEFT",
        "RIGHT",
        "TOGGLE",
        "TOUCH:100:200",
        "MODE"
    ]
    
    try:
        for cmd in commands:
            print(f"Sending command: {cmd}")
            
            # Send command
            sock.sendto(cmd.encode('utf-8'), (host, port))
            
            # Wait for acknowledgment
            try:
                data, addr = sock.recvfrom(1024)
                response = data.decode('utf-8')
                print(f"  Response: {response}")
            except socket.timeout:
                print(f"  No response received (timeout)")
            
            time.sleep(0.5)
            
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()

def test_keyboard_help():
    """Print keyboard shortcuts"""
    print("\nKeyboard Controls:")
    print("  Arrow Keys / WASD: Directional movement")
    print("  Space: Toggle AUTO/MANUAL mode")
    print("  F: Toggle fullscreen")
    print("  R: Reconnect to RTSP stream")
    print("  Q/Esc: Quit application")

def test_gamepad_help():
    """Print gamepad controls"""
    print("\nGamepad Controls:")
    print("  A Button: Toggle AUTO/MANUAL mode")
    print("  B Button: Up movement")
    print("  X Button: Down movement")
    print("  Y Button: Left movement")
    print("  Left Bumper: Right movement")
    print("  Left Stick: Directional movement")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        if sys.argv[1] == "--help":
            test_keyboard_help()
            test_gamepad_help()
            sys.exit(0)
        elif sys.argv[1] == "--host" and len(sys.argv) > 2:
            host = sys.argv[2]
            test_udp_controller(host)
            sys.exit(0)
    
    test_udp_controller()
    test_keyboard_help()
    test_gamepad_help()