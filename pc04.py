import urllib.request
import json

# Target the IP of your Arduino Q and the bridge.py port
BRIDGE_URL = "http://192.168.2.24:8080/command"
BRIDGE_URL = "http://100.85.119.51:8080/command"

def send_command(direction, duration=0.0, speed=150):
    # The args array now matches the C++ function: [String, float, int]
    payload = json.dumps({
        "method": "move",
        "args": [direction, float(duration), int(speed)]
    }).encode('utf-8')
    
    req = urllib.request.Request(
        BRIDGE_URL,
        data=payload,
        headers={"Content-Type": "application/json"}
    )
    
    print(f"Sending: {direction} ({duration}s @ PWM {speed})...")
    try:
        with urllib.request.urlopen(req, timeout=10) as response:
            result = json.loads(response.read().decode('utf-8'))
            if result.get("success"):
                print("  ✓ SUCCESS")
            else:
                print(f"  ✗ ERROR from robot: {result.get('error')}")
    except Exception as e:
        print(f"  ✗ HTTP Request failed: {e}")

print("=== Lafvin HTTP Variable Speed Control ===")
print("Commands: F (Forward), B (Backward), L (Left), R (Right), S (Stop)")
print("Format: <Command>:<Seconds>:<Speed> (e.g., F:2.5:200).")
print("Default if omitted is 1.0s at speed 150.")
print("Type 'Q' to quit.\n")

while True:
    cmd = input("Enter command: ").strip().upper()
    if cmd == 'Q':
        break
    if not cmd:
        continue
        
    action = cmd
    duration = 1.0
    speed = 150
    
    # Parse the input format
    if ":" in cmd:
        parts = cmd.split(":")
        action = parts[0]
        if len(parts) >= 2:
            try: duration = float(parts[1])
            except ValueError: pass
        if len(parts) >= 3:
            try: speed = int(parts[2])
            except ValueError: pass
    
    # Bound the PWM speed to safe analogWrite values
    speed = max(0, min(255, speed))
    
    if action == "F": send_command("forward", duration, speed)
    elif action == "B": send_command("back", duration, speed)
    elif action == "L": send_command("left", duration, speed)
    elif action == "R": send_command("right", duration, speed)
    elif action == "S": send_command("stop", 0.0, 0)
    else: print("Unknown command.")
