"""Verify V8:2's Dreamcast table-fog contract against the retail binary.

The guard anchors the COLS fog-colour source, PVR density packing, custom
129-sample table builder, and hardware-register flush path. It deliberately
checks the executable directly rather than accepting a screenshot-derived
curve or a host-renderer tuning value.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct


IMAGE_BASE = 0x8C010000
IMAGE_SHA256 = "06f5a6fcdc9f8e0ef486a1ce1df7fcf61f5a756dbef7248139291ae520e3d3e7"


def verify(image: Path):
    data = image.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != IMAGE_SHA256:
        raise ValueError(f"Unexpected Dreamcast executable SHA-256: {digest}")

    def u16(address):
        return struct.unpack_from("<H", data, address - IMAGE_BASE)[0]

    def u32(address):
        return struct.unpack_from("<I", data, address - IMAGE_BASE)[0]

    def f32(address):
        return struct.unpack_from("<f", data, address - IMAGE_BASE)[0]

    instructions = {
        0x8C043C80: 0xD307,  # density setter
        0x8C043CC0: 0xD207,  # table-colour setter
        0x8C043D00: 0xC718,  # custom 129-sample table packer
        0x8C046840: 0x2FE6,  # dirty fog-state register flush
        0x8C084700: 0x2F86,  # COLS loader
        0x8C094640: 0xF41D,  # IEEE-754 to PVR density packer
        0x8C094680: 0x2F86,  # retail level fog builder
        0x8C0946AE: 0x2F72,  # preserve packed RGB argument across colour call
        0x8C0946B6: 0xF408,  # load 0.276f density
        0x8C0946D4: 0x9620,  # 129 table samples (last index 128)
        0x8C094702: 0xF71A,  # store generated table sample
    }
    for address, expected in instructions.items():
        actual = u16(address)
        if actual != expected:
            raise ValueError(
                f"Instruction mismatch at {address:08x}: "
                f"{actual:04x} != {expected:04x}"
            )

    words = {
        # COLS word destinations: word one is the fog RGB; words three/four
        # are the independent terrain colour-ramp endpoints.
        0x8C084AC8: 0x8C288920,
        0x8C084ACC: 0x8C288904,
        0x8C084AD0: 0x8C28EC18,
        0x8C084AD4: 0x8C28F230,
        0x8C084AD8: 0x8C28FA40,
        0x8C084ADC: 0x8C2905D8,
        0x8C084AE0: 0x8C28EC24,
        # Fog builder API and storage anchors.
        0x8C09471C: 0x8C288904,
        0x8C094720: 0x8C043CC0,
        0x8C094724: 0x8C094640,
        0x8C09472C: 0x8C043C80,
        0x8C094730: 0x8C28EC28,
        0x8C094734: 0x8C043D00,
        0x8C043D68: 0x8C349324,
        0x8C043D6C: 0x8C349524,
        0x8C0468CC: 0x8C04E040,
        0x8C0468D0: 0x8C34952C,
        0x8C0468D4: 0x8C349530,
    }
    for address, expected in words.items():
        actual = u32(address)
        if actual != expected:
            raise ValueError(
                f"Word mismatch at {address:08x}: "
                f"{actual:08x} != {expected:08x}"
            )

    constants = {
        "requested_density": (0x8C094728, 0.27600002),
        "table_numerator": (0x8C094738, 4.4160004),
        "visibility_low": (0x8C09473C, 0.080000006),
        "visibility_high": (0x8C094740, 0.13800001),
        "visibility_inverse_span": (0x8C094744, 17.241377),
        "table_quantizer": (0x8C043D64, 255.0),
    }
    decoded = {}
    for name, (address, expected) in constants.items():
        actual = f32(address)
        tolerance = max(abs(expected) * 1e-6, 1e-7)
        if abs(actual - expected) > tolerance:
            raise ValueError(
                f"Float mismatch at {address:08x}: {actual!r} != {expected!r}"
            )
        decoded[name] = actual

    requested = f32(0x8C094728)
    bits = struct.unpack("<I", struct.pack("<f", requested))[0]
    packed = ((((bits & 0x007F0000) | 0x00800000) >> 8) |
              (((bits & 0x7F800000) >> 23) - 127) & 0xFF)
    exponent = struct.unpack("b", bytes([packed & 0xFF]))[0]
    mantissa = (packed >> 8) & 0xFF
    hardware_density = mantissa / 128.0 * (2.0 ** exponent)

    return {
        "sha256": digest,
        "builder": {
            "entry": "8c094680",
            "fog_rgb": "COLS word 1 -> 8c288904",
            "terrain_ramp": [
                "COLS word 3 -> 8c28f230",
                "COLS word 4 -> 8c28fa40",
            ],
            "samples": 129,
            "packed_entries": 128,
            "quantization": "truncate(sample * 255)",
        },
        "density": {
            "requested": requested,
            "packed": f"{packed:04x}",
            "hardware": hardware_density,
        },
        "constants": decoded,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.image), indent=2))


if __name__ == "__main__":
    main()
