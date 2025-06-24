import socket
import struct
import os
import argparse
import time

DOWNLOAD_PACKET_TYPE = 0x01
DATA_PACKET_FLAG     = 0xDD
PACKET_DATA_SIZE     = 31       # payload
FILENAME_MAXLEN      = 8 + 1 + 3

def send_file(server_ip: str, server_port: int, file_path: str):
    file_size = os.path.getsize(file_path)
    file_name = os.path.basename(file_path).encode('utf-8')
    if len(file_name) > FILENAME_MAXLEN:
        raise ValueError(f"Имя файла слишком длинное (макс {FILENAME_MAXLEN} байт)")

    service_pkt = struct.pack('<B', DOWNLOAD_PACKET_TYPE)
    service_pkt += struct.pack('<I', file_size)
    service_pkt += file_name.ljust(FILENAME_MAXLEN, b'\x00')

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((server_ip, server_port))
        sock.sendall(service_pkt)
        print(f"[+] Sent service packet: type=0x{DOWNLOAD_PACKET_TYPE:02X}, "
              f"size={file_size}, name={file_name!r}")
        time.sleep(1)
        with open(file_path, 'rb') as f:
            while True:
                chunk = f.read(PACKET_DATA_SIZE)
                if not chunk:
                    break
                if len(chunk) < PACKET_DATA_SIZE:
                    chunk = chunk.ljust(PACKET_DATA_SIZE, b'\x00')
                pkt = struct.pack('!B', DATA_PACKET_FLAG) + chunk
                sock.sendall(pkt)
        print(f"[+] File '{file_path}' sent successfully.")

if __name__ == "__main__":
    p = argparse.ArgumentParser(description="Python client for ESP file transfer")
    p.add_argument("host", help="IP address of ESP server")
    p.add_argument("port", type=int, help="Port number of ESP server")
    p.add_argument("file", help="Path to the file to send")
    args = p.parse_args()
    send_file(args.host, args.port, args.file)
