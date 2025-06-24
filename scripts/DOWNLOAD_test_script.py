import socket
import argparse
import os

def main(ip: str, port: int, filename: str):
    pkt_type = b"\x02"
    encoded_name = filename.encode('utf-8')
    if len(encoded_name) > 20:
        encoded_name = encoded_name[:20]
    else:
        encoded_name = encoded_name.ljust(20, b'\x00')
    packet = pkt_type + encoded_name
    packet += b"\x00" * (32 - len(packet))

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ip, port))
    print(f"Connected to {ip}:{port}")

    sock.sendall(packet)
    print(f"Sent service packet for file '{filename}'")

    with open(filename, 'wb') as f:
        while True:
            data = sock.recv(32)
            if not data:
                print("Connection closed by server")
                break
            if len(data) < 32:
                print(f"Received incomplete packet: {len(data)} bytes")
                break
            if data[0] != 0xDD:
                print(f"Error: Unexpected packet type 0x{data[0]:02X}")
                continue
            f.write(data[1:])

    sock.close()
    print("Client terminated")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Simple file download client")
    parser.add_argument('ip', help='Server IP address')
    parser.add_argument('port', type=int, help='Server port')
    parser.add_argument('filename', help='Name of the file to request and save')
    args = parser.parse_args()
    main(args.ip, args.port, args.filename)
