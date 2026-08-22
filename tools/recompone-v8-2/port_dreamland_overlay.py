#!/usr/bin/env python3
"""Generate the V8:2 Dreamland overlay from the working V8 translation."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import v82_arena_registry as arena_registry
import v82_native_water_conversion as native_water


REPO = Path(__file__).resolve().parents[2]
SOURCE = REPO / "reference" / "generated" / "recompiled" / "DREAMLND.cs"
OUTPUT = REPO / "tools" / "recompone-v8-2" / "LEVELS_N64_DREAMLND.cs"
ENTRY = REPO / "reference-v8-2" / "generated" / "recompiled" / "Entry.cs"


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


# V8 and V8:2 use the same logical movable-object fields but V8:2 inserted
# its callback/flags header and reorganized the model/link portion of the
# record.  This is the byte-for-byte ABI mapping recovered by comparing the
# retail V8 arena ports with their V8:2 counterparts.  The converter applies
# it to generated code; the runtime must never recognize an imported arena or
# proxy its memory accesses.
OBJECT_LAYOUT_RANGES = (
    (0x00, 0x08, 4),
    (0x08, 0x0A, 0),
    (0x0A, 0x34, 0x10),
    (0x34, 0x40, -0x28),
    (0x40, 0x64, 4),
    (0x64, 0x68, -0x64),
    (0x68, 0x9C, 0),
    (0x9C, 0xB4, 4),
    (0xB4, 0xD2, 0),
    (0xD2, 0xE8, 0x0C),
)


# Each exported callback saves A0 in one stable callee-saved register before
# doing any work.  Rewriting only that object base (and address aliases derived
# from it) avoids mistaking stack vectors, overlay tables, and model packets for
# game objects.  Secondary callback slots are handled independently below.
OBJECT_ROOTS = {
    "DreamContactActor": "S1",
    "DreamLnd": "S0",
    "cobblebridge": "S0",
    "DBridge": "S0",
    "Rainbow": "S2",
    "well": "S0",
    "castle": "S0",
    "Canon": "S2",
    "DreamProjectileSpawn": "S1",
    "DreamGoodProjectile": "S1",
    "mushroom_good": "S1",
    "DreamBadProjectile": "S1",
    "mushroom_bad": "S1",
    "DreamOrbitingActor": "S0",
    "DreamActorDestroy": "S0",
    "DreamActorBurst": "S2",
    "DreamActorPathStart": "S0",
    "DreamActorPathStep": "S0",
    "Knight": "S2",
    "Footman_Walk": "S2",
    "Chicken_Walk": "S0",
    "Piggy": "S0",
}

GPR_NAMES = (
    "ZERO", "AT", "V0", "V1", "A0", "A1", "A2", "A3",
    "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7",
    "S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7",
    "T8", "T9", "K0", "K1", "GP", "SP", "FP", "RA",
)
NATIVE_OBJECT_LINK_OFFSETS = {0x0C, 0x10, 0x14}
# Native engine routines whose return value is a newly allocated game object.
# This is deliberately an allow-list: ordinary engine calls return scalars,
# handles, or transient structure pointers that must not receive object ABI
# translation.
OBJECT_RETURNING_CALLS = {
    "func_8002C17C",  # allocate from database/model id
    "func_80052188",  # clone an existing object into a requested-size record
}
CALLER_SAVED_GPRS = {
    "V0", "V1", "A0", "A1", "A2", "A3",
    "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7", "T8", "T9",
}


def native_object_offset(source_offset: int) -> int:
    for start, end, delta in OBJECT_LAYOUT_RANGES:
        if start <= source_offset < end:
            return source_offset + delta
    raise ValueError(f"unmapped V8 object offset 0x{source_offset:X}")


def _rewrite_root_object_accesses(method: str, body: str) -> str:
    root = OBJECT_ROOTS.get(method)
    if root is None:
        return body

    # Convert every constant address derived directly from the stable object
    # root.  Assignments such as S1=S0+0x24 become native position aliases;
    # later +4/+8 vector accesses remain correct and therefore are not remapped.
    pattern = re.compile(rf"c\.{root} \+ 0x([0-9A-Fa-f]+)u")

    def replace_offset(match: re.Match[str]) -> str:
        source_offset = int(match.group(1), 16)
        if source_offset >= 0xE8:
            return match.group(0)
        return f"c.{root} + 0x{native_object_offset(source_offset):X}u"

    body = pattern.sub(replace_offset, body)

    # An unadorned root used as a memory address is V8 field +0 (flags), which
    # is V8:2 field +4.  Passing the object itself as an engine argument remains
    # unchanged.
    direct_patterns = (
        rf"(c\.(?:LoadWord|StoreWord)\([^\n]*?, m, )c\.{root}(\);)",
        rf"(m\.(?:ReadU8|ReadU16|ReadU32|WriteU8|WriteU16|WriteU32)\()c\.{root}([,)])",
    )
    for direct in direct_patterns:
        body = re.sub(direct, rf"\1(c.{root} + 0x4u)\2", body)
    return body


def _rewrite_object_pointer_aliases(method: str, body: str) -> str:
    """Propagate native object layout conversion through object pointers.

    Generated MIPS commonly loads an object's sibling/child/parent pointer
    into A0 or V1, or copies the result of the native object allocator into an
    S-register, and then accesses fields through that register.  Rewriting only
    the exported callback's stable S-register leaves those second-level object
    fields in the V8 layout.  Track both kinds of aliases and translate their
    direct field accesses until the register is redefined.  This is structure
    dataflow, independent of any arena callback.
    """
    root = OBJECT_ROOTS.get(method)
    if root is None:
        return body

    aliases = {root}
    converted: list[str] = []
    for line in body.splitlines(keepends=True):
        for alias in aliases - {root}:
            pattern = re.compile(rf"c\.{alias} \+ 0x([0-9A-Fa-f]+)u")

            def replace_alias_offset(match: re.Match[str]) -> str:
                source_offset = int(match.group(1), 16)
                if source_offset >= 0xE8:
                    return match.group(0)
                return f"c.{alias} + 0x{native_object_offset(source_offset):X}u"

            line = pattern.sub(replace_alias_offset, line)

            # An unadorned object alias used as a memory address is source
            # field +0 (flags), not the object pointer being passed to an
            # engine routine.  V8:2 moved flags to +4.
            direct_patterns = (
                rf"(c\.(?:LoadWord|StoreWord)\([^\n]*?, m, )c\.{alias}(\);)",
                rf"(m\.(?:ReadU8|ReadU16|ReadU32|WriteU8|WriteU16|WriteU32)\()c\.{alias}([,)])",
            )
            for direct in direct_patterns:
                line = re.sub(direct, rf"\1(c.{alias} + 0x4u)\2", line)
        converted.append(line)

        call = re.search(r"Vigilante82PC\.(\w+)\(c, m\);", line)
        if call:
            # A native call may overwrite every caller-saved register.  Seed
            # V0 again only for functions proven to return a game object.
            aliases.difference_update(CALLER_SAVED_GPRS - {root})
            if call.group(1) in OBJECT_RETURNING_CALLS:
                aliases.add("V0")
            continue

        load = re.search(
            r"c\.LoadWord\((\d+), m, \(c\.(\w+) \+ 0x([0-9A-Fa-f]+)u\)\);",
            line,
        )
        if load:
            destination_index = int(load.group(1))
            if destination_index < len(GPR_NAMES):
                destination = GPR_NAMES[destination_index]
                source = load.group(2)
                offset = int(load.group(3), 16)
                if source in aliases and offset in NATIVE_OBJECT_LINK_OFFSETS:
                    aliases.add(destination)
                elif destination != root:
                    aliases.discard(destination)
            continue

        copy = re.search(r"c\.CopyRegister\((\d+), (\d+)\);", line)
        if copy:
            destination_index = int(copy.group(1))
            source_index = int(copy.group(2))
            if destination_index < len(GPR_NAMES) and source_index < len(GPR_NAMES):
                destination = GPR_NAMES[destination_index]
                source = GPR_NAMES[source_index]
                if source in aliases:
                    aliases.add(destination)
                elif destination != root:
                    aliases.discard(destination)
            continue

        assignment = re.search(r"c\.(\w+)\s*=", line)
        if assignment and assignment.group(1) != root:
            aliases.discard(assignment.group(1))

    return "".join(converted)


def convert_object_layout(text: str) -> str:
    method_pattern = re.compile(
        r"(    public static void (\w+)\(CpuContext c, IMemory m\)\n"
        r"    \{)(.*?)(?=\n    \[System\.Runtime\.CompilerServices\.MethodImpl|\n\})",
        re.S,
    )

    def convert_method(match: re.Match[str]) -> str:
        method = match.group(2)
        body = _rewrite_root_object_accesses(method, match.group(3))
        return match.group(1) + _rewrite_object_pointer_aliases(method, body)

    text = method_pattern.sub(convert_method, text)

    # The model/resource owner can be reached through a child returned by the
    # native hierarchy walker, so its temporary register is not one of the
    # exported callback's stable roots.  +0x58 is exclusively the V8 resource
    # pointer field in this overlay and is +0x5C in V8:2.
    text = re.sub(r"c\.(\w+) \+ 0x58u", r"c.\1 + 0x5Cu", text)

    # Every +0x64 store in this overlay installs an object callback.  V8:2's
    # scheduler owns that callback at +0, regardless of which child/model
    # register currently names the object.
    text = re.sub(
        r"c\.StoreWord\((\d+), m, \(c\.(\w+) \+ 0x64u\)\);",
        r"c.StoreWord(\1, m, c.\2);",
        text,
    )
    return text


def convert_globals(text: str) -> str:
    # Fold the V8 global addresses into their native V8:2 locations.  Leaving
    # the preceding register-load instruction intact is harmless and makes the
    # transformation resilient to regenerated register allocation.
    text = re.sub(
        r"c\.LoadWord\((\d+), m, \(c\.\w+ \+ 0x59FCu\)\);",
        r"c.LoadWord(\1, m, 0x8006B814u);",
        text,
    )
    text = re.sub(
        r"c\.LoadWord\((\d+), m, \(c\.\w+ \+ 0x37E8u\)\);",
        r"c.LoadWord(\1, m, 0x8006B41Cu);",
        text,
    )
    text = re.sub(
        r"c\.LoadWord\((\d+), m, \(c\.\w+ \+ 0x5F8u\)\);",
        r"c.LoadWord(\1, m, 0x8006B41Cu);",
        text,
    )
    text = re.sub(r"c\.S3 = c\.\w+ \+ 0x5A18u;", "c.S3 = 0x8006B830u;", text)
    text = re.sub(r"c\.A0 = c\.A0 \+ 0x5A50u;", "c.A0 = 0x8006B868u;", text)
    text = re.sub(
        r"c\.StoreWord\((\d+), m, \(c\.\w+ \+ 0x5A10u\)\);",
        r"c.StoreWord(\1, m, 0x8006B828u);",
        text,
    )
    text = re.sub(r"c\.A1 = c\.V1 \+ 0x7B4u;", "c.A1 = 0x80065C90u;", text)
    return text


def convert_primary_event_abi(text: str, primary_method: str) -> str:
    """Insert V8:2's lifecycle event into an original-V8 arena callback.

    Retail V8:2 conversions preserve events 0..15, reserve event 16, and move
    original-V8 events 16 and 17 to 17 and 18.  Rewrite only the primary
    callback's table index; the callback continues to receive the native V8:2
    event number in A1, matching the retail converted overlays.
    """

    method_pattern = re.compile(
        rf"(    public static void {re.escape(primary_method)}"
        rf"\(CpuContext c, IMemory m\)\n    \{{)(.*?)"
        rf"(?=\n    \[System\.Runtime\.CompilerServices\.MethodImpl|\n\}})",
        re.S,
    )
    match = method_pattern.search(text)
    if match is None:
        raise RuntimeError(
            f"primary arena callback {primary_method!r} is missing"
        )
    body = match.group(2)
    bound = "        c.V0 = c.A1 < 0x00000012u ? 1u : 0u;\n"
    if body.count(bound) != 1:
        raise RuntimeError("primary arena event-table bound moved")
    no_op = re.search(
        r"if \(c\.V0 == 0u\) \{\n"
        r"            c\.StoreWord\(17, m, \(c\.SP \+ 0x24u\)\);\n"
        r"            goto (L[0-9A-Fa-f]+);",
        body,
    )
    if no_op is None:
        raise RuntimeError("primary arena no-op event target moved")
    table_index = "        c.V1 = c.A1 << 2;\n"
    if body.count(table_index) != 1:
        raise RuntimeError("primary arena event-table index moved")
    body = body.replace(
        bound,
        "        c.V0 = c.A1 < 0x00000013u ? 1u : 0u;\n",
        1,
    )
    body = body.replace(
        table_index,
        f"        if (c.A1 == 0x00000010u) goto {no_op.group(1)};\n"
        "        c.V1 = c.A1 < 0x00000011u ? c.A1 : c.A1 - 0x1u;\n"
        "        c.V1 = c.V1 << 2;\n",
        1,
    )
    return text[:match.start(2)] + body + text[match.end(2):]


NATIVE_HELPERS = r'''
    // Native V8:2 implementations emitted as part of the converted arena
    // overlay.  These are map behavior, not runtime compatibility hooks.
    public static void DreamlandVehicleHazardDamage(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        int amount = unchecked((int)c.A1);
        uint impulse = c.SP - 0x10u;
        m.WriteU32(impulse, 0u);
        m.WriteU32(impulse + 4u, 0u);
        m.WriteU32(impulse + 8u, 0u);
        c.A1 = unchecked((uint)-System.Math.Abs(amount));
        c.A2 = impulse;
        c.A3 = 1u;
        Vigilante82PC.func_8003A020(c, m);
        c.RA = savedRa;
    }

    static uint DreamlandNativeSoundId(uint sourceId) => sourceId switch
    {
        0x45u => 0x42u, 0x46u => 0x43u, 0x48u => 0x44u,
        0x49u => 0x45u, 0x4Au => 0x46u, 0x4Bu => 0x47u,
        0x4Cu => 0x48u, 0x4Eu => 0x49u, 0x4Fu => 0x4Au,
        0x50u => 0x4Bu, 0x5Fu => 0x4Cu,
        0x8Du => 0x30u, 0x8Eu => 0x27u, 0x97u => 0x35u, 0xA6u => 0x39u,
        _ => throw new System.InvalidOperationException(
            $"converted arena contains unmapped source SFX 0x{sourceId:X2}"),
    };

    public static void DreamlandPlaySound(
        CpuContext c, IMemory m, uint nativeFunction)
    {
        uint savedRa = c.RA;
        c.A1 = m.ReadU32(0x8006B41Cu);
        c.A2 = DreamlandNativeSoundId(c.A2);
        Dispatcher.Call(c, m, nativeFunction);
        c.RA = savedRa;
    }

    public static void DreamlandFindNearestObject(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint query = c.A0;
        int requestedId = unchecked((int)c.A1);
        int requestedFamily = -1;
        if (requestedId < 0)
            requestedFamily = unchecked((sbyte)m.ReadU8(
                0x8006AD74u + unchecked((uint)~requestedId)));

        uint exact = 0u, family = 0u;
        uint exactDistance = uint.MaxValue, familyDistance = uint.MaxValue;
        uint listRoot = m.ReadU32(0x8006B830u);
        for (uint node = m.ReadU32(listRoot); node != 0u; node = m.ReadU32(node))
        {
            uint obj = m.ReadU32(node + 8u);
            if (obj == 0u || m.ReadU8(obj + 8u) != 2u ||
                m.ReadU16(obj + 0x1Cu) == 0u)
                continue;
            c.A0 = query;
            c.A1 = obj + 0x4Cu;
            Vigilante82PC.func_80029F6C(c, m);
            uint distance = c.V0;
            int objectId = unchecked((short)m.ReadU16(obj + 0x1Au));
            if (objectId == requestedId && distance < exactDistance)
            {
                exact = obj;
                exactDistance = distance;
            }
            else if (objectId <= 0)
            {
                int objectFamily = unchecked((sbyte)m.ReadU8(
                    0x8006AD74u + unchecked((uint)~objectId)));
                if (objectFamily == requestedFamily && distance < familyDistance)
                {
                    family = obj;
                    familyDistance = distance;
                }
            }
        }
        c.V0 = exact != 0u ? exact : family;
        c.RA = savedRa;
    }

    public static void DreamlandContactEffect(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint position = c.A0;
        uint resource = c.A1;
        uint primary = c.A2 & 0xFFFFu;
        uint secondary = c.A3 & 0xFFFFu;
        uint sound = m.ReadU32(c.SP + 0x10u) & 0xFFFFu;
        uint interval = m.ReadU32(c.SP + 0x14u);
        uint lifetime = m.ReadU32(c.SP + 0x18u);

        c.A0 = resource; c.A1 = primary; c.A2 = 0x9Cu; c.A3 = 8u;
        Vigilante82PC.func_8002C17C(c, m);
        uint effect = c.V0;
        c.A0 = effect + 0x20u;
        Vigilante82PC.func_8002A39C(c, m);
        m.WriteU32(effect + 0x34u, m.ReadU32(position));
        m.WriteU32(effect + 0x38u, m.ReadU32(position + 4u));
        m.WriteU32(effect + 0x3Cu, m.ReadU32(position + 8u));
        m.WriteU32(effect, 0x80103500u);
        m.WriteU32(effect + 0x58u, 0x8000u);
        m.WriteU32(effect + 0x98u, resource);
        m.WriteU16(effect + 0x96u, (ushort)secondary);
        m.WriteU32(effect + 4u, m.ReadU32(effect + 4u) | 0xA4u);
        m.WriteU16(effect + 0x82u, (ushort)interval);
        c.A0 = effect; Vigilante82PC.func_800305FC(c, m);
        c.A0 = effect; c.A1 = lifetime; Vigilante82PC.func_80030CB0(c, m);
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0; c.A1 = m.ReadU32(0x8006B41Cu);
        c.A2 = DreamlandNativeSoundId(sound); c.A3 = position;
        Vigilante82PC.func_8001E580(c, m);
        c.V0 = effect;
        c.RA = savedRa;
    }

    public static void DreamlandContactEffectUpdate(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint effect = c.A0;
        if (c.A1 == 2u)
        {
            m.WriteU16(effect + 0x80u, 0xFFFF);
            c.V0 = 0u; c.RA = savedRa; return;
        }
        if (c.A1 == 5u)
        {
            m.WriteU32(effect + 0x64u, 0u);
            m.WriteU32(effect + 4u, m.ReadU32(effect + 4u) | 2u);
            c.V0 = uint.MaxValue; c.RA = savedRa; return;
        }
        if (c.A1 != 0u) { c.V0 = 0u; c.RA = savedRa; return; }

        ushort counter = unchecked((ushort)(m.ReadU16(effect + 0x80u) - 1));
        m.WriteU16(effect + 0x80u, counter);
        if (counter == 0xFFFF)
        {
            c.A0 = m.ReadU32(effect + 0x98u);
            c.A1 = m.ReadU16(effect + 0x96u);
            c.A2 = 0x80u; c.A3 = 8u;
            Vigilante82PC.func_8002C17C(c, m);
            uint particle = c.V0;
            c.A0 = particle + 0x20u;
            Vigilante82PC.func_8002A39C(c, m);
            int radius = unchecked((int)m.ReadU32(effect + 0x58u));
            Vigilante82PC.func_8002AC5C(c, m);
            m.WriteU32(particle + 0x34u,
                unchecked((uint)(((unchecked((int)c.V0) * 2L * radius) >> 15) - radius)));
            m.WriteU32(particle + 0x38u, 0u);
            Vigilante82PC.func_8002AC5C(c, m);
            m.WriteU32(particle + 0x3Cu,
                unchecked((uint)(((unchecked((int)c.V0) * 2L * radius) >> 15) - radius)));
            m.WriteU32(particle, 0x8004CA68u);
            c.A0 = effect; c.A1 = particle;
            Vigilante82PC.func_8002CC48(c, m);
            m.WriteU16(effect + 0x80u, m.ReadU16(effect + 0x82u));
        }
        if (m.ReadU32(effect + 0x10u) == 0u)
        {
            c.A0 = effect;
            Vigilante82PC.func_800309A0(c, m);
            c.V0 = uint.MaxValue;
        }
        else
        {
            c.V0 = 0u;
        }
        c.RA = savedRa;
    }

    public static void DreamlandLegacyModelEvent(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint eventCode = c.A1;
        uint actor = c.A2;
        Vigilante82PC.func_8002CD78(c, m);
        uint child = c.V0;
        uint callback = child == 0u ? 0u : m.ReadU32(child);
        if (callback >= 0x80000000u)
        {
            c.A0 = child;
            c.A1 = eventCode;
            c.A2 = actor;
            Dispatcher.Call(c, m, callback);
        }
        else
        {
            c.V0 = 0u;
        }
        c.RA = savedRa;
    }
'''


def inject_native_water_initializer(
    text: str,
    init_argument: int,
) -> str:
    """Initialize V8:2 global water on the converted arena's event-1 path."""

    # Event 1 is the second jump-table entry. L8010067C is event 0 (the
    # per-object update path), so setup there reinitializes global water for
    # every Dreamland object on every tick. L80100984 is the arena's one-time
    # initialization path, matching the lifecycle used by retail V8:2 arenas.
    anchor = "        L80100984: ;\n"
    if text.count(anchor) != 1:
        raise RuntimeError("Dreamland event-1 initializer anchor moved")
    initializer = (
        "        // Native V8:2 global-water setup translated from the "
        "authored V8 plane.\n"
        "        c.StoreWord(6, m, (c.SP + 0x34u));\n"
        f"        c.A0 = 0x{init_argument:08X}u;\n"
        "        c.A1 = 0x7FFF0000u;\n"
        "        RecompOne.Runtime.Sdk.V82NativeWaterDiagnostics.Initialize("
        "c, m, c.A0, c.A1);\n"
        "        Vigilante82PC.func_80017F34(c, m);\n"
        "        c.LoadWord(6, m, (c.SP + 0x34u));\n"
        "        c.CopyRegister(4, 16);\n"
    )
    return text.replace(anchor, anchor + initializer, 1)


def generate(
    source: Path,
    primary_method: str = "DreamLnd",
    native_water_init_argument: int = 0x0002A400,
) -> str:
    text = source.read_text(encoding="utf-8")
    text = text.replace("Vigilante8PC", "Vigilante82PC")
    text = text.replace("DREAMLNDDispatchTable", "LEVELS_N64_DREAMLNDDispatchTable")
    text = text.replace('public string Name => "DREAMLND";',
                        'public string Name => "LEVELS_N64_DREAMLND";')
    # Preserve the source's base-minus-immediate expression while relocating
    # the linked callback into this arena's own dispatch table.
    text = text.replace("0x80040000u", "0x80104D8Cu")
    text = convert_primary_event_abi(text, primary_method)

    host_calls = {
        "0x8FFF0100u": "Vigilante82PC.DreamlandVehicleHazardDamage(c, m);",
        # N64 draws the authored water once per frame.  Its V8:2 equivalent is
        # the same native global-water renderer used by Bayou.
        "0x8FFF0104u": (
            "RecompOne.Runtime.Sdk.V82NativeWaterDiagnostics.Frame(c, m);\n"
            "        Vigilante82PC.func_80017EB8(c, m);"
        ),
        "0x8FFF0108u": "Vigilante82PC.DreamlandContactEffect(c, m);",
        "0x8FFF010Cu": "Vigilante82PC.DreamlandFindNearestObject(c, m);",
        "0x8FFF0110u": (
            "Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E580u);"
        ),
        "0x8FFF0114u": (
            "Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);"
        ),
        "0x8FFF0118u": (
            "Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E098u);"
        ),
    }
    for address, replacement in host_calls.items():
        needle = f"Dispatcher.Call(c, m, {address});"
        count = text.count(needle)
        if count == 0:
            raise RuntimeError(f"Dreamland source no longer calls host service {address}")
        text = text.replace(needle, replacement)

    for old, new in ENGINE_CALLS.items():
        needle = f"Vigilante82PC.{old}(c, m);"
        replacement = f"Vigilante82PC.{new}(c, m);"
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

    text = convert_globals(convert_object_layout(text))
    text = inject_native_water_initializer(text, native_water_init_argument)
    class_end = "\n}\n\npublic sealed class LEVELS_N64_DREAMLNDDispatchTable"
    if text.count(class_end) != 1:
        raise RuntimeError("Dreamland generated class boundary moved")
    text = text.replace(
        class_end,
        "\n" + NATIVE_HELPERS + class_end,
        1,
    )
    dispatch_anchor = (
        "            [0x80102FE0u] = Vigilante82PC.Piggy,\n"
    )
    if text.count(dispatch_anchor) != 1:
        raise RuntimeError("Dreamland dispatch table tail moved")
    text = text.replace(
        dispatch_anchor,
        dispatch_anchor +
        "            [0x80103500u] = Vigilante82PC.DreamlandContactEffectUpdate,\n"
        "            [0x80103540u] = Vigilante82PC.DreamlandLegacyModelEvent,\n",
        1,
    )
    if "V82DreamlandCompat" in text or "0x8FFF01" in text:
        raise RuntimeError("Dreamland conversion retained a runtime compatibility seam")
    return text


def patch_entry(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    legacy_host_registration = (
        '        RecompOne.Runtime.Sdk.V82DreamlandCompat.'
        'RegisterHostFunctions();\n'
    )
    migrated = legacy_host_registration in text
    text = text.replace(legacy_host_registration, "")
    registration = (
        '        Dispatcher.Register("LEVELS_N64_DREAMLND", '
        'new LEVELS_N64_DREAMLNDDispatchTable());\n'
    )
    if text.count(registration) == 1:
        if migrated:
            path.write_text(text, encoding="utf-8")
        return migrated
    if "LEVELS_N64_DREAMLNDDispatchTable" in text:
        raise RuntimeError("Dreamland Entry registration is incomplete")

    anchor = (
        '        Dispatcher.Register("LEVELS_V8_WILDWEST", '
        'new LEVELS_V8_WILDWESTDispatchTable());\n'
    )
    if text.count(anchor) != 1:
        raise RuntimeError("Dreamland Entry registration anchor moved")
    path.write_text(text.replace(anchor, anchor + registration), encoding="utf-8")
    return True


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=SOURCE)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--entry", type=Path, default=ENTRY)
    parser.add_argument(
        "--arena-exp",
        type=Path,
        default=REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--arena-dll",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "files" / "LEVELS" / "N64" / "DREAMLND.DLL"
        ),
    )
    args = parser.parse_args()
    primary_method = arena_registry.primary_export_name(
        args.arena_dll.resolve().read_bytes()
    )
    water_source = native_water.find_native_water_source(
        native_water.iff.parse(args.arena_exp.resolve().read_bytes())
    )
    init_argument = native_water.native_water_init_argument(water_source.plane_y)
    result = generate(args.source.resolve(), primary_method, init_argument)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(result, encoding="utf-8")
    print(
        f"wrote {args.output.resolve()} ({len(result)} characters, "
        f"primary-export={primary_method}, V8:2-event-ABI=1, "
        f"native-water=1, water-init=0x{init_argument:08X})"
    )
    changed = patch_entry(args.entry.resolve())
    print(
        f"{'patched' if changed else 'validated'} "
        f"Dreamland registration in {args.entry.resolve()}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
