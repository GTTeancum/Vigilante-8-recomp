"""Verify the retail Dreamcast world-object/building draw contract.

This binary guard anchors object culling, material identity, mesh emission,
and the object emitter's near-plane clipping to the authoritative US
1ST_READ.BIN.  In particular it demonstrates that the ordinary object walker
does not choose a different texture or material as a function of camera
distance, and that partly visible object triangles are clipped and rebuilt
rather than being dropped at the viewport edge.
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
        0x8C0A7930: 0x780C,  # object center is object + 12
        0x8C0A7932: 0x7A18,  # object radius is object + 24
        0x8C0A7934: 0xF4A8,  # load radius
        0x8C0A793A: 0x470B,  # center/radius visibility test
        0x8C0A794A: 0x400B,  # transform object center
        0x8C0A7958: 0xF125,  # far depth > transformed depth
        0x8C0A7962: 0x470B,  # construct object transform
        0x8C0A79A2: 0x51A2,  # material descriptor pointer at object + 8
        0x8C0A79AA: 0x3170,  # compare with last material identity
        0x8C0A79B2: 0x171C,  # store changed material at context + 48
        0x8C0A79B6: 0x410B,  # compile changed material/header
        0x8C0A79E8: 0x410B,  # emit the object's mesh
    }
    for address, expected in instructions.items():
        actual = u16(address)
        if actual != expected:
            raise ValueError(
                f"Instruction mismatch at {address:08x}: "
                f"{actual:04x} != {expected:04x}"
            )

    mesh_instructions = {
        0x8C0A6F5C: 0x7C20,  # mesh vertex stream begins at mesh + 0x20
        0x8C0A6F6A: 0x5087,  # mesh count at mesh + 0x1c
        0x8C0A6F70: 0x403D,  # 32-byte source records
        0x8C0A6F84: 0x490B,  # project source vertex through 0x8c0a6cc0
        0x8C0A6FB2: 0xC73D,  # load exact 0.01 near-plane literal
        0x8C0A6FB6: 0xF215,  # classify first vertex against near plane
        0x8C0A6FBE: 0xF215,  # classify second vertex against near plane
        0x8C0A6FEE: 0xF215,  # classify third vertex against near plane
        0x8C0A700E: 0xD127,  # load ordinary PVR vertex command
        0x8C0A7010: 0x2B12,  # write command to 32-byte output vertex
        0x8C0A7048: 0x0B83,  # submit/prefetch completed output vertex
        0x8C0A705A: 0xCA01,  # preserve triangle-strip parity
        0x8C0A706C: 0x3216,  # dispatch the six nontrivial clip masks
        0x8C0A75CA: 0xD121,  # trailing visible-vertex command
        0x8C0A7602: 0xD116,  # trailing end-of-strip command
        0x8C0A762A: 0x0B83,  # submit/prefetch trailing vertex
        0x8C0A762C: 0x3B6C,  # advance output by 32 bytes
    }
    for address, expected in mesh_instructions.items():
        actual = u16(address)
        if actual != expected:
            raise ValueError(
                f"Mesh instruction mismatch at {address:08x}: "
                f"{actual:04x} != {expected:04x}"
            )

    words = {
        0x8C0A7A48: 0x8C244080,
        0x8C0A7A4C: 0x8C243840,
        0x8C0A7A50: 0x8C0440E0,
        0x8C0A7A54: 0x8C044300,
        0x8C0A7A68: 0x8C2B2A04,
        0x8C0A7A6C: 0x8C0A6620,
        0x8C0A7A70: 0x8C2B28C8,
        0x8C0A7A74: 0x8C0A4C80,
        0x8C0A7A7C: 0x8C100392,
        0x8C0A7A80: 0x8C24387C,
        0x8C0A7A84: 0x8C041E60,
        0x8C0A7A88: 0x8C04182A,
        0x8C0A7A8C: 0x8C0A6F40,
    }
    for address, expected in words.items():
        actual = u32(address)
        if actual != expected:
            raise ValueError(
                f"Word mismatch at {address:08x}: "
                f"{actual:08x} != {expected:08x}"
            )

    mesh_words = {
        0x8C0A7098: 0x7C001000,  # transformed-vertex scratch
        0x8C0A709C: 0x7C002000,  # parallel projected scratch
        0x8C0A70A0: 0x8C2747CC,  # projection scale
        0x8C0A70A4: 0x8C0A6CC0,  # vertex projection helper
        0x8C0A70AC: 0xE0000000,  # ordinary PVR vertex
        0x8C0A70B8: 0x8C0A6D00,  # clipped-edge interpolation helper
        0x8C0A7648: 0x8C0A6EC0,  # complementary clip helper
        0x8C0A764C: 0x8C0A6D00,  # clipped-edge interpolation helper
        0x8C0A7650: 0xE0000000,  # ordinary PVR vertex
        0x8C0A765C: 0xF0000000,  # end-of-strip PVR vertex
    }
    for address, expected in mesh_words.items():
        actual = u32(address)
        if actual != expected:
            raise ValueError(
                f"Mesh word mismatch at {address:08x}: "
                f"{actual:08x} != {expected:08x}"
            )

    far_depth = f32(0x8C0A7A78)
    if far_depth != 2097152.0:
        raise ValueError(f"Unexpected object far depth: {far_depth!r}")

    near_depth = f32(0x8C0A70A8)
    if near_depth != struct.unpack("<f", bytes.fromhex("0ad7233c"))[0]:
        raise ValueError(f"Unexpected object near plane: {near_depth!r}")

    return {
        "sha256": digest,
        "walker": {
            "entry": "8c0a78c0",
            "object_list": "8c2b2a04",
            "center_offset": 12,
            "radius_offset": 24,
            "visibility_test": "8c0a6620",
            "far_depth": far_depth,
            "transform_builder": "8c100392",
            "mesh_emitter": "8c0a6f40",
        },
        "material": {
            "identity_source": "*(object + 8)",
            "context_slot": "8c24387c + 48",
            "header_compiler": "8c041e60",
            "update_rule": "only when material identity changes",
            "distance_texture_switch": False,
        },
        "mesh_emitter": {
            "entry": "8c0a6f40",
            "source_record_bytes": 32,
            "output_vertex_bytes": 32,
            "projection_scale": "8c2747cc",
            "projection_helper": "8c0a6cc0",
            "near_plane": near_depth,
            "clip_interpolators": ["8c0a6d00", "8c0a6ec0"],
            "nontrivial_clip_masks": 6,
            "ordinary_vertex_command": "e0000000",
            "end_of_strip_command": "f0000000",
            "partial_triangles_rebuilt": True,
        },
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.image), indent=2))


if __name__ == "__main__":
    main()
