"""Decode both PS1 quest formats from original data, without changing game files."""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from extract_psx_iso import PsxIso, parse_dir_records

V8_NAMES = ("chassey_blue", "slick_clyde", "sheila", "john_torque", "dave",
            "convoy", "loki", "houston_3", "boogie", "beezwax", "molo", "sid_burn", "y_the_alien")
V82_NAMES = ("Sheila", "Torque", "Trio", "Houston", "Convoy", "Cultsmen", "Dallas",
             "Nina", "Molo", "Clyde", "Obake", "Boogie", "Bob O", "Garbage Man",
             "Chase", "Chassey", "Padre", "Dusty")

def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]

def cstring(data, offset):
    assert 0 <= offset < len(data)
    return data[offset:data.index(0, offset)].decode("latin1")

def original_file(track, path, raw_stream=False):
    iso = PsxIso(track, 24)
    try:
        pvd = iso.read_sector(16)
        assert pvd[1:6] == b"CD001"
        data = iso.read_extent(u32(pvd, 158), u32(pvd, 166))
        for component in path.replace('\\', '/').split('/'):
            record = next((r for r in parse_dir_records(data) if r[0].upper() == component.upper()), None)
            if record is None: raise FileNotFoundError(path)
            data = iso.read_extent(record[1], record[2])
        if raw_stream:
            sectors = (record[2] + 2047) // 2048
            iso.fp.seek(record[1] * 2352)
            raw = iso.fp.read(sectors * 2352)
            return b''.join(raw[n*2352+16:(n+1)*2352] for n in range(sectors))
        return data
    finally:
        iso.close()

def original_quest(track):
    return original_file(track, 'QUEST.BIN')

def actors(data, offset, count):
    assert offset + count * 6 <= len(data)
    return [dict(zip(("type", "flags", "spawn", "aux"),
                     struct.unpack_from("<BBhH", data, offset + i * 6))) for i in range(count)]

def decode_original(data):
    count = u32(data, 0)
    assert count <= len(V8_NAMES)
    result = []
    for character, name in enumerate(V8_NAMES):
        missions = []
        if character < count:
            number, offset = struct.unpack_from("<II", data, 4 + character * 8)
            for mission in range(number):
                record = offset + mission * 16
                arena, variant, target_first, target_last, actor_count, actor_offset, text = struct.unpack_from("<BBHHHII", data, record)
                missions.append({"record_offset": record, "arena": arena,
                    "loading_card_variant": variant, "target_first": target_first, "target_last": target_last,
                    "actors": actors(data, actor_offset, actor_count), "briefing": cstring(data, text)})
        result.append({"source_type": character, "stable_id": "guest.v8." + name,
                       "eligible": bool(missions), "missions": missions})
    return result

def decode_sequel(data, executable):
    count = u32(data, 0)
    assert count == len(V82_NAMES)
    # Native loader 800324A0: three route pointers, then one pointer per type.
    teams = executable[0x8006B08C - 0x80010000 + 0x800:][:count]
    routes = []
    for team in range(3):
        offset = u32(data, 4 + team * 4)
        number = u32(data, offset)
        routes.append({"offset": offset, "count": number,
                       "records": [data[offset + 4 + i * 20:offset + 24 + i * 20].hex() for i in range(number)]})
    result = []
    for character, name in enumerate(V82_NAMES):
        offset = u32(data, 16 + character * 4)
        missions = []
        if offset:
            for mission in range(routes[teams[character]]["count"]):
                actor_count, actor_offset, text = struct.unpack_from("<III", data, offset + mission * 12)
                missions.append({"actors": actors(data, actor_offset, actor_count), "briefing": cstring(data, text)})
        result.append({"source_type": character, "name": name, "team": teams[character],
                       "eligible": bool(missions), "missions": missions})
    return routes, result

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "artifacts/original-quests-20260910")
    args = parser.parse_args()
    original = original_quest(ROOT / "BINCUE/Vigilante 8 (USA) (Track 01).bin")
    sequel = (ROOT / "V8_2_LOOSE/QUEST.BIN").read_bytes()
    routes, native = decode_sequel(sequel, (ROOT / "V8_2_LOOSE/SLUS_008.68").read_bytes())
    imported = decode_original(original)
    report = {"original_sha256": hashlib.sha256(original).hexdigest(),
              "sequel_sha256": hashlib.sha256(sequel).hexdigest(),
              "original": imported, "sequel": native, "native_routes": routes}
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "quest-source-audit.json").write_text(json.dumps(report, indent=2))
    (args.output / "v8-quest.bin").write_bytes(original)
    for game, entries in (("V8", imported), ("V8:2", native)):
        print(game, "eligible:", sum(e["eligible"] for e in entries), "missions:", sum(len(e["missions"]) for e in entries))
        print("Excluded:", [e.get("stable_id", e.get("name")) for e in entries if not e["eligible"]])

if __name__ == "__main__":
    main()
