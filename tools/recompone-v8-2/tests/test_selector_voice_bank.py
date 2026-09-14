import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from build_v8_selector_voice_bank import (
    build, parse_bank, sample_bytes, split_resident_bank,
    EXPECTED_V8_SELECTION_VOICES, V8_ROSTER_RECORD_OFFSET,
)


def make_bank(count):
    table, payload = [], bytearray()
    for i in range(count):
        table.append((len(payload) // 8, 1024 + i))
        payload += bytes([i]) * (16 * (i + 1))
    return (struct.pack('<HH', count, len(payload) // 8)
            + b''.join(struct.pack('<HH', *e) for e in table) + payload)


class SelectorVoiceTests(unittest.TestCase):
    def setUp(self):
        self.dll = bytearray(V8_ROSTER_RECORD_OFFSET + 20 * 13)
        for i, voice in enumerate(EXPECTED_V8_SELECTION_VOICES):
            self.dll[V8_ROSTER_RECORD_OFFSET + i * 20 + 12] = voice
        self.v8, self.v82 = make_bank(30), make_bank(14)
        self.expanded, _ = build(self.dll, self.v8, self.v82)
        self.resident, self.voices = split_resident_bank(self.expanded)

    def test_retail_samples_and_pitches_unchanged(self):
        a, ap = parse_bank(self.v82)
        b, bp = parse_bank(self.resident)
        self.assertEqual(a, b[:14])
        for i in range(14):
            self.assertEqual(sample_bytes(a, ap, i), sample_bytes(b, bp, i))

    def test_source_mapping_is_not_sample_order(self):
        a, ap = parse_bank(self.v8)
        b, bp = parse_bank(self.voices)
        self.assertEqual(len(b), 13)
        for i, source in enumerate(EXPECTED_V8_SELECTION_VOICES):
            self.assertEqual(sample_bytes(a, ap, source), sample_bytes(b, bp, i))
            self.assertEqual(a[source][1], b[i][1])

    def test_two_independent_slots_fit_every_sample(self):
        a, ap = parse_bank(self.resident)
        b, bp = parse_bank(self.voices)
        self.assertEqual(len(a), 16)
        maximum = max(len(sample_bytes(b, bp, i)) for i in range(13))
        for i in (14, 15):
            self.assertEqual(a[i][1], 0)
            self.assertEqual(len(sample_bytes(a, ap, i)), maximum)

    def test_rebuild_resident_pair_is_idempotent(self):
        rebuilt, _ = build(self.dll, self.v8, self.resident)
        self.assertEqual(split_resident_bank(rebuilt), (self.resident, self.voices))


if __name__ == '__main__':
    unittest.main()
