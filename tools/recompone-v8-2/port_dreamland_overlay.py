#!/usr/bin/env python3
"""Generate the V8:2 Dreamland overlay from the working V8 translation."""

from __future__ import annotations

import argparse
from pathlib import Path
import re


REPO = Path(__file__).resolve().parents[2]
SOURCE = REPO / "reference" / "generated" / "recompiled" / "DREAMLND.cs"
OUTPUT = REPO / "tools" / "recompone-v8-2" / "LEVELS_N64_DREAMLND.cs"


ENGINE_CALLS = {
    "FUN_80012068": "func_80015B00",
    "FUN_80016aac": "func_80029F6C",
    "FUN_80016bd8": "func_8002A168",
    "FUN_80016da8": "func_8002A39C",
    "FUN_80016fa8": "func_8002A5EC",
    "FUN_800170c8": "func_8002ABC4",
    "FUN_80017160": "func_8002AC5C",
    "FUN_80017594": "func_8002B1FC",
    "FUN_8001ac08": "func_8002C05C",
    "FUN_8001ac44": "func_8002C17C",
    "FUN_8001add0": "func_8002C344",
    "FUN_8001af48": "func_8002C4B4",
    "FUN_8001b038": "func_8002C5F4",
    "FUN_8001b0c4": "func_8002C7D0",
    "FUN_8001bddc": "func_8001FEB8",
    "FUN_8001d470": "func_8002CBE8",
    "FUN_8001d564": "func_8002CCBC",
    "FUN_8001d5a0": "func_8002CD78",
    "FUN_8001d624": "func_8002CE50",
    "FUN_8001d68c": "func_8002CEAC",
    "FUN_8001d6e0": "func_8002CF44",
    "FUN_8001d708": "func_8002CF74",
    "FUN_8001d748": "func_8002CFBC",
    "FUN_8001d840": "func_8002D114",
    "FUN_8001dc1c": "func_8002D1DC",
    "FUN_8001f974": "func_8002FB70",
    "FUN_8001ffd4": "func_80030250",
    "FUN_800202f4": "func_800305FC",
    "FUN_8002036c": "func_8003066C",
    "FUN_800205f8": "func_800309A0",
    "FUN_80020620": "func_800309C8",
    "FUN_80020744": "func_80030B78",
    "FUN_80020778": "func_80030BA8",
    "FUN_800207c4": "func_80030BF0",
    "FUN_800207f8": "func_80030C20",
    "FUN_80020844": "func_80030C68",
    "FUN_80020890": "func_80030CB0",
    "FUN_80021808": "func_800318D0",
    "FUN_80022120": "func_800327CC",
    "FUN_80022320": "func_80032B90",
    "FUN_8002239c": "func_80032CF0",
    "FUN_80023d00": "func_80034B34",
    "FUN_80024718": "func_800359CC",
    "FUN_80025400": "func_8001B750",
    "FUN_80025648": "func_8001B998",
    "FUN_8002c3ac": "func_80039AF8",
    "FUN_8002c958": "func_8003A020",
    "FUN_80031294": "func_80041FEC",
    "FUN_80031454": "func_80042638",
    "FUN_8003d080": "func_8004AC1C",
    "FUN_8003e76c": "func_8004C9C8",
    "FUN_8003fc50": "func_8004DC94",
    "FUN_8003fc94": "func_8004DCD8",
    "FUN_8003fd24": "func_8004DE54",
    "FUN_8003fdcc": "func_8004E128",
    "FUN_8003fea8": "func_8004E414",
    "FUN_80042390": "func_800285E4",
    "FUN_80042698": "func_800512A8",
    "FUN_80042724": "func_80051334",
    "FUN_80042cdc": "func_800518DC",
    "FUN_80043408": "func_80024148",
    "FUN_800434f8": "func_80024238",
    "FUN_8004410c": "func_8001DD9C",
    "FUN_800441c8": "func_8001DE78",
    "FUN_80044574": "func_8001E2C8",
    "FUN_800446dc": "func_8001E478",
    "FUN_800449bc": "func_8001E7A8",
    "FUN_80044ac8": "func_8001E8B0",
    "FUN_80045088": "func_800520D8",
    "FUN_80045134": "func_80052188",
    "RotMatrixYXZ_gte": "func_8005A78C",
    "SquareRoot0": "func_800597BC",
    "VectorNormalS": "func_800598D8",
    "VectorNormalSS": "func_8005991C",
}


def generate(source: Path) -> str:
    text = source.read_text(encoding="utf-8")
    text = text.replace("Vigilante8PC", "Vigilante82PC")
    text = text.replace(
        "Dispatcher.NormalizeLinkedAddress(m, ",
        "RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, ",
    )
    text = text.replace(
        "Dispatcher.Call(c, m, ",
        "RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, ",
    )
    text = text.replace("DREAMLNDDispatchTable", "LEVELS_N64_DREAMLNDDispatchTable")
    text = text.replace('public string Name => "DREAMLND";',
                        'public string Name => "LEVELS_N64_DREAMLND";')
    # The old callback at 0x8003e7b4 reads a child callback from V8's +0x64.
    # V8:2's superficially similar 0x8004ca10 reads +0 instead, where a
    # converted no-callback node still carries its V8 flags word. Route those
    # events through the compatibility proxy. Keeping the value expressed as
    # base-minus-immediate preserves the translated branch structure.
    text = text.replace("0x80040000u", "0x8FFF196Cu")

    for old, new in ENGINE_CALLS.items():
        needle = f"Vigilante82PC.{old}(c, m);"
        bridge = (
            "CallObjectFactory" if old == "FUN_8001ac44" else
            "CallObjectModelFactory" if old == "FUN_8001add0" else
            "SkipStaticModelDetach" if old == "FUN_8001d564" else
            "CallModelNodeFinder" if old == "FUN_8001d624" else
            "CallObjectReallocator" if old == "FUN_80045134" else
            "CallLegacyCooldown" if old == "FUN_80022320" else
            "CallEngine"
        )
        replacement = (
            f"RecompOne.Runtime.Sdk.V82DreamlandCompat.{bridge}("
            f"c, m, Vigilante82PC.{new});"
        )
        count = text.count(needle)
        if count == 0:
            raise RuntimeError(f"Dreamland source no longer calls {old}")
        text = text.replace(needle, replacement)

    known = set(ENGINE_CALLS)
    remaining = {
        name for name in re.findall(r"Vigilante82PC\.(\w+)\(c, m\);", text)
        if name.startswith("FUN_") or name in {
            "RotMatrixYXZ_gte", "SquareRoot0", "VectorNormalS", "VectorNormalSS"
        }
    }
    if remaining:
        raise RuntimeError(f"unmapped Dreamland engine calls: {sorted(remaining)}")

    destroy_anchor = """    public static void DreamActorDestroy(CpuContext c, IMemory m)
    {
"""
    if text.count(destroy_anchor) != 1:
        raise RuntimeError("Dreamland actor-model preservation seam moved")
    text = text.replace(
        destroy_anchor,
        destroy_anchor +
        "        if (RecompOne.Runtime.Sdk.V82DreamlandCompat.PreserveActorModel(c, m)) return;\n",
        1,
    )

    text = re.sub(
        r"(\[0x801[0-9A-Fa-f]+u\] = )(Vigilante82PC\.\w+)(,)",
        r"\1RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(\2)\3",
        text,
    )
    return text


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=SOURCE)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    args = parser.parse_args()
    result = generate(args.source.resolve())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(result, encoding="utf-8")
    print(f"wrote {args.output.resolve()} ({len(result)} characters)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
