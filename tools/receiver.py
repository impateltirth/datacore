#!/usr/bin/env python3
"""
DataCore host receiver.

Reads the framed ADC stream from the DataCore firmware, validates CRC-8,
and logs timestamped samples to CSV. Optionally shows a live plot.

Packet (little-endian): AA 55 | seq u16 | ch0..chN u16 | CRC-8 (poly 0x07)

Usage:
    python3 receiver.py --port /dev/ttyUSB0 --baud 921600 --channels 4
    python3 receiver.py --port /dev/ttyUSB0 --baud 921600 --channels 4 --csv log.csv
    python3 receiver.py --port /dev/ttyUSB0 --baud 921600 --channels 4 --plot
"""

import argparse
import struct
import sys
import time

SYNC = b"\xAA\x55"


def crc8(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ 0x07) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc


def frame_iter(ser, n_channels):
    """Yield (seq, samples) tuples, resynchronizing on CRC failure."""
    frame_len = 2 + 2 + 2 * n_channels + 1
    buf = bytearray()
    while True:
        chunk = ser.read(max(1, ser.in_waiting or 1))
        if not chunk:
            continue
        buf += chunk
        while True:
            i = buf.find(SYNC)
            if i < 0:
                buf = buf[-1:]  # keep a possible partial sync
                break
            if len(buf) - i < frame_len:
                del buf[:i]
                break
            frame = bytes(buf[i:i + frame_len])
            del buf[:i + frame_len]
            if crc8(frame[:-1]) != frame[-1]:
                continue  # corrupt frame: drop and resync
            seq = struct.unpack_from("<H", frame, 2)[0]
            samples = struct.unpack_from("<%dH" % n_channels, frame, 4)
            yield seq, samples


def main():
    ap = argparse.ArgumentParser(description="DataCore stream receiver")
    ap.add_argument("--port", required=True, help="serial port, e.g. /dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=921600)
    ap.add_argument("--channels", type=int, default=4)
    ap.add_argument("--csv", help="log samples to this CSV file")
    ap.add_argument("--plot", action="store_true", help="live plot (needs matplotlib)")
    args = ap.parse_args()

    try:
        import serial
    except ImportError:
        sys.exit("pyserial is required: pip install pyserial")

    csv_file = None
    if args.csv:
        csv_file = open(args.csv, "w")
        header = "t,seq," + ",".join("ch%d" % i for i in range(args.channels)) + "\n"
        csv_file.write(header)

    plotter = None
    if args.plot:
        try:
            import matplotlib.pyplot as plt
            from collections import deque
            fig, ax = plt.subplots()
            history = [deque(maxlen=500) for _ in range(args.channels)]
            lines = [ax.plot([], [])[0] for _ in range(args.channels)]
            ax.set_ylim(0, 4095)
            ax.legend(["ch%d" % i for i in range(args.channels)])
            plt.ion()
            plt.show(block=False)
            plotter = (plt, ax, history, lines)
        except ImportError:
            sys.exit("matplotlib is required for --plot: pip install matplotlib")

    ser = serial.Serial(args.port, args.baud, timeout=1)
    t0 = time.time()
    n_frames = n_bad = 0
    last_seq = None
    print("Listening on %s @ %d baud, %d channels. Ctrl-C to stop."
          % (args.port, args.baud, args.channels))
    try:
        for seq, samples in frame_iter(ser, args.channels):
            n_frames += 1
            if last_seq is not None and (seq - last_seq) % 65536 != 1:
                n_bad += 1  # gap: MCU-side drop or serial overrun
            last_seq = seq
            t = time.time() - t0
            if csv_file:
                csv_file.write("%.6f,%d,%s\n"
                               % (t, seq, ",".join(map(str, samples))))
            if plotter:
                plt, ax, history, lines = plotter
                for i, s in enumerate(samples):
                    history[i].append(s)
                    lines[i].set_data(range(len(history[i])), list(history[i]))
                ax.set_xlim(0, max(500, len(history[0])))
                plt.pause(0.001)
            if n_frames % 1000 == 0:
                print("frames=%d gaps=%d" % (n_frames, n_bad), flush=True)
    except KeyboardInterrupt:
        pass
    finally:
        print("\nDone: %d frames, %d sequence gaps." % (n_frames, n_bad))
        ser.close()
        if csv_file:
            csv_file.close()


if __name__ == "__main__":
    main()
