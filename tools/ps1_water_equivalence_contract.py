"""Verify the PS1 V8:2 water surface is the same authored grid algorithm.

The PC port executes the US PS1 binary.  This guard proves that its shared
surface generator has the same 32-cell grid cadence, terrain classification,
two wave periods, and two spatial wavelengths recovered from Dreamcast.  The
Enhanced renderer may therefore translate the function-owned packets' PVR
material/depth semantics without replacing the original geometry generator.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct


IMAGE_BASE = 0x80010000
FILE_HEADER = 0x800
IMAGE_SHA256 = "9ecfd0a7986cee816ef79284a6635eaadba435183362f8b796f667b8fb5a3eb6"


def verify(image: Path):
    data = image.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != IMAGE_SHA256:
        raise ValueError(f"Unexpected PS1 executable SHA-256: {digest}")

    def u32(address):
        return struct.unpack_from(
            "<I", data, FILE_HEADER + address - IMAGE_BASE
        )[0]

    words = {
        # Shared surface generator entry and 32-cell extent clamps.
        0x80016664: 0x27BDFF38,
        0x80016DE4: 0x28420020,
        0x80016E04: 0x28420020,
        # First wave: divide (tick << 12) by 240 and advance 873/4096 turn
        # per terrain cell (about 150 world units per wavelength).
        0x80017044: 0x3C028888,
        0x8001704C: 0x34428889,
        0x800171E8: 0x258C0369,
        # Second wave: divide (tick << 12) by 420 and advance 1456/4096 turn
        # per terrain cell (exactly 90 world units per wavelength).
        0x800171EC: 0x3C029C09,
        0x800171F4: 0x3442C09D,
        0x80017370: 0x258C05B0,
        # The two 32x32 scratch grids and shared sine table.
        0x8001709C: 0x3C02800B,
        0x800170A0: 0x24575570,
        0x800170A4: 0x3C02800B,
        0x800170A8: 0x24565D70,
        0x800170D8: 0x3C028006,
        0x800170DC: 0x24505C90,
    }
    for address, expected in words.items():
        actual = u32(address)
        if actual != expected:
            raise ValueError(
                f"Instruction mismatch at {address:08x}: "
                f"{actual:08x} != {expected:08x}"
            )

    return {
        "sha256": digest,
        "surface_entry": "80016664",
        "maximum_grid": [32, 32],
        "wave_period_ticks": [240, 420],
        "spatial_phase_steps_turn4096": [873, 1456],
        "wave_wavelength_world_units": [150, 90],
        "translation": (
            "retain function-owned PS1 grid geometry; apply recovered "
            "Dreamcast material, alpha, filtering, fog, and depth state"
        ),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.image), indent=2))


if __name__ == "__main__":
    main()
