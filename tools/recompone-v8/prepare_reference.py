#!/usr/bin/env python3
"""Prepare RecompOne maps/config from the checked-in Vigilante 8 analysis."""

from __future__ import annotations

import argparse
import json
import re
import shutil
from pathlib import Path


OVERLAY_BASE = "0x80100000"
CODE_LABEL_PATTERN = re.compile(r"(?:_OBJ_|^caseD_|^switchD_|^LAB_)")
SHELL_CARD_CALLBACKS = tuple(range(0x80110D00, 0x80110DA0, 0x14))
MAIN_INTERRUPT_TAILS = (0x80011CCC, 0x80011F0C)
MAIN_FUNCTION_EXTENTS = {
    # Ghidra stopped at the first computed primitive handler even though the
    # function's jump tables cover the renderer block through 0x8001D36C.
    0x8001BE5C: 0x8001D370,
}
MAIN_DYNAMIC_CALLBACK_EXTENTS = {
    # Twelve 0x24-byte wrappers are referenced only through a callback table,
    # so Ghidra did not promote them to functions. Each supplies a different
    # descriptor pointer to the common handler at 0x8002A350.
    address: address + 0x24
    for address in range(0x8002A3E8, 0x8002A598, 0x24)
}
# Object update callback referenced from runtime state rather than a direct JAL.
# Ghidra stopped the preceding function at this exact address.
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x8002E2BC] = 0x8002E604
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x800129AC] = 0x800129E8
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x80022CB0] = 0x80022CD0
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x80021E5C] = 0x80021F30
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x80022044] = 0x800220D4
# Object factory callback selected by LOAD.DLL for arena records. The address
# is stored in DAT_80065A34 and reached only through FUN_80021B80's JALR.
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x800222A8] = 0x80022320
MAIN_DYNAMIC_CALLBACK_EXTENTS[0x8002B98C] = 0x8002BC18
MAIN_DYNAMIC_CALLBACK_EXTENTS.update(
    {
        # Event dispatcher stored at object +0x64. The preceding callback
        # returns at 0x8003AB60; this routine owns the case table through its
        # epilogue at 0x8003B134, before the next callback at 0x8003B138.
        0x8003AB64: 0x8003B138,
        0x8003C61C: 0x8003CB64,
        0x8003CB64: 0x8003CD0C,
        0x8003CD0C: 0x8003CE24,
        # Indirect per-vehicle arena update routine immediately following
        # FUN_8003D1E8; its address is stored in runtime state, not JAL-called.
        0x8003D214: 0x8003D898,
        0x8003FA78: 0x8003FAC4,
        0x800402B8: 0x8004035C,
        0x8004035C: 0x80040378,
        # Oilfield steam puffs use the second small particle callback in the
        # 0x8004042C block, installed directly as object callback 0x80040470.
        0x80040470: 0x800404C4,
    }
)
MAIN_NATIVE_CALLBACK_STARTS = (
    # Indirect object callbacks recovered as standalone functions by the
    # native decompilation. These do not have direct JAL xrefs, so several
    # appear only as undefined gaps in Ghidra's function inventory.
    0x8002BDD0,
    0x8002BFB8,
    0x8002C210,
    0x8002E2BC,
    0x80030F34,
    0x80031634,
    0x80031AFC,
    0x80031BBC,
    0x80031FA0,
    0x800321C0,
    0x8003277C,
    0x80032AA4,
    0x80032C60,
    0x8003302C,
    0x80033290,
    0x800336FC,
    0x80033C74,
    0x8003403C,
    0x80034920,
    0x80034CEC,
    0x80034E70,
    0x80034EF8,
    0x8003502C,
    0x800352AC,
    0x8003565C,
    0x800359C0,
    # Deferred object callback reached by the sustained Oil Fields vehicle
    # exercise; the next known native callback begins at 0x800363E0.
    0x80035CF8,
    0x800363E0,
    # Deferred object callback stored at object +0x64. The next independently
    # installed native callback begins at 0x80036AD8.
    0x80036910,
    0x80036AD8,
    # Deferred object callback observed at object +0x64 during the corrected
    # Oil Fields physics smoke; the next native callback begins at 0x800372B0.
    0x80036D48,
    0x800372B0,
    0x800378D0,
    0x80037B94,
    # Deferred object callback stored at object +0x64. Its native body ends at
    # the next known callback, 0x800380C8.
    0x80037D34,
    0x800380C8,
    0x8003828C,
    0x80038324,
    0x8003846C,
    # Weapon-slot event dispatcher stored directly at object +0x64. The
    # preceding callback returns at 0x80038808 and this entry starts with its
    # own stack-frame prologue; its body ends before callback 0x80038A0C.
    0x8003880C,
    0x80038A0C,
    0x80038CF8,
    0x80038D18,
    # Weapon/event callback installed at object +0x64 by FUN_8003C61C.
    # It dispatches events 0-14 and ends immediately before FUN_80039274.
    0x80038FC0,
    0x80039274,
    0x8003935C,
    # Native code materializes this address and stores it in an object's
    # callback slot; its body ends at the next callback, 0x8003959C.
    0x80039458,
    0x8003959C,
    # Object event callback stored at +0x64 by the gameplay path. Its prologue
    # starts at 0x80039D14 and dispatches through the switch at 0x80039D58.
    0x80039D14,
    0x8003A084,
    # Deferred object callback between the known 0x8003A084 and 0x8003A9DC
    # callbacks, reached during the longer Oil Fields physics exercise.
    0x8003A56C,
    0x8003A9DC,
    0x8003B138,
    0x8003B1E0,
    # Object event callback stored at +0x64 by the gameplay path. Its native
    # prologue begins at 0x8003B3C8 and its epilogue ends immediately before
    # the already mapped callback at 0x8003B8D4.
    0x8003B3C8,
    0x8003B8D4,
    0x8003BDE0,
    # Deferred object callback reached after the deterministic braking phase;
    # Ghidra resumes with the next direct function at 0x8003C288.
    0x8003BEA8,
    0x8003C61C,
    0x8003CB64,
    0x8003E7B4,
    0x8003E80C,
    0x8003E868,
    0x8003E8A0,
    0x8003ED38,
    0x8003EE88,
    0x8003EFC8,
    0x8003F45C,
    0x8004007C,
    0x8004042C,
    0x800404C4,
    0x80040540,
    0x80040894,
    0x80040B38,
)
MAIN_INTERNAL_ENTRYPOINTS = (
    # Constant tail branches emitted from recovered helper/case functions.
    # These enter valid code inside larger Ghidra-owned extents.
    0x80016860,
    0x80016940,
    0x80019530,
    0x800197A4,
    0x8001A6A8,
    0x8001A704,
    0x8001A838,
    0x8001B670,
    # AIRGRAVE's exported event callback invokes this third object-lookup
    # wrapper. The overlay entry point was absent from Ghidra's function list,
    # so overlay JAL discovery could not see the otherwise ordinary call.
    0x80021830,
    0x80021DB0,
    0x80021E5C,
    0x8003D0D0,
    0x8003D988,
    0x8003DC10,
    0x8003DFB0,
    0x8003DFD8,
    # Collision/event code calls the second routine embedded in Ghidra's
    # overlapping 0x8003DFD8 extent. Its independent prologue begins at this
    # direct JAL target and runs through FUN_8003E254.
    0x8003DFFC,
    0x8003F4F0,
    0x8003F600,
    0x8003F684,
    0x800523E4,
    0x80053ECC,
)
SHELL_INTERNAL_EXTENTS = {
    # Player-count jump-table entries are callable functions in Ghidra, but
    # two of them branch back into the middle of the owning menu routine.
    0x8010281C: 0x80102BDC,
    # The options hub's computed page helpers tail back to the owning routine's
    # resource/setup restart and steady-state input loop. Generated case
    # helpers dispatch these labels as relocated callable continuations.
    0x8010C2FC: 0x8010C690,
    0x8010C480: 0x8010C690,
    # Computed main-menu case helpers tail back into this input/dispatch loop.
    0x8010CCC8: 0x8010D034,
}
LOAD_INTERNAL_EXTENTS = {
    # Jump-table case helpers tail back into the owning mesh routine here.
    0x80101364: 0x80101574,
}
SKIRESRT_INTERNAL_EXTENTS = {
    # GTE terrain clipping helper. Capstone/Ghidra do not recognize its first
    # COP2 instruction as a normal function prologue, but six relocated JALs
    # enter this exact address and the routine ends before the next helper.
    0x8010038C: 0x80100424,
    # Indirect terrain helper called through a relocated callback pointer.
    # The routine returns at 0x801005DC before FUN_801005E0 begins.
    0x80100424: 0x801005E0,
}
ARENA_DYNAMIC_CALLBACK_EXTENTS = {
    "CANYNLND": {
        # Small event callback stored at object +0x64; it starts immediately
        # after FUN_80100CBC's epilogue and ends at the image padding.
        0x80101274: 0x801012A0,
    },
    "HOOVRDAM": {
        # Same source event callback shape, placed after FUN_80101A98.
        0x80101B5C: 0x80101B88,
    },
    "WILDWEST": {
        # Per-object event callback stored at +0x64. It follows the prior
        # routine's epilogue and ends at FUN_801004CC's prologue.
        0x801004A0: 0x801004CC,
    },
}
OVERLAY_ENTRYPOINTS = {
    # Exported terrain callbacks recorded in each DLL header at file offset
    # 0x0C. Several have no direct JAL xref, so Ghidra omitted them.
    "AIRGRAVE": 0x801000EC,
    "CANYNLND": 0x8010007C,
    "CASNOCTY": 0x8010013C,
    "HOOVRDAM": 0x8010036C,
    "OILFIELD": 0x801000E4,
    "SANDFACT": 0x801000F4,
    "SCRTBASE": 0x80100200,
    "SKIRESRT": 0x8010016C,
    "VALLYFRM": 0x801002C4,
    "WILDWEST": 0x80100234,
}
def read_json(path: Path):
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def write_json(path: Path, value) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(value, stream, indent=2)
        stream.write("\n")


def extend_functions(output: Path, extents: dict[int, int]) -> int:
    function_map = read_json(output)
    for entry in function_map["functions"]:
        address = int(entry["address"], 16)
        if address in extents:
            entry["size"] = extents[address] - address
    write_json(output, function_map)
    return len(function_map["functions"])


def add_explicit_extents(output: Path, extents: dict[int, int]) -> int:
    function_map = read_json(output)
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    for address, end in extents.items():
        if address not in known:
            function_map["functions"].append(
                {
                    "address": f"0x{address:08X}",
                    "name": f"LAB_{address:08x}",
                    "size": end - address,
                }
            )
    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def add_explicit_starts(output: Path, addresses: tuple[int, ...]) -> int:
    function_map = read_json(output)
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    starts = sorted(known)
    for address in addresses:
        if address in known:
            continue
        end = next((start for start in starts if start > address), None)
        if end is None:
            raise ValueError(f"no following function after explicit start 0x{address:08X}")
        function_map["functions"].append(
            {
                "address": f"0x{address:08X}",
                "name": f"FUN_{address:08x}",
                "size": end - address,
            }
        )
        known.add(address)
        starts.append(address)
        starts.sort()
    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def add_bounded_explicit_starts(output: Path, addresses: tuple[int, ...]) -> int:
    """Add adjacent standalone routines omitted from the function inventory."""
    function_map = read_json(output)
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    boundaries = sorted(known | set(addresses))
    for address in addresses:
        if address in known:
            continue
        end = next((start for start in boundaries if start > address), None)
        if end is None:
            raise ValueError(f"no following function after explicit start 0x{address:08X}")
        function_map["functions"].append(
            {
                "address": f"0x{address:08X}",
                "name": f"LAB_{address:08x}",
                "size": end - address,
            }
        )
        known.add(address)
    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def convert_inventory(source: Path, output: Path, address_min: int, address_max: int) -> int:
    entries = read_json(source)
    functions = []
    for entry in entries:
        address = int(entry["address"], 16)
        size = int(entry.get("size", 0))
        if not (address_min <= address < address_max) or size <= 0:
            continue
        functions.append(
            {
                "address": f"0x{address:08X}",
                "name": entry["name"],
                "size": size,
            }
        )
    functions.sort(key=lambda item: int(item["address"], 16))
    write_json(output, {"functions": functions, "labels": []})
    return len(functions)


def add_symbol_branch_targets(inventory: Path, symbols: Path, output: Path) -> int:
    function_map = read_json(output)
    inventory_entries = sorted(
        read_json(inventory), key=lambda entry: int(entry["address"], 16)
    )
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    functions = [
        (int(entry["address"], 16), int(entry.get("size", 0)))
        for entry in inventory_entries
        if int(entry.get("size", 0)) > 0
    ]
    code_min = functions[0][0]
    code_max = max(address + size for address, size in functions)

    labels = {}
    with symbols.open("r", encoding="utf-8") as stream:
        for raw in stream:
            parts = raw.rstrip().split("\t", 2)
            if len(parts) != 3 or parts[1] != "Label":
                continue
            address = int(parts[0], 16)
            name = parts[2]
            if (
                code_min <= address < code_max
                and address not in known
                and CODE_LABEL_PATTERN.search(name)
            ):
                labels.setdefault(address, name)

    for address, name in labels.items():
        containing_end = next(
            (
                start + size
                for start, size in functions
                if start < address < start + size
            ),
            None,
        )
        if containing_end is not None:
            end = containing_end
        else:
            end = next(
                (start for start, _ in functions if start > address), code_max
            )
        size = end - address
        if size <= 0:
            continue
        function_map["functions"].append(
            {
                "address": f"0x{address:08X}",
                "name": name,
                "size": size,
            }
        )

    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def add_explicit_tail_targets(output: Path, addresses: tuple[int, ...]) -> int:
    function_map = read_json(output)
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    for index, address in enumerate(addresses):
        if address in known:
            continue
        end = addresses[index + 1] if index + 1 < len(addresses) else address + 0x14
        function_map["functions"].append(
            {"address": f"0x{address:08X}", "name": f"LAB_{address:08x}", "size": end - address}
        )
    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def add_external_direct_targets(inventory: Path, mips_dir: Path, output: Path) -> int:
    function_map = read_json(output)
    inventory_entries = sorted(
        read_json(inventory), key=lambda entry: int(entry["address"], 16)
    )
    source_functions = {
        int(entry["address"], 16): int(entry.get("size", 0))
        for entry in inventory_entries
        if int(entry.get("size", 0)) > 0
    }
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    starts = sorted(source_functions)
    code_min = starts[0]
    code_max = max(start + source_functions[start] for start in starts)
    target_pattern = re.compile(
        r"^([0-9a-fA-F]{8}):.*\b(?:b\w*|j)\s+[^#\r\n]*?0x([0-9a-fA-F]{8})\b"
    )
    recovered = set()

    for disasm in mips_dir.glob("*.s"):
        try:
            owner = int(disasm.stem, 16)
        except ValueError:
            continue
        owner_size = source_functions.get(owner)
        if owner_size is None:
            continue
        lines = disasm.read_text(encoding="utf-8", errors="replace").splitlines()
        for line in lines:
            match = target_pattern.search(line)
            if match is None:
                continue
            instruction = int(match.group(1), 16)
            target = int(match.group(2), 16)
            if not (code_min <= target < code_max):
                continue
            instruction_in_owner = owner <= instruction < owner + owner_size
            target_in_owner = owner <= target < owner + owner_size
            if instruction_in_owner and target_in_owner:
                continue
            if target not in known:
                recovered.add(target)

    for address in sorted(recovered):
        end = next((start for start in starts if start > address), code_max)
        size = end - address
        if size <= 0 or size > 0x400:
            continue
        function_map["functions"].append(
            {
                "address": f"0x{address:08X}",
                "name": f"LAB_{address:08x}",
                "size": size,
            }
        )

    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def add_overlay_main_targets(
    inventory: Path, overlay_analysis: Path, output: Path
) -> int:
    function_map = read_json(output)
    inventory_entries = sorted(
        read_json(inventory), key=lambda entry: int(entry["address"], 16)
    )
    starts = [int(entry["address"], 16) for entry in inventory_entries]
    sizes = {
        int(entry["address"], 16): int(entry.get("size", 0))
        for entry in inventory_entries
    }
    code_min = starts[0]
    code_max = max(start + sizes[start] for start in starts)
    known = {int(entry["address"], 16) for entry in function_map["functions"]}
    call_pattern = re.compile(r"\bjal\s+0x([0-9a-fA-F]{8})\b")
    targets = set()

    for disasm in overlay_analysis.glob("*/mips/*.s"):
        text = disasm.read_text(encoding="utf-8", errors="replace")
        for match in call_pattern.finditer(text):
            target = int(match.group(1), 16)
            if code_min <= target < code_max and target not in known:
                targets.add(target)

    for address in sorted(targets):
        end = next((start for start in starts if start > address), code_max)
        size = end - address
        if size <= 0 or size > 0x1000:
            continue
        function_map["functions"].append(
            {
                "address": f"0x{address:08X}",
                "name": f"FUN_{address:08x}",
                "size": size,
            }
        )

    function_map["functions"].sort(key=lambda item: int(item["address"], 16))
    write_json(output, function_map)
    return len(function_map["functions"])


def relative_posix(target: Path, start: Path) -> str:
    import os

    return Path(os.path.relpath(target, start)).as_posix()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cue", default="reference/assets/disc/Vigilante8.cue")
    parser.add_argument("--output", default="reference/generated")
    args = parser.parse_args()

    repo = Path(__file__).resolve().parents[2]
    output = (repo / args.output).resolve()
    maps = output / "function-maps"
    config_dir = output
    overlay_manifest = read_json(Path(__file__).with_name("overlay_paths.json"))

    counts = {}
    main_source = repo / "analysis" / "SLUS_005.10" / "functions.json"
    main_symbols = repo / "analysis" / "SLUS_005.10" / "symbols.txt"
    main_map = maps / "SLUS_005.10.json"
    counts["main"] = convert_inventory(main_source, main_map, 0x80000000, 0x80200000)
    counts["main"] = extend_functions(main_map, MAIN_FUNCTION_EXTENTS)
    counts["main"] = add_explicit_extents(main_map, MAIN_DYNAMIC_CALLBACK_EXTENTS)
    counts["main"] = add_symbol_branch_targets(main_source, main_symbols, main_map)
    counts["main"] = add_external_direct_targets(
        main_source, repo / "analysis" / "SLUS_005.10" / "mips", main_map
    )
    counts["main"] = add_overlay_main_targets(
        main_source, repo / "analysis" / "dll", main_map
    )
    counts["main"] = add_explicit_tail_targets(main_map, MAIN_INTERRUPT_TAILS)
    counts["main"] = add_bounded_explicit_starts(
        main_map, MAIN_NATIVE_CALLBACK_STARTS
    )
    counts["main"] = add_explicit_starts(main_map, MAIN_INTERNAL_ENTRYPOINTS)

    overlays = []
    for name, path_info in overlay_manifest.items():
        source = repo / "analysis" / "dll" / name / "functions.json"
        if not source.exists():
            raise FileNotFoundError(f"missing overlay inventory: {source}")
        function_map = maps / f"{name}.json"
        counts[name] = convert_inventory(source, function_map, 0x80100000, 0x80200000)
        counts[name] = add_symbol_branch_targets(
            source, repo / "analysis" / "dll" / name / "symbols.txt", function_map
        )
        if name in OVERLAY_ENTRYPOINTS:
            counts[name] = add_explicit_starts(
                function_map, (OVERLAY_ENTRYPOINTS[name],)
            )
        if name == "SHELL":
            counts[name] = add_explicit_extents(function_map, SHELL_INTERNAL_EXTENTS)
            counts[name] = add_explicit_tail_targets(function_map, SHELL_CARD_CALLBACKS)
        elif name == "LOAD":
            counts[name] = add_explicit_extents(function_map, LOAD_INTERNAL_EXTENTS)
        elif name == "SKIRESRT":
            counts[name] = add_explicit_extents(function_map, SKIRESRT_INTERNAL_EXTENTS)
        if name in ARENA_DYNAMIC_CALLBACK_EXTENTS:
            counts[name] = add_explicit_extents(
                function_map, ARENA_DYNAMIC_CALLBACK_EXTENTS[name]
            )
        overlays.append(
            {
                "name": name,
                "funcMap": relative_posix(function_map, config_dir),
                "base": OVERLAY_BASE,
                "file": path_info["file"],
                "v8Relocate": True,
            }
        )

    cue = (repo / args.cue).resolve()
    config = {
        "game": {
            "id": "SLUS-00510",
            "name": "Vigilante8PC",
            "title": "Vigilante 8 PC",
            "output": "recompiled",
        },
        "cue": relative_posix(cue, config_dir),
        "funcMap": relative_posix(main_map, config_dir),
        "debug": False,
        "linearSweep": False,
        "overlays": overlays,
        "stubs": [],
        "ignored": [],
        "patches": [
            {
                "overlay": "main",
                "address": "80045004",
                "target": "RecompOne.Runtime.Sdk.V8Compat.Alloc",
                "mode": "replace",
            },
            {
                "overlay": "main",
                "address": "80045088",
                "target": "RecompOne.Runtime.Sdk.V8Compat.Free",
                "mode": "replace",
            },
            {
                "overlay": "main",
                "address": "80015368",
                "target": "RecompOne.Runtime.Sdk.V8Compat.Fatal",
                "mode": "replace",
            },
            {
                "overlay": "main",
                "address": "800159B4",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceStreamOpen",
                "mode": "pre",
            },
            {
                "overlay": "LOAD",
                "address": "8010167C",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceLevelLoadEntry",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80021B80",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceLevelFactoryPre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80021B80",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceLevelFactoryPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "800165CC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.ServiceDisplayTransitionWait",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800128BC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.ServiceDrawSyncWait",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80025400",
                "target": "RecompOne.Runtime.Sdk.V8Compat.ValidateTerrainQuery",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8002F9BC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehiclePhysicsTick",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800173FC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehicleIntegratePre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800173FC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehicleIntegratePost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "8002D82C",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerCollisionPre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8002D82C",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerCollisionPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "80031300",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerChildSpawnPre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80031300",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerChildSpawnPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "80031454",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerWeaponHitPre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80031454",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerWeaponHitPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "8002C6FC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehicleDamagePre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8002C6FC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehicleDamagePost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "8002BD84",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehicleFullDestroy",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8002BE84",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceVehicleSoftKill",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8001392C",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceResultScreen",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80012A90",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePauseMenuPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "80044080",
                "target": "RecompOne.Runtime.Sdk.V8Compat.ApplyUserGameVolume",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80019A58",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceMenuText",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800120D4",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceMenuPad",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800120D4",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceMenuPadPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "800126F0",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceMenuPad",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80020890",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceObjectSchedule",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800205F8",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceObjectRetire",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8001F9CC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceAnimationObject",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8001F9CC",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceAnimationObjectPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "80024998",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceCollisionNeighborScanPre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80024998",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceCollisionNeighborScanPost",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "80024888",
                "target": "RecompOne.Runtime.Sdk.V8Compat.FixMissingDiagonalCollisionNeighbor",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "800378D0",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceChildCursorClearingAnimationPre",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800378D0",
                "target": "RecompOne.Runtime.Sdk.V8Compat.FixChildCursorClearingAnimationEnd",
                "mode": "post",
            },
            {
                "overlay": "CASNOCTY",
                "address": "80101A90",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceCursorClearingAnimationPre",
                "mode": "pre",
            },
            {
                "overlay": "CASNOCTY",
                "address": "80101A90",
                "target": "RecompOne.Runtime.Sdk.V8Compat.FixCursorClearingAnimationEnd",
                "mode": "post",
            },
            {
                "overlay": "HOOVRDAM",
                "address": "80100A18",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TraceCursorClearingAnimationPre",
                "mode": "pre",
            },
            {
                "overlay": "HOOVRDAM",
                "address": "80100A18",
                "target": "RecompOne.Runtime.Sdk.V8Compat.FixCursorClearingAnimationEnd",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "800202F4",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TrackObjectOwner",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "800202F4",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TracePlayerProjectileRegister",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "80040378",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TrackReturnedObjectOwner",
                "mode": "post",
            },
            {
                "overlay": "main",
                "address": "80015A20",
                "target": "RecompOne.Runtime.Sdk.LibCd.ReadV8FileBytes",
                "mode": "replace",
            },
            {
                "overlay": "main",
                "address": "80015BF0",
                "target": "RecompOne.Runtime.Sdk.LibCd.SeekV8File",
                "mode": "replace",
            },
            {
                "overlay": "main",
                "address": "8001570C",
                "target": "RecompOne.Runtime.Sdk.LibCd.BeginV8FileRead",
                "mode": "pre",
            },
            {
                "overlay": "main",
                "address": "8004F8EC",
                "target": "RecompOne.Runtime.Sdk.LibGpu.MoveImage",
                "mode": "replace",
            },
            {
                "overlay": "main",
                "address": "800156D4",
                "target": "RecompOne.Runtime.Sdk.LibCd.WaitForV8Sector",
                "mode": "replace",
            },
            {
                "overlay": "SHELL",
                "address": "80111260",
                "target": "RecompOne.Runtime.Sdk.V8Compat.WaitCardEvent",
                "mode": "replace",
            },
            {
                "overlay": "SHELL",
                "address": "801100F4",
                "target": "RecompOne.Runtime.Sdk.V8Compat.WaitCardOperation",
                "mode": "replace",
            },
            {
                "overlay": "SHELL",
                "address": "8010DE20",
                "target": "RecompOne.Runtime.Sdk.LibCdStream.StGetNext",
                "mode": "replace",
            },
            {
                "overlay": "SHELL",
                "address": "8010E500",
                "target": "RecompOne.Runtime.Sdk.V8Compat.TranslateOverlayDmaSource",
                "mode": "pre",
            },
            {
                "overlay": "SHELL",
                "address": "8010DD70",
                "target": "RecompOne.Runtime.Sdk.LibCdStream.StFreeRing",
                "mode": "replace",
            }
        ],
    }
    config_path = output / "v8.recompone.json"
    write_json(config_path, config)
    host_template = Path(__file__).with_name("reference-host")
    host_output = output / "recompiled"
    host_output.mkdir(parents=True, exist_ok=True)
    (host_output / "Vigilante8Reference.csproj").unlink(missing_ok=True)
    for name in ("Program.cs", "Vigilante8PC.csproj"):
        shutil.copyfile(host_template / name, host_output / name)

    print(f"Wrote {config_path}")
    print(f"Main functions: {counts['main']}")
    print(f"Overlay functions: {sum(v for k, v in counts.items() if k != 'main')}")
    if not cue.exists():
        print(f"Assets not present yet (expected cue: {cue})")
    else:
        print(f"Cue found: {cue}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
