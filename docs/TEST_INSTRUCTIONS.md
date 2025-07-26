# Testing Kria CLIENT - Step by Step

## Problem Fixed ✅
- **Server binding**: Fixed to listen on `0.0.0.0` (all interfaces)
- **Client address**: Changed to `127.0.0.1` (localhost) for testing
- **Button handling**: Fixed arrow button clicks to send server commands
- **Direction mapping**: Fixed UP/DOWN/LEFT/RIGHT to arrow symbol conversion

## Step 1: Start the Debug UDP Listener

Open Terminal 1:
```bash
cd /Users/osmancangudul/kria
python3 debug_udp.py
```

You should see:
```
DEBUG: Starting UDP listener on 0.0.0.0:8556
✓ Successfully bound to 0.0.0.0:8556
Waiting for UDP packets...
```

## Step 2: Start Kria Application

Open Terminal 2:
```bash
cd /Users/osmancangudul/kria/build
./kria.app/Contents/MacOS/kria
```

Look for these log messages:
```
Starting Native Controller as CLIENT
Server Address: 127.0.0.1
UDP Port: 8556
```

## Step 3: Test Controls

### Test Keyboard Controls
In the Kria window, press:
- **Arrow Keys** (Up/Down/Left/Right)
- **WASD Keys**
- **Space Bar** (mode toggle)

### Test Mouse Controls  
- **Click anywhere** in the video area (AUTO mode only)

### Test UI Buttons
- **Toggle AUTO/MANUAL** button (bottom right)
- **Arrow buttons** (only visible in MANUAL mode)

## Expected Output in Debug Listener

When you press **Up Arrow** key:
```
📦 FROM 127.0.0.1:54321
   Raw bytes: b'BUTTON:UP'
   Decoded  : 'BUTTON:UP'
   Length   : 9 bytes
   ✓ Sent ACK: ACK:BUTTON:UP
```

When you press **Space** (mode toggle):
```
📦 FROM 127.0.0.1:54321
   Raw bytes: b'MODE:MANUAL'
   Decoded  : 'MODE:MANUAL'
   Length   : 11 bytes
   ✓ Sent ACK: ACK:MODE:MANUAL
```

When you **click mouse** in AUTO mode:
```
📦 FROM 127.0.0.1:54321
   Raw bytes: b'TOUCH:320:240'
   Decoded  : 'TOUCH:320:240'
   Length   : 13 bytes
   ✓ Sent ACK: ACK:TOUCH:320:240
```

## Troubleshooting

### No UDP packets received?
1. Check Kria logs for "UDP command sent:" messages
2. Verify debug listener shows "✓ Successfully bound"
3. Try running debug listener as root: `sudo python3 debug_udp.py`
4. Check firewall settings

### Commands not being sent?
1. Make sure you're in the right mode:
   - **Arrow buttons**: Only work in MANUAL mode
   - **Mouse clicks**: Only work in AUTO mode
   - **Keyboard**: Works in both modes
2. Check Kria application logs for debug messages

### Connection errors?
1. Verify server address in mainwindow.h is set to `127.0.0.1`
2. Check that UDP port 8556 is not in use by another process

## Alternative Testing

### Use the Full Test Server
```bash
python3 test_server.py --udp-only
```

### Test with netcat
```bash
# Listen for UDP packets
nc -ul 8556
```

## Production Use

When ready for production:
1. Change `m_tcpAddress` in `mainwindow.h` to your actual server IP
2. Rebuild the application
3. Run your actual server on that IP

## Command Reference

Kria sends these commands:
- `BUTTON:UP` / `BUTTON:DOWN` / `BUTTON:LEFT` / `BUTTON:RIGHT`
- `MODE:AUTO` / `MODE:MANUAL`  
- `TOUCH:x:y` (coordinates)