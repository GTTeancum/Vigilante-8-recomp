"""Inventory the installed vehicle sources for exhaustive water-fit review."""
import hashlib
import argparse
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/blender_addons"))
from vigilante8_vehicle_tools import registry, stats


def inventory():
    loose = ROOT / "V8_2_LOOSE"
    source = loose / "SLUS_008.68"
    native = stats.StatsFile(source.read_bytes(), "V8_2")
    vehicles = [{"type": i, "identity": f"retail.v82.{i:02}",
        "source": str(source), "supports_transformations": True,
        "status": "pending attachment-fit verification"}
        for i in range(native.profile.record_count)]
    paths = sorted((loose / "mods").rglob("VEHICLES.V8R"))
    if len(paths) != 1:
        raise RuntimeError("Resolve active registry precedence before assigning custom type IDs")
    data = paths[0].read_bytes()
    game, entries = registry.parse_registry(data)
    if game != "V8_2":
        raise RuntimeError("Installed registry is not V8:2")
    excluded = []
    for i, entry in enumerate(entries):
        row = {"type": 64 + i, "identity": entry.stable_id,
            "display_name": entry.display_name, "source": str(paths[0]),
            "supports_transformations": entry.supports_transformations,
            "controller_class": entry.controller_class,
            "status": "pending attachment-fit verification"}
        # User-defined audit exclusion only, never a runtime behavior rule.
        if entry.stable_id == "guest.v8.y_the_alien":
            row["status"] = "excluded by user: Y the Alien"
            excluded.append(row)
        else:
            vehicles.append(row)
    return {"registry_sha256": hashlib.sha256(data).hexdigest(),
        "native_executable_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
        "count": len(vehicles), "vehicles": vehicles, "excluded": excluded}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = inventory()
    encoded = json.dumps(result, indent=2)
    if args.output:
        if args.output.exists():
            raise RuntimeError("Refusing to overwrite an existing roster review")
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
        print(f"{result['count']} targets; inventory saved to {args.output}")
    else:
        print(encoded)
