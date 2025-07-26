#!/usr/bin/env python3
"""
Simple test server for the Kria Native Controller CLIENT
Receives UDP commands and TCP connections from Kria app
"""

import socket
import threading
import time
import sys

def udp_server(host='0.0.0.0', port=8556):
    """UDP server to receive commands from Kria client"""
    
    print(f"Starting UDP server on {host}:{port}")
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((host, port))
    
    print("UDP server listening for Kria commands...")
    print("=" * 50)
    
    try:
        while True:
            # Receive data
            data, addr = sock.recvfrom(1024)
            command = data.decode('utf-8')
            timestamp = time.strftime("%H:%M:%S")
            
            print(f"[{timestamp}] FROM {addr[0]}:{addr[1]} -> {command}")
            
            # Parse command
            if command.startswith("BUTTON:"):
                button = command.split(":", 1)[1]
                print(f"  -> Button pressed: {button}")
            elif command.startswith("TOUCH:"):
                parts = command.split(":")
                if len(parts) >= 3:
                    x, y = parts[1], parts[2]
                    print(f"  -> Touch coordinate: ({x}, {y})")
            elif command.startswith("MODE:"):
                mode = command.split(":", 1)[1]
                print(f"  -> Mode changed: {mode}")
            
            # Send acknowledgment
            ack = f"ACK:{command}"
            sock.sendto(ack.encode('utf-8'), addr)
            
    except KeyboardInterrupt:
        print("\nUDP server stopped")
    finally:
        sock.close()

def tcp_server(host='0.0.0.0', port=8555):
    """TCP server to receive persistent connections from Kria client"""
    
    print(f"Starting TCP server on {host}:{port}")
    
    # Create TCP socket
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind((host, port))
    server_sock.listen(5)
    
    print("TCP server listening for Kria connections...")
    
    def handle_client(client_sock, addr):
        """Handle individual TCP client connection"""
        print(f"TCP client connected from {addr[0]}:{addr[1]}")
        
        # Send welcome message
        welcome = "WELCOME:Kria Test Server\n"
        client_sock.send(welcome.encode('utf-8'))
        
        try:
            while True:
                data = client_sock.recv(1024)
                if not data:
                    break
                
                commands = data.decode('utf-8').strip().split('\n')
                for command in commands:
                    if command:
                        timestamp = time.strftime("%H:%M:%S")
                        print(f"[{timestamp}] TCP FROM {addr[0]} -> {command}")
                        
                        # Send acknowledgment
                        ack = f"ACK:{command}\n"
                        client_sock.send(ack.encode('utf-8'))
                        
        except Exception as e:
            print(f"TCP client error: {e}")
        finally:
            print(f"TCP client {addr[0]}:{addr[1]} disconnected")
            client_sock.close()
    
    try:
        while True:
            client_sock, addr = server_sock.accept()
            # Handle each client in a separate thread
            client_thread = threading.Thread(target=handle_client, args=(client_sock, addr))
            client_thread.daemon = True
            client_thread.start()
            
    except KeyboardInterrupt:
        print("\nTCP server stopped")
    finally:
        server_sock.close()

def print_help():
    """Print usage information"""
    print("Kria Test Server - Receives commands from Kria CLIENT")
    print("=" * 50)
    print()
    print("This server receives commands from the Kria application.")
    print("Expected command formats:")
    print("  BUTTON:UP, BUTTON:DOWN, BUTTON:LEFT, BUTTON:RIGHT")
    print("  TOUCH:x:y (coordinates from AUTO mode)")
    print("  MODE:AUTO, MODE:MANUAL")
    print()
    print("Usage:")
    print("  python3 test_server.py [--udp-only] [--tcp-only] [--help]")
    print()
    print("Options:")
    print("  --udp-only    Start only UDP server")
    print("  --tcp-only    Start only TCP server")
    print("  --help        Show this help")
    print()

if __name__ == "__main__":
    if "--help" in sys.argv:
        print_help()
        sys.exit(0)
    
    udp_only = "--udp-only" in sys.argv
    tcp_only = "--tcp-only" in sys.argv
    
    print("Kria Test Server")
    print("Waiting for commands from Kria CLIENT application...")
    print("Press Ctrl+C to stop")
    print()
    
    threads = []
    
    try:
        if not tcp_only:
            # Start UDP server in a thread
            udp_thread = threading.Thread(target=udp_server)
            udp_thread.daemon = True
            udp_thread.start()
            threads.append(udp_thread)
        
        if not udp_only:
            # Start TCP server in a thread
            tcp_thread = threading.Thread(target=tcp_server)
            tcp_thread.daemon = True
            tcp_thread.start()
            threads.append(tcp_thread)
        
        # Keep main thread alive
        while True:
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nShutting down servers...")
        sys.exit(0)