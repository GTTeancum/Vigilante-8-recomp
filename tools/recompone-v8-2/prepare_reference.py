#!/usr/bin/env python3
"""Prepare the first RecompOne configuration for Vigilante 8: 2nd Offense.

The bootstrap configuration deliberately relies on RecompOne's call discovery
and linear sweep.  Ghidra-derived function maps can replace that discovery as
the sequel analysis matures, without changing the generated-project layout.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_CUE = (
    REPO
    / "V8_2_BINCUE"
    / "Vigilante 8 - 2nd Offensive [U] [SLUS-00868].cue"
)
DEFAULT_DISC = REPO / "V8_2_WORK" / "disc"
DEFAULT_OUTPUT = REPO / "reference-v8-2" / "generated"

# Original-binary callback or call targets proven at runtime but omitted by the
# bootstrap scanner. Keep this list address-only until Ghidra supplies names.
EXTRA_MAIN_FUNCTIONS = [
    "0x8004282C",
    "0x80014FD8",
    "0x80019EB8",
    "0x80019EF0",
    "0x80019F00",
    "0x80019F10",
    "0x80019F8C",
    "0x80019FAC",
    "0x80019FC0",
    "0x80019FD0",
    "0x80019FE4",
    "0x80019ABC",
    "0x80019AF0",
    "0x80019B0C",
    "0x8001A00C",
    "0x8001A014",
    "0x8001A064",
    "0x8001A078",
    "0x8001A084",
    "0x8001A0A0",
    "0x8001A0A8",
    "0x8001A0C8",
    "0x8001A0E0",
    "0x8001F76C",
    "0x8001F6F8",
    "0x8001F7DC",
    "0x8001F84C",
    "0x8001F8A4",
    "0x8001F924",
    "0x8001F9A8",
    "0x8001FA54",
    "0x8001FA8C",
    "0x8001FADC",
    "0x8001FB2C",
    "0x8001FB40",
    "0x8001FBA4",
    "0x8001E9B8",
    "0x8001EA0C",
    "0x8001EAA0",
    "0x8001EAAC",
    "0x8001EAB8",
    "0x8001EACC",
    "0x8001EAD0",
    "0x8001EAF8",
    "0x8001EB08",
    "0x80020AD4",
    "0x80020B04",
    "0x80020CB8",
    "0x80020CD4",
    "0x80020D00",
    "0x80020D50",
    "0x80021FA8",
    "0x8002215C",
    "0x80022164",
    "0x800221CC",
    "0x800221DC",
    "0x8002224C",
    "0x80022280",
    "0x800222E4",
    "0x80022330",
    "0x800223D4",
    "0x800223F0",
    "0x8002246C",
    "0x800224A0",
    "0x800224E8",
    "0x80022504",
    "0x80022580",
    "0x800225AC",
    "0x8002260C",
    "0x80022640",
    "0x80022674",
    "0x8002271C",
    "0x800227AC",
    "0x80022828",
    "0x80022870",
    "0x80022910",
    "0x800229A0",
    "0x80022A1C",
    "0x80022A4C",
    "0x80022B04",
    "0x80022B44",
    "0x80022B7C",
    "0x80022C54",
    "0x80022DA8",
    "0x80022E2C",
    "0x80022E78",
    "0x80022FBC",
    "0x8002313C",
    # Remaining direct targets in the secondary geometry dispatcher table at
    # 0x80021F04, including paths reached by terrain events and weapon effects.
    "0x8002356C",
    "0x80023658",
    "0x80023758",
    "0x80023880",
    # Full 16-way geometry primitive dispatcher table at 0x80021E8C.
    # Complex weapon effects use entries that normal arena rendering does not.
    "0x800239BC",
    "0x80023A70",
    # Shared projected-vertex continuation tail-called by the 0x23B3C,
    # 0x23C2C and 0x23D34 primitive handlers.
    "0x80023AD0",
    "0x80023B3C",
    "0x80023C2C",
    "0x80023D34",
    "0x800564F0",
    "0x800565F0",
    "0x8005663C",
    "0x800566BC",
    "0x8005673C",
    # DrawPrim/DMA ordering-table helper continuation reached by Pacific
    # Harbor's first arena callback through func_800598D8.
    "0x80059928",
    "0x8005F12C",
    "0x8005F158",
    "0x8005F188",
    "0x8005F198",
    "0x8005F1A0",
    "0x8005F1BC",
    "0x8005F1C4",
    "0x8005F1CC",
    "0x8005F254",
    "0x8005F25C",
    "0x8005F2B8",
    "0x8005F2C0",
    "0x8005F2C8",
    "0x8005F2D0",
    "0x8005F2EC",
    "0x8005F2F0",
    "0x8005F334",
    "0x8005F370",
    "0x8005F374",
    "0x8005F3A8",
    "0x8005F3B8",
    "0x8005F418",
    "0x8005F41C",
    "0x8005F450",
    "0x8005F45C",
    "0x8005F70C",
    "0x8005F720",
    # Internal continuations selected by the retail sprintf formatter table at
    # 0x80011458. RecompOne must retain every target because SHELL uses %c and
    # numeric formats while constructing the character-selection labels.
    "0x8005FAFC",
    "0x8005FD5C",
    "0x8005FD68",
    "0x8005FD74",
    "0x8005FDA0",
    "0x8005FDF8",
    "0x8005FF18",
    "0x80060018",
    "0x8006002C",
    "0x80060038",
    "0x80060144",
    "0x80060164",
    "0x800601F0",
    "0x80060224",
]

EXTRA_OVERLAY_FUNCTIONS = {
    # Bayou schedules this internal arena-event continuation directly as an
    # object callback after its opening simulation frames.
    "LEVELS_BAYOU": [
        "0x801001A0",
    ],
    # Steel Mill installs this event handler into spawned map objects.
    "LEVELS_STEELMIL": [
        "0x80100370",
    ],
    # Alaska/Oil Fields damage event handler stored directly in map objects.
    "LEVELS_OILFIELD": [
        "0x80100190",
        "0x80100348",
    ],
    # Secret Base registry predicate reached by its scripted event tree.
    "LEVELS_V8_SCRTBASE": [
        "0x801001D0",
    ],
    # SHELL's hand-written VLC/MDEC decoder uses tail jumps between internal
    # continuations. These are runtime-proven dispatch targets, including when
    # the shell is relocated by the retail loader.
    "SHELL_SHELL": [
        "0x80110E20",
        "0x80110EB0",
        "0x8011107C",
        "0x8011117C",
        "0x80111198",
        "0x801111C4",
    ],
    # LOAD's hand-written VLC decoder likewise tail-jumps among internal
    # continuations that do not look like ordinary ABI function entries.
    "SHELL_LOAD": [
        "0x80109810",
        "0x801098A0",
        "0x80109A6C",
        "0x80109B6C",
        "0x80109B88",
        "0x80109BB4",
    ],
}

PROVEN_PATCHES = [
    # Retail PsyQ SDK routines identified by instruction-exact comparison
    # against the symbolized Vigilante 8 executable.
    {
        "overlay": "main",
        "address": "80054C4C",
        "target": "RecompOne.Runtime.Sdk.LibEtc.VSync",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005BF88",
        "target": "RecompOne.Runtime.Sdk.LibGpu.DrawSync",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005C57C",
        "target": "RecompOne.Runtime.Sdk.LibGpu.DrawOTag",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005C2F4",
        "target": "RecompOne.Runtime.Sdk.LibGpu.MoveImage",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005C5EC",
        "target": "RecompOne.Runtime.Sdk.LibGpu.PutDrawEnv",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005C7B8",
        "target": "RecompOne.Runtime.Sdk.LibGpu.PutDispEnv",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80062B4C",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadInitDirect",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8006066C",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadChkVsync",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8006068C",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadStartCom",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800606AC",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadStopCom",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80060718",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadGetState",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800606CC",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadChkMtap",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800607E4",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadInfoMode",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800608DC",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadInfoAct",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800609B0",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadInfoComb",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80060A58",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadSetActAlign",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80060A90",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadSetMainMode",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80060AD8",
        "target": "RecompOne.Runtime.Sdk.LibPad.PadSetAct",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80052060",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PcMalloc",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800520D8",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PcFree",
        "mode": "replace",
    },
    {
        # The retail shape streams support record kinds 0, 1 and 2. A damaged
        # or legacy arena record otherwise loops forever in func_8002E998.
        "overlay": "main",
        "address": "8002E998",
        "target": "RecompOne.Runtime.Sdk.V82Compat.SafeShapeCollision",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80052188",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PcRealloc",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80052020",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExtendHeapPost",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80020A80",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TrackVramAllocationPre",
        "mode": "pre",
    },
    {
        # The Arcade result builder uses the retail formatter and strlen
        # helpers.  Keep their behavior unchanged, but retain the native
        # caller and arguments so a corrupt result pointer is attributable to
        # the exact branch instead of surfacing only as an unmapped read.
        "overlay": "main",
        "address": "8005EFDC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceResultStringLength",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8005FA9C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceResultFormat",
        "mode": "pre",
    },
    {
        # Appended original-V8 result banks are absent from V8:2's boot-time
        # retail file tree. Resolve only that nested loose path to a native
        # file descriptor, then leave its XA streamer/filter/SPU path intact.
        "overlay": "main",
        "address": "80018210",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ResolveOriginalResultVoiceFile",
        "mode": "pre",
    },
    {
        # Keep the V8:2 XA player, callbacks, filtering, and SPU path. Original
        # V8 result banks store one driver per channel instead of one file per
        # driver, so translate only the native result player's channel.
        "overlay": "main",
        "address": "8001DA14",
        "target": "RecompOne.Runtime.Sdk.V82Compat.OverrideResultVoiceChannel",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80020A80",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TrackVramAllocationPost",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80020DF0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.IgnoreSyntheticVramFree",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8002091C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.MarkOriginalMatchVramReset",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80014D00",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceGameplayOrderingTable",
        "mode": "pre",
    },
    {
        # Maximum LOD can exceed the retail 128 KiB primitive buffers. Preserve
        # old-buffer accounting before func_80014B3C flips packet arenas.
        "overlay": "main",
        "address": "80014B3C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PrepareExpandedPrimitiveBuffer",
        "mode": "pre",
    },
    {
        # Retain the retail OT/object retirement, then redirect the new frame's
        # packet cursor into a reserved devkit-RAM arena.
        "overlay": "main",
        "address": "80014B3C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ActivateExpandedPrimitiveBuffer",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80015D9C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RecoverMatchVramFailure",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80031DDC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceObjectFactorySource",
        "mode": "pre",
    },
    {
        # Arena event DLLs pass linked key addresses to the global object
        # registry. Relocate the register value as well as memory reads so the
        # registry's pointer-identity lookup can match the active arena key.
        "overlay": "main",
        "address": "80031994",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RelocateLookupKey",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80031DDC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceObjectFactoryResult",
        "mode": "post",
    },
    {
        # Model/object nodes allocated from relocated vehicle or arena DLLs
        # retain linked callbacks and hierarchy pointers. Record their owning
        # image and reject packed/corrupt non-RAM values before a later weapon
        # callback traverses them as child/sibling links.
        "overlay": "main",
        "address": "8002C17C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ValidateConstructedObject",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "8001B750",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RepairTerrainQuery",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001B944",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RepairTerrainQuery",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001B998",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RepairTerrainQuery",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8002D9E0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RepairObjectTerrainQuery",
        "mode": "pre",
    },
    {
        # Extended draw distance moves only the object limit, leaving distant
        # props standing past the last terrain row. Push the traversal polygon
        # out to match before the walker selects rows.
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ScaleTerrainTraversalRange",
        "mode": "pre",
    },
    {
        # func_8001C158 hands func_8001BECC the completed world-space traversal
        # polygon. Its far corners lose lateral reach against the widened 16:9
        # frustum whenever the camera is pitched, so refit them here before the
        # walker selects terrain rows.
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainTraversalWideFit",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainTraversalAspect",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainTraversalOmnidirectional",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainTraversalLateral",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainTraversalPolygon",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001BECC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceTerrainTraversalPolygon",
        "mode": "pre",
    },
    {
        # func_8001BE68 receives the inclusive/exclusive X-cell span selected
        # for one terrain row. Diagnostic widescreen padding at this exact
        # layer distinguishes insufficient row coverage from packet clipping.
        "overlay": "main",
        "address": "8001BE68",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainRowSpan",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "800288E0",
        "target": (
            "RecompOne.Runtime.Sdk.V82Compat."
            "BeginTerrainRoutePacketWrites"
        ),
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "800288E0",
        "target": (
            "RecompOne.Runtime.Sdk.V82Compat."
            "EndTerrainRoutePacketWrites"
        ),
        "mode": "post",
    },
    {
        # func_8001C158 constructs the native terrain traversal polygon from
        # gp+0xEDC (horizontal extent) and gp+0xF20 (vertical extent). Expand
        # only the horizontal source while Enhanced true widescreen is active,
        # then restore it immediately after the traversal completes.
        "overlay": "main",
        "address": "8001C134",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandTerrainFrustum",
        "mode": "pre",
    },
    {
        "overlay": "main",
        # Restore the authored 320-pixel packet clip before func_8001C158
        # copies gp+0xEDC into scratchpad+0x94. The expanded value is only a
        # world-space traversal input; carrying it into the packet clip makes
        # terrain clipping asymmetric after the GTE widescreen projection.
        "address": "8001C89C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RestoreTerrainFrustum",
        "mode": "pre",
    },
    {
        # func_8002DFF0 builds the object/scenery frustum planes from the
        # authored 320-wide clip width, and func_8002E22C tests every object
        # and every backdrop quad against them. Widen the width across the
        # build so the planes cover the widescreen view, then restore it.
        "overlay": "main",
        "address": "8002DFF0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExpandObjectFrustum",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8002DFF0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RestoreObjectFrustum",
        "mode": "post",
    },
    {
        # Capture the native object renderer's exact packet-buffer interval so
        # Enhanced receives vehicle/material ownership before GPU decoding.
        "overlay": "main",
        "address": "8002D9E0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.BeginObjectRender",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8002D9E0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.EndObjectRender",
        "mode": "post",
    },
    {
        # Resolve each native model descriptor to its authored imported bank
        # and group before it emits packets.  This differentiates body/LOD/
        # wheel geometry without texture, palette, colour, or screen heuristics.
        "overlay": "main",
        "address": "80021F70",
        "target": "RecompOne.Runtime.Sdk.V82Compat.BeginImportedRenderGroup",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80021F70",
        "target": "RecompOne.Runtime.Sdk.V82Compat.EndImportedRenderGroup",
        "mode": "post",
    },
    {
        # func_8002E22C is the per-object frustum test. Its three planes are
        # built for the authored 4:3 view, so widescreen culls objects that are
        # still on screen at the left and right edges.
        "overlay": "main",
        "address": "8002E22C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.WidenObjectVisibilityTest",
        "mode": "pre",
    },
    {
        # func_8002D9E0 compares this transform's distance output against
        # 0x003FFFFF immediately after return.
        "overlay": "main",
        "address": "80059AFC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ExtendObjectDrawDistance",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80033234",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceCommonObjectLoadPre",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80033234",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceCommonObjectLoadPost",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80036B64",
        "target": "RecompOne.Runtime.Sdk.V82VehicleRegistry.ResolveVehicleCallback",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80036C2C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceVehicleCreateRequest",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "800117C0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ServiceMatchStartPadWait",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "800313C8",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RunObjectScheduler",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80022164",
        "target": "RecompOne.Runtime.Sdk.V82Compat.EnterGeometry22164",
        "mode": "pre",
    },
    {
        # A damaged textured primitive can index a packed vertex word as a
        # texture pointer. Skip that primitive and continue the retail stream.
        "overlay": "main",
        "address": "80022E78",
        "target": "RecompOne.Runtime.Sdk.V82Compat.GuardGeometry22E78",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80022164",
        "target": "RecompOne.Runtime.Sdk.V82Compat.LeaveGeometryContinuation",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80022910",
        "target": "RecompOne.Runtime.Sdk.V82Compat.EnterGeometry22910",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80022910",
        "target": "RecompOne.Runtime.Sdk.V82Compat.LeaveGeometryContinuation",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80052F9C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.SpuMallocPre",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80052F9C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.SpuMallocPost",
        "mode": "post",
    },
    {
        "overlay": "main",
        "address": "80018110",
        "target": "RecompOne.Runtime.Sdk.V82Compat.WaitForSector",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80014D94",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ServiceDrawSyncWait",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80021C24",
        "target": "RecompOne.Runtime.Sdk.V82Compat.ServiceDisplayTransitionWait",
        "mode": "pre",
    },
    {
        # Selector text/preview probes are completely dormant unless
        # RECOMPONE_TRACE_V82_SELECTOR=1. Keeping the hooks scoped to the
        # native SHELL selector lets us identify the exact retail draw calls
        # without a wrapper UI or framebuffer guesswork.
        "overlay": "main",
        "address": "8001A3B0",
        "target": "RecompOne.Runtime.Sdk.V82VehicleRegistry.OverrideNativeSelectorText",
        "mode": "pre",
    },
    {
        # Observe the retail SHELL text renderer rather than introducing a
        # host-side menu. This supplies deterministic stage/capture points for
        # the unmodified V8:2 Options pages and native PC extensions.
        "overlay": "main",
        "address": "8001A3B0",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceNativeOptionsText",
        "mode": "pre",
    },
    {
        # Gameplay pause/objective prompts use the sibling formatted-text
        # renderer. Observe that stock seam as well so context-sensitive PC
        # bindings can retain ordinary face-button navigation in overlays.
        "overlay": "main",
        "address": "8001A6BC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceNativeOptionsText",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001ADF8",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceNativeSelectorCall",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80019564",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceNativeSelectorCall",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80019614",
        "target": "RecompOne.Runtime.Sdk.V82Compat.TraceNativeSelectorCall",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8001EF34",
        "target": (
            "RecompOne.Runtime.Sdk.V82VehicleRegistry."
            "PrepareNativeSelectorBank"
        ),
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8003C464",
        "target": "RecompOne.Runtime.Sdk.V82VehicleRegistry.BuildNativeSelectorPreview",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8003C464",
        "target": "RecompOne.Runtime.Sdk.V82VehicleRegistry.FinalizeNativeSelectorPreview",
        "mode": "post",
    },
    {
        # Retain the sequel's native camera object/lifecycle while imported
        # vehicles use the first game's exact follow distance.
        "overlay": "main",
        "address": "8004B898",
        "target": (
            "RecompOne.Runtime.Sdk.V82VehicleRegistry."
            "BeginOriginalV8CameraValues"
        ),
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "8004B898",
        "target": (
            "RecompOne.Runtime.Sdk.V82VehicleRegistry."
            "FinishOriginalV8CameraValues"
        ),
        "mode": "post",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "8010536C",
        "target": "RecompOne.Runtime.Sdk.V82VehicleRegistry.ZoomNativeSelectorPreview",
        "mode": "pre",
    },
    {
        # The native selector calls this once per inner-frame update. Guest
        # proof capture waits here until the rotating model and stat bars have
        # settled, so an interpolation frame cannot be mistaken for a result.
        "overlay": "main",
        "address": "800149AC",
        "target": "RecompOne.Runtime.Sdk.V82VehicleRegistry.TickNativeSelector",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80018148",
        "target": "RecompOne.Runtime.Sdk.LibCd.BeginV82FileRead",
        "mode": "pre",
    },
    {
        "overlay": "main",
        "address": "80018478",
        "target": "RecompOne.Runtime.Sdk.LibCd.ReadV82FileBytes",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80018644",
        "target": "RecompOne.Runtime.Sdk.LibCd.SeekV82File",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80055E00",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdSyncCallback",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80055E14",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdReadyCallback",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80055E28",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdControl",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80055F64",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdControlF",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80056098",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdControlB",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80056204",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdGetSector",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800580FC",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdRead",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "80058298",
        "target": "RecompOne.Runtime.Sdk.LibCd.CdReadSync",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005881C",
        "target": "RecompOne.Runtime.Sdk.LibCdStream.StSetRing",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "8005884C",
        "target": "RecompOne.Runtime.Sdk.LibCdStream.StClearRing",
        "mode": "replace",
    },
    {
        "overlay": "main",
        "address": "800588AC",
        "target": "RecompOne.Runtime.Sdk.LibCdStream.StSetStream",
        "mode": "replace",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80111330",
        "target": "RecompOne.Runtime.Sdk.LibCdStream.StFreeRing",
        "mode": "replace",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "8010669C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.BeginNativeGuestSelector",
        "mode": "pre",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "8010669C",
        "target": "RecompOne.Runtime.Sdk.V82Compat.EndNativeGuestSelector",
        "mode": "post",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80107AD4",
        "target": "RecompOne.Runtime.Sdk.V82Compat.BeginNativeGuestSelector",
        "mode": "pre",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80107AD4",
        "target": "RecompOne.Runtime.Sdk.V82Compat.EndNativeGuestSelector",
        "mode": "post",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "801113E0",
        "target": "RecompOne.Runtime.Sdk.LibCdStream.StGetNext",
        "mode": "replace",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80111AE4",
        "target": "RecompOne.Runtime.Sdk.V8Compat.TranslateOverlayDmaSource",
        "mode": "pre",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "801109FC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PreserveShellDecodeCallerPre",
        "mode": "pre",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "801109FC",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PreserveShellDecodeCallerPost",
        "mode": "post",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "801105D4",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PreserveShellImageDecodePre",
        "mode": "pre",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "801105D4",
        "target": "RecompOne.Runtime.Sdk.V82Compat.PreserveShellImageDecodePost",
        "mode": "post",
    },
    {
        "overlay": "SHELL_LOAD",
        "address": "80109704",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RunLoadVlc",
        "mode": "replace",
    },
    {
        # SHELL uses the same hand-written, tail-entered VLC decoder shape as
        # LOAD. Recompiling its internal labels as independent C# functions
        # loses native RA/delay-slot flow and leaves menu/FMVs with the gray
        # 32-pixel strip in place of their decoded background.
        "overlay": "SHELL_SHELL",
        "address": "80110D14",
        "target": "RecompOne.Runtime.Sdk.V82Compat.RunShellVlc",
        "mode": "replace",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80114E48",
        "target": "RecompOne.Runtime.Sdk.V8Compat.WaitCardEvent",
        "mode": "replace",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80114F20",
        "target": "RecompOne.Runtime.Sdk.V8Compat.WaitCardEvent",
        "mode": "replace",
    },
    {
        "overlay": "SHELL_SHELL",
        "address": "80113C18",
        "target": "RecompOne.Runtime.Sdk.V82Compat.WaitCardOperation",
        "mode": "replace",
    },
]


def relative_posix(path: Path, start: Path) -> str:
    return Path(os.path.relpath(path.resolve(), start.resolve())).as_posix()


def overlay_name(relative_path: Path) -> str:
    parts = [part.upper().replace("-", "_") for part in relative_path.with_suffix("").parts]
    return "_".join(parts)


def discover_overlays(disc_root: Path) -> list[dict[str, object]]:
    preferred = [
        disc_root / "SHELL" / "SHELL.DLL",
        disc_root / "SHELL" / "LOAD.DLL",
    ]
    remaining = sorted(
        (
            path
            for path in disc_root.rglob("*.DLL")
            if path not in preferred
        ),
        key=lambda path: path.relative_to(disc_root).as_posix().upper(),
    )

    overlays: list[dict[str, object]] = []
    seen_names: set[str] = set()
    for path in [*preferred, *remaining]:
        if not path.is_file():
            raise FileNotFoundError(f"required overlay is missing: {path}")
        relative = path.relative_to(disc_root)
        name = overlay_name(relative)
        if name in seen_names:
            raise ValueError(f"duplicate generated overlay name: {name}")
        seen_names.add(name)
        overlay = {
            "name": name,
            "base": "0x80100000",
            "file": str(relative).replace("/", "\\"),
            "v8Relocate": True,
            "linearSweep": True,
        }
        if name in EXTRA_OVERLAY_FUNCTIONS:
            overlay["functions"] = [
                {"address": address}
                for address in EXTRA_OVERLAY_FUNCTIONS[name]
            ]
        overlays.append(overlay)
    return overlays


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cue", type=Path, default=DEFAULT_CUE)
    parser.add_argument("--disc-root", type=Path, default=DEFAULT_DISC)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    cue = args.cue.resolve()
    disc_root = args.disc_root.resolve()
    output = args.output.resolve()
    if not cue.is_file():
        raise FileNotFoundError(f"retail CUE is missing: {cue}")
    if not (disc_root / "SYSTEM.CNF").is_file():
        raise FileNotFoundError(f"extracted disc root is missing SYSTEM.CNF: {disc_root}")
    if not (disc_root / "SLUS_008.68").is_file():
        raise FileNotFoundError(f"extracted main executable is missing: {disc_root / 'SLUS_008.68'}")

    output.mkdir(parents=True, exist_ok=True)
    config = {
        "game": {
            "id": "SLUS-00868",
            "name": "Vigilante82PC",
            "title": "Vigilante 8: 2nd Offense PC",
            "output": "recompiled",
        },
        "cue": relative_posix(cue, output),
        "debug": False,
        "linearSweep": True,
        "functions": [{"address": address} for address in EXTRA_MAIN_FUNCTIONS],
        "overlays": discover_overlays(disc_root),
        "stubs": [],
        "ignored": [],
        "patches": PROVEN_PATCHES,
    }

    config_path = output / "v82.recompone.json"
    config_path.write_text(json.dumps(config, indent=2) + "\n", encoding="utf-8")
    host_source = Path(__file__).resolve().parent / "reference-host"
    generated_project = output / "recompiled"
    generated_project.mkdir(parents=True, exist_ok=True)
    for host_file in ("Program.cs", "Vigilante82PC.csproj"):
        shutil.copy2(host_source / host_file, generated_project / host_file)
    print(f"Wrote {config_path}")
    print(f"Configured {len(config['overlays'])} sequel overlays")
    print(f"Refreshed host project in {generated_project}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
