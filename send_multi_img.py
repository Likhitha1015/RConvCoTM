import socket
import os

# ----------------------------
# FPGA TCP Configuration
# ----------------------------
ip_address = "192.168.1.10"   # FPGA IP address
port_number = 6001            # TCP port number

# ----------------------------
# Bitfile Options
# ----------------------------
bitfiles = {
    "1": "fmnist_10ktestpadded.bit",
    "2": "kmnist_10ktestpadded.bit",
    "3": "mnist_10ktestpadded.bit"
}

print("Select bitfile to send:")
print("1 → FMNIST")
print("2 → KMNIST")
print("3 → MNIST")

user_choice = input("Enter choice (1/2/3): ").strip()

if user_choice not in bitfiles:
    raise ValueError("Invalid choice! Please run again and enter 1, 2, or 3.")

bitfile_path = bitfiles[user_choice]
print(f"\nSelected bitfile: {bitfile_path}\n")

# ----------------------------
# Constants
# ----------------------------
LINE_SIZE_BITS = 512
LINE_SIZE_BYTES = LINE_SIZE_BITS // 8  # 64 bytes per line

# ----------------------------
# Utility Functions
# ----------------------------
def recv_exact(sock, size):
    data = bytearray()
    while len(data) < size:
        packet = sock.recv(size - len(data))
        if not packet:
            raise ConnectionError("Connection lost during recv_exact()")
        data.extend(packet)
    return data

def reverse_16_blocks(data):
    output = bytearray()
    for i in range(0, len(data), 16):
        output.extend(data[i:i+16][::-1])
    return output

# ----------------------------
# Main Execution
# ----------------------------
print(f"Connecting to FPGA at {ip_address}:{port_number} ...")
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(150)
s.connect((ip_address, port_number))
print("Connected.\n")

try:
    with open(bitfile_path, 'r') as f:
        line_count = 0
        for line in f:
            line = line.strip()
            if not line:
                continue

            if len(line) != LINE_SIZE_BITS:
                raise ValueError(f"Line {line_count+1}: Expected {LINE_SIZE_BITS} bits, got {len(line)}")

            val = int(line, 2)
            data = val.to_bytes(LINE_SIZE_BYTES, byteorder='big')
            data = reverse_16_blocks(data)

            s.sendall(data)

            ack = recv_exact(s, 3)
            if ack != b'ACK':
                raise RuntimeError(f"No ACK for line {line_count+1}, got: {ack}")

            line_count += 1
            if line_count % 1000 == 0:
                print(f"{line_count} lines sent successfully...")

    print(f"\n✅ Transmission complete — {line_count} binary lines sent successfully with ACKs.")

finally:
    s.close()
    print("\n🔒 Connection closed.")
