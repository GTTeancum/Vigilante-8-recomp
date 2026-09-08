"""Verify the recovered Dreamcast sky renderer against the retail binary.

This guard anchors the XBGM loader and camera-relative multi-strip renderer to
the authoritative US Dreamcast executable.  It deliberately verifies code and
constants, not a screenshot or a reconstructed VRAM image.
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

    # Loader: three big-endian XBGM header values become segment count, width,
    # and height, followed by one PVRT load and half-texel UV construction.
    instruction_bytes = {
        0x8C085B44: "122e",
        0x8C085B4E: "122c",
        0x8C085B58: "122d",
        0x8C085B6E: "0b41",
        0x8C085B86: "10f1",
        0x8C085B8A: "13f5",
        0x8C085BB0: "dcf4",
        0x8C085BB4: "11f4",
        # First and second camera-relative four-vertex strips terminate only
        # on vertex three.
        0x8C095F82: "0388",
        0x8C096088: "03e1",
        0x8C09608A: "173d",
        0x8C09619E: "0388",
        0x8C09624C: "03e3",
        0x8C09624E: "373d",
        # Final six-vertex horizon/wrap strip selects an end-of-strip command
        # whenever the one-based vertex count is divisible by three.
        0x8C0964A2: "1d38",
        0x8C0964B4: "1038",
        0x8C0964BC: "1927",
        0x8C0964C0: "1b27",
        0x8C0964D0: "05e1",
        0x8C0964D2: "173d",
    }
    for address, expected in instruction_bytes.items():
        actual = data[address - IMAGE_BASE : address - IMAGE_BASE + 2].hex()
        if actual != expected:
            raise ValueError(
                f"Instruction mismatch at {address:08x}: "
                f"{actual} != {expected}"
            )

    words = {
        # XBGM loader destinations and helpers.
        0x8C085CE4: 0x8C28EC1C,
        0x8C085CEC: 0x8C2748A0,
        0x8C085CF0: 0x8C288924,
        0x8C085D00: 0x8C084B80,
        0x8C085D10: 0x8C223F5E,
        0x8C085D14: 0x8C28FA8C,
        0x8C085D1C: 0x8C2888C0,
        0x8C085D20: 0x8C28FA4C,
        # Sky renderer source blocks and material/header pipeline.
        0x8C095FCC: 0x8C28FA8C,
        0x8C095FD0: 0x8C2888C0,
        0x8C095FD4: 0x8C2430C0,
        0x8C095FD8: 0x00080000,
        0x8C095FDC: 0x8C2905D0,
        0x8C095FE4: 0x8C113DB4,
        0x8C095FE8: 0x8C041E60,
        0x8C095FEC: 0x8C069160,
        0x8C095FF0: 0x8C244080,
        # PVR strip commands, UVs, and final six-vertex index table.
        0x8C096010: 0xE0000000,
        0x8C0961BC: 0xF0000000,
        0x8C0961C4: 0x8C223F58,
        0x8C0961CC: 0x8C095BE0,
        0x8C0961D8: 0x8C289158,
        0x8C0963FC: 0x8C2747C4,
        0x8C096524: 0x8C113F00,
        0x8C09652C: 0x7C001004,
        0x8C096530: 0x8C095BE0,
        0x8C096534: 0x55555556,
        0x8C096538: 0xF0000000,
        0x8C09653C: 0xE0000000,
        0x8C096544: 0x8C095C00,
    }
    for address, expected in words.items():
        actual = u32(address)
        if actual != expected:
            raise ValueError(
                f"Word mismatch at {address:08x}: "
                f"{actual:08x} != {expected:08x}"
            )

    constants = {
        "camera_angle_scale": (0x8C095F94, 0.001335144),
        "angle_wrap": (0x8C095FAC, 16384.0),
        "backdrop_depth": (0x8C095FC8, -70.0),
        "projection_to_uv": (0x8C096038, 0.0125),
        "second_projection_to_uv": (0x8C0961EC, 0.0125),
        "horizon_extent": (0x8C096400, 70.0),
        "horizon_depth": (0x8C096418, -70.0),
        "inverse_horizon_extent": (0x8C096420, 1.0 / 70.0),
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

    return {
        "sha256": digest,
        "loader": {
            "entry": "8c085b20",
            "header_fields": ["segment_count", "width", "height"],
            "uv_rule": "half-texel inset from XBGM width and height",
            "vector_blocks": ["8c28fa8c", "8c2888c0", "8c28fa4c"],
        },
        "renderer": {
            "entry": "8c095c40",
            "camera_relative_strips": [4, 4],
            "horizon_wrap_vertices": 6,
            "horizon_strip_breaks_after_vertices": [3, 6],
            "material_context": "8c2430c0",
            "texture_binding": "8c069160",
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
