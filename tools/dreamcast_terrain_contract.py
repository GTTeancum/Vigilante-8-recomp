"""Binary-anchored terrain subdivision decision reference; not a renderer.

Reads the supplied US Dreamcast executable and reproduces the decision at
8c1020a6..8c1020de for equal-depth, front-facing terrain patches. This deliberately
does not model height morphing, mixed-depth subdivision, or SH-4 rasterization.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct


IMAGE_BASE = 0x8C010000
IMAGE_SHA256 = "06f5a6fcdc9f8e0ef486a1ce1df7fcf61f5a756dbef7248139291ae520e3d3e7"


def read_contract(image):
    data = image.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != IMAGE_SHA256:
        raise ValueError(f"Unexpected Dreamcast executable SHA-256: {digest}")

    def word(address):
        return struct.unpack_from("<H", data, address - IMAGE_BASE)[0]

    def real(address):
        return struct.unpack_from("<f", data, address - IMAGE_BASE)[0]

    # Assert the relevant instructions against original binary bytes, rather
    # than accepting an earlier prose description as the reference.
    instructions = {
        0x8C101FC4: 0xE002,  # initial subdivision state = 2
        0x8C1020A2: 0xED04,  # initial patch width = 4
        0x8C1020AA: 0x0C12,  # r12 = GBR
        0x8C1020AC: 0x401D,  # state << 5
        0x8C1020AE: 0x7C14,  # table at GBR + 20
        0x8C1020B4: 0x8915,  # state zero -> textured material path
        0x8C1020D4: 0xF1C6,  # threshold at tuple + 8
        0x8C1020D6: 0xF015,  # minimum corner depth > threshold
        0x8C1020DA: 0xA0CE,  # far enough -> untextured submission
        0x8C1020DE: 0xA155,  # otherwise -> subdivision
        0x8C10238C: 0x4D01,  # halve patch width
        0x8C1026C6: 0x70FF,  # decrement subdivision state
        0x8C10270C: 0xACCB,  # evaluate first child with same decision
    }
    for address, expected in instructions.items():
        actual = word(address)
        if actual != expected:
            raise ValueError(f"Instruction mismatch at {address:08x}: {actual:04x}")

    modes = {}
    for mode, base in (("normal", 0x8C113DE8), ("alternate", 0x8C113E48)):
        modes[mode] = {
            str(state): {
                "far_end": real(base + 32 * state + 8),
                "span": real(base + 32 * state + 12),
                "inverse_span": real(base + 32 * state + 16),
                "morph_tail": real(base + 32 * state + 20),
            }
            for state in (1, 2)
        }
    return {"sha256": digest, "modes": modes,
            "projection_depth_floor": real(0x8C101E0C),
            "quad_rejection_depth": real(0x8C102380)}


def uniform_patch_decision(depth, tuples):
    """Trace a patch whose transformed corners all have the supplied depth."""
    state, width, count = 2, 4, 1
    while state and depth <= tuples[str(state)]["far_end"]:
        state -= 1
        width //= 2
        count *= 4
    return {"depth": depth, "leaf_width": width,
            "leaf_count": count, "textured": state == 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    args = parser.parse_args()
    contract = read_contract(args.image)
    contract["uniform_patch_examples"] = {
        mode: [uniform_patch_decision(depth, tuples)
               for depth in (10.0, 20.0, 26.0, 34.0, 35.0, 60.0)]
        for mode, tuples in contract["modes"].items()
    }
    print(json.dumps(contract, indent=2))


if __name__ == "__main__":
    main()
