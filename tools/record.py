#!/usr/bin/env python3
"""
WAV Receiver - TCP server that receives WAV files from the ESP32 glove.

Usage:
    python wav_receiver.py [--host 0.0.0.0] [--port 12345] [--output-dir ./recordings]
"""

import argparse
import socket
import os
import struct
from datetime import datetime


def parse_wav_header(data: bytes) -> dict:
    if len(data) < 44:
        return {}

    fields = {}
    fields["riff"] = data[0:4].decode("ascii", errors="replace")
    fields["file_size"] = struct.unpack_from("<I", data, 4)[0]
    fields["wave"] = data[8:12].decode("ascii", errors="replace")
    fields["audio_format"] = struct.unpack_from("<H", data, 20)[0]
    fields["channels"] = struct.unpack_from("<H", data, 22)[0]
    fields["sample_rate"] = struct.unpack_from("<I", data, 24)[0]
    fields["byte_rate"] = struct.unpack_from("<I", data, 28)[0]
    fields["bits_per_sample"] = struct.unpack_from("<H", data, 34)[0]
    fields["data_size"] = struct.unpack_from("<I", data, 40)[0]
    return fields


def receive_wav(conn: socket.socket, addr: tuple, output_dir: str):
    print(f"Connection from {addr[0]}:{addr[1]}")

    data = bytearray()
    while True:
        chunk = conn.recv(4096)
        if not chunk:
            break
        data.extend(chunk)

    conn.close()

    if len(data) < 44:
        print(f"  ERROR: Received only {len(data)} bytes (need at least 44 for WAV header)")
        return

    header = parse_wav_header(bytes(data[:44]))
    pcm_size = len(data) - 44
    duration = pcm_size / header.get("byte_rate", 1) if header.get("byte_rate") else 0

    print(f"  Received: {len(data)} bytes total")
    print(
        f"  Format:   {header.get('channels', '?')}ch, "
        f"{header.get('sample_rate', '?')}Hz, "
        f"{header.get('bits_per_sample', '?')}bit"
    )
    print(f"  Duration: {duration:.2f} seconds")
    print(f"  PCM data: {pcm_size} bytes")

    if header.get("riff") != "RIFF" or header.get("wave") != "WAVE":
        print(
            f"  WARNING: Invalid WAV header (RIFF={header.get('riff')}, WAVE={header.get('wave')})"
        )

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"recording_{timestamp}.wav"
    filepath = os.path.join(output_dir, filename)

    os.makedirs(output_dir, exist_ok=True)
    with open(filepath, "wb") as f:
        f.write(data)

    print(f"  Saved to: {filepath}\n")


def main():
    parser = argparse.ArgumentParser(description="Receive WAV files from ESP32 over TCP")
    parser.add_argument("--host", default="0.0.0.0", help="Bind address (default: 0.0.0.0)")
    parser.add_argument("--port", type=int, default=12345, help="Listen port (default: 12345)")
    parser.add_argument(
        "--output-dir", default="./recordings", help="Output directory (default: ./recordings)"
    )
    args = parser.parse_args()

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((args.host, args.port))
    server.listen(1)

    print(f"WAV Receiver listening on {args.host}:{args.port}")
    print(f"Output directory: {os.path.abspath(args.output_dir)}")
    print("Waiting for connections... (Ctrl+C to stop)\n")

    try:
        while True:
            conn, addr = server.accept()
            receive_wav(conn, addr, args.output_dir)
    except KeyboardInterrupt:
        print("\nShutting down.")
    finally:
        server.close()


if __name__ == "__main__":
    main()
