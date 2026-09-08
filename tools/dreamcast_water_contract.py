"""Verify the recovered Dreamcast water-rendering contract against retail code.

This is a binary-anchored guard for the US Dreamcast executable. It does not
render water; it makes later translations fail loudly if their cited source
binary or the recovered material/geometry anchors differ.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct


IMAGE_BASE = 0x8C010000
IMAGE_SHA256 = "06f5a6fcdc9f8e0ef486a1ce1df7fcf61f5a756dbef7248139291ae520e3d3e7"

# Hash the complete contiguous retail ranges, including each function's
# branch-skipped literal pools.  Individual opcode anchors are useful for
# documenting semantics, but they are not evidence that the rest of a large
# routine was reconstructed.  These hashes make any unreviewed byte anywhere
# in the dispatcher or either water pass fail the contract.
FUNCTION_INTERVALS = {
    "untextured": (0x8C0D72A0, 0x2CA,
        "5c3d621487fd616b4dd356b645129320c52e1543dcd4fc7dd1b3a5f7100b48b9"),
    "textured_xwat": (0x8C0D75C0, 0xD38,
        "b542eb65bbb1af1c69d4ba66bec5e27f176e7499e65b7d8d496b58e466b14d77"),
    "dispatcher": (0x8C0D8320, 0x5C,
        "a0ceec021e47328778d4c1d02839ce5b6ed2cfed5403a170f86b08bca097ccff"),
}


def verify(image: Path):
    data = image.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != IMAGE_SHA256:
        raise ValueError(f"Unexpected Dreamcast executable SHA-256: {digest}")

    interval_hashes = {}
    for name, (address, size, expected) in FUNCTION_INTERVALS.items():
        offset = address - IMAGE_BASE
        actual = hashlib.sha256(data[offset : offset + size]).hexdigest()
        if actual != expected:
            raise ValueError(
                f"Complete {name} interval mismatch at {address:08x}: "
                f"{actual} != {expected}"
            )
        interval_hashes[name] = {
            "address": f"{address:08x}",
            "size": size,
            "sha256": actual,
        }

    def u16(address):
        return struct.unpack_from("<H", data, address - IMAGE_BASE)[0]

    def u32(address):
        return struct.unpack_from("<I", data, address - IMAGE_BASE)[0]

    def f32(address):
        return struct.unpack_from("<f", data, address - IMAGE_BASE)[0]

    instructions = {
        # Shared visibility dispatcher: camera/water height gate, orientation
        # gate, then the untextured and textured passes in that order.
        0x8C0D8348: 0xFC11,  # camera height - water plane
        0x8C0D834E: 0xF1C5,  # 60.0 > vertical distance
        0x8C0D8354: 0xF2C2,  # orientation component * height delta
        0x8C0D835E: 0xF108,  # load 0.4 orientation threshold
        0x8C0D8368: 0x410B,  # call untextured pass
        0x8C0D8370: 0x410B,  # call textured surface pass
        0x8C0D735A: 0xF25D,  # abs plane Y
        0x8C0D735E: 0xF15D,  # abs plane X
        0x8C0D7360: 0xF215,  # choose horizontal/vertical quad branch
        0x8C0D736A: 0x29C2,  # vertex command
        0x8C0D736C: 0x19B6,  # vertex ARGB
        0x8C0D7382: 0xF3F2,  # first plane scale * height delta
        0x8C0D73A0: 0xF213,  # divide projected Y by plane Y
        0x8C0D73AE: 0xF12A,  # first reciprocal-depth store
        0x8C0D73EE: 0xF309,  # load second plane scale
        0x8C0D7404: 0xF213,  # divide second projected Y by plane Y
        0x8C0D7412: 0xF12A,  # second reciprocal-depth store
        0x8C0D741A: 0x2122,  # final strip command
        0x8C0D7BC2: 0xE11F,  # grid maximum index = 31
        0x8C0D7BC4: 0x3217,  # cap first grid dimension
        0x8C0D7C16: 0xE11F,  # grid maximum index = 31
        0x8C0D7C18: 0x3217,  # cap second grid dimension
        0x8C0D7C62: 0x410D,  # visibility row index << 5
        0x8C0D7D66: 0xE007,  # displacement row shift = 7
        0x8C0D7D68: 0x490D,
        0x8C0D7E0A: 0xFC12,  # multiply the two first-wave sine terms
        0x8C0D7E10: 0xF1C2,  # first-wave amplitude
        0x8C0D7E68: 0x4228,  # tick << 16 for 420-tick phase
        0x8C0D7E6C: 0x321D,  # signed magic multiply
        0x8C0D7E74: 0x413C,  # arithmetic phase shift = 8
        0x8C0D7E84: 0xFE2E,  # row phase += min_z * phase_b
        0x8C0D7EEA: 0x4A0B,  # first second-wave sine
        0x8C0D7EF0: 0x4A0B,  # second second-wave sine
        0x8C0D7EF6: 0xFC12,  # multiply second-wave sine terms
        0x8C0D7EFA: 0xC749,  # second-wave amplitude address
        0x8C0D7EFC: 0xF008,  # second-wave amplitude load
        0x8C0D7EFE: 0xF1CE,  # fused second-wave amplitude/add
        0x8C0D7F00: 0xF81A,  # add second wave to displacement
        0x8C0D7F44: 0x400B,  # compile/reserve XWAT context
        0x8C0D7F80: 0x4B0B,  # copy PVR material header
        0x8C0D80D8: 0x2A02,  # textured vertex command
        0x8C0D811A: 0x4D0B,  # project first strip vertex
        0x8C0D813A: 0xF122,  # projected depth * 0.9
        0x8C0D814A: 0x2D12,  # textured vertex ARGB
        0x8C0D8154: 0xF15D,  # abs horizontal wave delta
        0x8C0D8156: 0xF1C2,  # horizontal delta * UV scale
        0x8C0D8168: 0xF15D,  # abs vertical wave delta
        0x8C0D816A: 0xF1C2,  # vertical delta * UV scale
        0x8C0D81B4: 0x4D0B,  # project alternating strip vertex
        0x8C0D81D0: 0x2A12,  # final strip command
        0x8C0D8222: 0x2AD2,  # nonterminal alternating command
        0x8C0D825A: 0xF15D,  # alternating horizontal wave delta
        0x8C0D826A: 0xF15D,  # alternating vertical wave delta
        0x8C0D82A2: 0x8D02,  # continue inclusive columns
        0x8C0D82BE: 0x8901,  # finish exact row-strip count
    }
    for address, expected in instructions.items():
        actual = u16(address)
        if actual != expected:
            raise ValueError(
                f"Instruction mismatch at {address:08x}: {actual:04x} != {expected:04x}"
            )

    words = {
        0x8C0D837C: 0x8C2B2804,
        0x8C0D8380: 0x8C28892C,
        0x8C0D838C: 0x8C0D72A0,
        0x8C0D8390: 0x8C0D75C0,
        0x8C0D756C: 0x8C2747C4,
        0x8C0D7570: 0x8C28EBF0,
        0x8C0D7574: 0x0000B000,
        0x8C0D7578: 0x8C244080,
        0x8C0D757C: 0x8C243900,
        0x8C0D7580: 0x8C0440E0,
        0x8C0D7584: 0xFF000038,
        0x8C0D7588: 0xFF00003C,
        0x8C0D758C: 0x03FFFFFF,
        0x8C0D7590: 0xE0000000,
        0x8C0D7594: 0x8C04182A,
        0x8C0D75AC: 0xF0000000,
        0x8C0D75B0: 0xFC000000,
        0x8C0D75B4: 0x0FFFFFFF,
        0x8C0D7DC0: 0x8C28892C,
        0x8C0D7DC4: 0x8C28D160,
        0x8C0D7DC8: 0x8C2BA940,
        0x8C0D7DD0: 0x8C1133EC,
        0x8C0D7DD4: 0x88888889,
        0x8C0D7DD8: 0x8C0D7260,
        0x8C0D7DDC: 0x8C2BAD40,
        0x8C0D8010: 0x8C1133EC,
        0x8C0D8014: 0x9C09C09D,
        0x8C0D8018: 0x8C0D7260,
        0x8C0D801C: 0x8C2BAD40,
        0x8C0D8024: 0x8C244080,
        0x8C0D8028: 0x8C2439C0,
        0x8C0D81E4: 0x8C2BAD40,
        0x8C0D81E8: 0xE0000000,
        0x8C0D81EC: 0x8C28892C,
        0x8C0D81F0: 0x8C2747CC,
        0x8C0D81F4: 0x8C0D70A0,
        0x8C0D81FC: 0x80FFFFFF,
        0x8C0D8200: 0xF0000000,
        0x8C0D8308: 0xE0000000,
        0x8C0D8310: 0x80FFFFFF,
    }
    for address, expected in words.items():
        actual = u32(address)
        if actual != expected:
            raise ValueError(
                f"Word mismatch at {address:08x}: {actual:08x} != {expected:08x}"
            )

    constants = {
        "vertical_visibility_band": (0x8C0D8384, 60.0),
        "orientation_gate": (0x8C0D8388, 0.4),
        "horizon_scale": (0x8C0D7598, 41.666668),
        "half": (0x8C0D759C, 0.5),
        "horizon_z": (0x8C0D75A0, 37.5),
        "near_scale": (0x8C0D75A4, 7.8124995),
        "near_z": (0x8C0D75A8, 7.0312495),
        "cell_size": (0x8C0D7DBC, 32.0),
        "wave_phase_a": (0x8C0D7DCC, 13981.014),
        "wave_amplitude_a": (0x8C0D8008, 0.15625),
        "wave_phase_b": (0x8C0D800C, 23301.69),
        "wave_amplitude_b": (0x8C0D8020, 0.03125),
        "uv_slope_scale": (0x8C0D81E0, 2.6666667),
        "projected_depth_scale": (0x8C0D81F8, 0.9),
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
        "complete_intervals": interval_hashes,
        "passes": {
            "untextured": {
                "entry": "8c0d72a0",
                "vertex_alpha": 0xB0,
                "strip_vertices": 4,
                "face_culling": "disabled",
                "rgb_source": "8c28ebf0..8c28ebf2",
                "viewport": {
                    "width": "8c2747c4",
                    "height": "8c2747c8",
                    "projection_scale": "8c2747cc",
                    "center_x": "8c2747d0",
                    "center_y": "8c2747d4",
                    "left": "8c2747d8",
                    "top": "8c2747dc",
                },
                "orientation_rule": "abs(plane_y) > abs(plane_x)",
                "horizontal_equations": {
                    "first_left_y": (
                        "center_y + (41.666668*t + 0.5*plane_x*width "
                        "- plane_z*projection)/plane_y"
                    ),
                    "first_right_y": (
                        "center_y + (41.666668*t - 0.5*plane_x*width "
                        "- plane_z*projection)/plane_y"
                    ),
                    "second_left_y": (
                        "center_y + (7.8124995*t + 0.5*plane_x*width "
                        "- plane_z*projection)/plane_y"
                    ),
                    "second_right_y": (
                        "center_y + (7.8124995*t - 0.5*plane_x*width "
                        "- plane_z*projection)/plane_y"
                    ),
                },
            },
            "textured_xwat": {
                "entry": "8c0d75c0",
                "vertex_argb": "80ffffff",
                "maximum_grid": [32, 32],
                "visibility_stride_bytes": 32,
                "displacement_stride_bytes": 128,
                "terrain_height_source": "8c28d160",
                "terrain_corner_visibility": (
                    "four adjacent classified corners set bit 0x80"
                ),
                "wave_period_ticks": [240, 420],
                "wave_wavelength_world_units": [150, 90],
            },
        },
        "dispatch": {
            "entry": "8c0d8320",
            "camera_matrix": "8c2b2804",
            "water_plane": "8c28892c",
            "vertical_visibility_band": decoded["vertical_visibility_band"],
            "orientation_gate": decoded["orientation_gate"],
            "call_order": ["8c0d72a0", "8c0d75c0"],
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
