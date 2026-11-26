"""Original V8 driver-to-form order and ground-engine sound provenance."""
from __future__ import annotations

import base64

from . import iff, registry


VEHICLES = (
    (0, "guest.v8.chassey_blue", "Chassey Blue", "'67 Rattler"),
    (1, "guest.v8.slick_clyde", "Slick Clyde", "'70 Clydesdale"),
    (2, "guest.v8.sheila", "Sheila", "'74 Strider"),
    (3, "guest.v8.john_torque", "John Torque", "'69 Jefferson"),
    (4, "guest.v8.dave", "Dave", "'70 Van"),
    (5, "guest.v8.convoy", "Convoy", "'72 Moth Truck"),
    (6, "guest.v8.loki", "Loki", "'73 Glenn 4x4"),
    (7, "guest.v8.houston_3", "Houston 3", "'75 Palamino"),
    (8, "guest.v8.boogie", "Boogie", "'76 Leprechaun"),
    (9, "guest.v8.beezwax", "Beezwax", "'70 Stag Pickup"),
    (10, "guest.v8.molo", "Molo", "'66 School Bus"),
    (11, "guest.v8.sid_burn", "Sid Burn", "'69 Manta"),
    (12, "guest.v8.y_the_alien", '"Y" the Alien', "'64 Luxo Saucer"),
)


def original_engine_sounds(arms: bytes) -> dict[str, dict[str, object]]:
    # US: HUD\ARMS.EXP is loaded per driver by 8002A598 (Rev 1:
    # 8002B628). 8002C59C (Rev 1: 8002D62C) starts sample 0 from that
    # bank, not sample 0 of the COMMON body/weapon bank.
    forms = tuple(iff.parse(arms).forms(b"XOBF"))
    if len(forms) > len(VEHICLES):
        raise ValueError("expected the original HUD/ARMS.EXP, not a vehicle archive")
    result = {}
    for source_index, stable_id, _, _ in VEHICLES:
        sounds = registry._decode_sounds(forms[source_index]) if source_index < len(forms) else ()
        if len(sounds) != 1:
            raise ValueError(f"{stable_id} must have exactly one HUD/ARMS engine sample")
        engine = sounds[0]
        blocks = base64.b64decode(engine["adpcm_base64"], validate=True)
        flags = blocks[1::16]
        if not engine["pitch"] or not flags or not any(f & 4 for f in flags) or flags[-1] & 3 != 3:
            raise ValueError(f"{stable_id} HUD/ARMS sample is not a playable loop")
        result[stable_id] = engine
    return result
