import importlib.util
from pathlib import Path
import struct
import unittest


MODULE_PATH = Path(__file__).parents[1] / "tools" / "receiver.py"
SPEC = importlib.util.spec_from_file_location("receiver", MODULE_PATH)
receiver = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(receiver)


class FakeSerial:
    def __init__(self, payload):
        self.payload = bytearray(payload)

    @property
    def in_waiting(self):
        return len(self.payload)

    def read(self, size):
        if not self.payload:
            raise EOFError("test stream exhausted")
        chunk = bytes(self.payload[:size])
        del self.payload[:size]
        return chunk


def frame(sequence, samples):
    payload = receiver.SYNC + struct.pack("<H", sequence)
    payload += struct.pack("<%dH" % len(samples), *samples)
    return payload + bytes([receiver.crc8(payload)])


class ReceiverTests(unittest.TestCase):
    def test_crc_reference_vector(self):
        self.assertEqual(receiver.crc8(b"123456789"), 0xF4)

    def test_decodes_frame(self):
        stream = receiver.frame_iter(FakeSerial(frame(42, (10, 20))), 2)
        self.assertEqual(next(stream), (42, (10, 20)))

    def test_recovers_sync_inside_corrupt_candidate(self):
        valid = frame(7, (100, 200))
        payload = receiver.SYNC + b"\x00\x00\x00" + valid
        stream = receiver.frame_iter(FakeSerial(payload), 2)
        self.assertEqual(next(stream), (7, (100, 200)))

    def test_rejects_invalid_channel_count(self):
        stream = receiver.frame_iter(FakeSerial(b""), 0)
        with self.assertRaises(ValueError):
            next(stream)


if __name__ == "__main__":
    unittest.main()
