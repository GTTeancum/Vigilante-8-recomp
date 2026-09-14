"""Install the original V8 album alongside (without replacing) V8:2 music."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[2]


def install(source: Path, destination: Path):
    manifest = json.loads((ROOT / "tools/recompone-reference/RecompOne.Runtime/Cdrom/V8LooseManifest.json").read_text())
    tracks = [t["source"] for t in manifest["tracks"] if t["number"] > 1]
    # Check the complete input set before copying any of it.
    for relative in tracks:
        if not (source / relative).is_file():
            raise FileNotFoundError(source / relative)
    records = []
    for relative in tracks:
        src, dst = source / relative, destination / relative
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
        digest = hashlib.sha256(src.read_bytes()).hexdigest()
        assert digest == hashlib.sha256(dst.read_bytes()).hexdigest(), dst
        records.append({"file": relative, "sha256": digest})
    return records


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=ROOT / "PS1 game")
    parser.add_argument("--destination", type=Path, default=ROOT / "V8_2_LOOSE")
    args = parser.parse_args()
    print(json.dumps(install(args.source.resolve(), args.destination.resolve()), indent=2))
