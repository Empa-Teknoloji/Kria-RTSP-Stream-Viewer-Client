#!/usr/bin/env python3
"""
Debug UDP listener to check if Kria is sending commands
"""

import socket
import sys

def debug_udp_listener(host='0.0.0.0', port=8556):
    """Simple UDP listener with verbose output"""
    
    print(f"DEBUG: Starting UDP listener on {host}:{port}")
    print("This will show ALL UDP traffic received on this port")
    print("=" * 60)
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        sock.bind((host, port))
        print(f"✓ Successfully bound to {host}:{port}")
        print("Waiting for UDP packets...")
        print()
        
        while True:
            try:
                # Receive data with longer timeout
                sock.settimeout(1.0)
                data, addr = sock.recvfrom(1024)
                
                # Decode and display
                try:
                    message = data.decode('utf-8')
                    print(f"📦 FROM {addr[0]}:{addr[1]}")
                    print(f"   Raw bytes: {data}")
                    print(f"   Decoded  : '{message}'")
                    print(f"   Length   : {len(data)} bytes")
                    
                    # Send acknowledgment
                    ack = f"ACK:{message}"
                    sock.sendto(ack.encode('utf-8'), addr)
                    print(f"   ✓ Sent ACK: {ack}")
                    
                except UnicodeDecodeError:
                    print(f"📦 FROM {addr[0]}:{addr[1]} (Binary data)")
                    print(f"   Raw bytes: {data}")
                    print(f"   Length   : {len(data)} bytes")
                
                print("-" * 40)
                
            except socket.timeout:
                # Just continue waiting
                pass
                
    except PermissionError:
        print(f"❌ Permission denied. Try running as root or use a port > 1024")
        sys.exit(1)
    except OSError as e:
        print(f"❌ Socket error: {e}")
        print("   Make sure no other process is using this port")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n🛑 Stopped by user")
    finally:
        sock.close()
        print("Socket closed")

if __name__ == "__main__":
    port = 8556
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("Usage: python3 debug_udp.py [port]")
            sys.exit(1)
    
    print("Kria UDP Debug Listener")
    print("======================")
    print()
    print("Instructions:")
    print("1. Run this script")
    print("2. Start Kria application")
    print("3. Press arrow keys or click buttons")
    print("4. Watch for UDP packets here")
    print()
    
    debug_udp_listener(port=port)