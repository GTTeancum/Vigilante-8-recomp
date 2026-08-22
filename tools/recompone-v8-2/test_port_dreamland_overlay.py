#!/usr/bin/env python3
"""Regression tests for reusable V8-to-V8:2 overlay object ABI conversion."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


MODULE_PATH = Path(__file__).with_name("port_dreamland_overlay.py")
SPEC = importlib.util.spec_from_file_location("port_dreamland_overlay", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
PORT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PORT)


class ObjectPointerAliasTests(unittest.TestCase):
    def test_generated_overlay_uses_native_water_once_per_lifecycle(self) -> None:
        generated = PORT.generate(PORT.SOURCE, "DreamLnd", 0x0002A400)
        self.assertEqual(generated.count("Vigilante82PC.func_80017F34(c, m);"), 1)
        self.assertEqual(generated.count("Vigilante82PC.func_80017EB8(c, m);"), 1)
        self.assertIn("c.A0 = 0x0002A400u;", generated)
        self.assertNotIn("0x8FFF0104", generated)
        event_zero = generated.index("        L8010067C: ;")
        event_one = generated.index("        L80100984: ;")
        init_call = generated.index("Vigilante82PC.func_80017F34(c, m);")
        self.assertGreater(init_call, event_one)
        self.assertNotIn(
            "Vigilante82PC.func_80017F34(c, m);",
            generated[event_zero:event_one],
        )

    def test_primary_event_abi_inserts_v82_event_16(self) -> None:
        source = (
            "    public static void Arena(CpuContext c, IMemory m)\n"
            "    {\n"
            "        c.V0 = c.A1 < 0x00000012u ? 1u : 0u;\n"
            "        if (c.V0 == 0u) {\n"
            "            c.StoreWord(17, m, (c.SP + 0x24u));\n"
            "            goto L801009C8;\n"
            "        }\n"
            "        c.V1 = c.A1 << 2;\n"
            "    }\n"
            "    [System.Runtime.CompilerServices.MethodImpl("
            "System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]\n"
        )
        converted = PORT.convert_primary_event_abi(source, "Arena")
        self.assertIn("c.A1 < 0x00000013u", converted)
        self.assertIn("c.A1 == 0x00000010u", converted)
        self.assertIn(
            "c.V1 = c.A1 < 0x00000011u ? c.A1 : c.A1 - 0x1u;",
            converted,
        )

    def test_child_object_field_is_translated(self) -> None:
        body = (
            "        c.CopyRegister(16, 4);\n"
            "        c.LoadWord(4, m, (c.S0 + 0x10u));\n"
            "        c.V1 = m.ReadU16((c.A0 + 0x40u));\n"
            "        m.WriteU16((c.A0 + 0x40u), (ushort)c.V1);\n"
        )
        converted = PORT._rewrite_object_pointer_aliases("DBridge", body)
        self.assertEqual(converted.count("c.A0 + 0x44u"), 2)
        self.assertNotIn("c.A0 + 0x40u", converted)

    def test_non_link_load_does_not_create_object_alias(self) -> None:
        body = (
            "        c.CopyRegister(16, 4);\n"
            "        c.LoadWord(4, m, (c.S0 + 0x5Cu));\n"
            "        c.V1 = m.ReadU16((c.A0 + 0x40u));\n"
        )
        converted = PORT._rewrite_object_pointer_aliases("DBridge", body)
        self.assertIn("c.A0 + 0x40u", converted)

    def test_redefined_alias_stops_field_translation(self) -> None:
        body = (
            "        c.CopyRegister(16, 4);\n"
            "        c.LoadWord(4, m, (c.S0 + 0x10u));\n"
            "        c.V1 = m.ReadU16((c.A0 + 0x40u));\n"
            "        c.A0 = c.SP + 0x10u;\n"
            "        c.V1 = m.ReadU16((c.A0 + 0x40u));\n"
        )
        converted = PORT._rewrite_object_pointer_aliases("DBridge", body)
        self.assertEqual(converted.count("c.A0 + 0x44u"), 1)
        self.assertEqual(converted.count("c.A0 + 0x40u"), 1)

    def test_allocator_return_fields_are_translated(self) -> None:
        body = (
            "        Vigilante82PC.func_8002C17C(c, m);\n"
            "        c.CopyRegister(17, 2);\n"
            "        m.WriteU8((c.S1 + 0x4u), (byte)c.V0);\n"
            "        m.WriteU16((c.S1 + 0x6u), (ushort)c.V0);\n"
            "        c.StoreWord(8, m, c.S1);\n"
            "        c.StoreWord(9, m, (c.S1 + 0x64u));\n"
        )
        converted = PORT._rewrite_object_pointer_aliases("Canon", body)
        self.assertIn("c.S1 + 0x8u", converted)
        self.assertIn("c.S1 + 0xAu", converted)
        self.assertIn("c.StoreWord(8, m, (c.S1 + 0x4u));", converted)
        self.assertIn("c.StoreWord(9, m, (c.S1 + 0x0u));", converted)

    def test_clone_return_fields_are_translated(self) -> None:
        body = (
            "        Vigilante82PC.func_80052188(c, m);\n"
            "        c.CopyRegister(16, 2);\n"
            "        m.WriteU16((c.S0 + 0x6u), (ushort)c.V0);\n"
            "        m.WriteU8((c.S0 + 0x4u), (byte)c.V0);\n"
            "        c.StoreWord(2, m, c.S0);\n"
            "        c.StoreWord(19, m, (c.S0 + 0x64u));\n"
        )
        converted = PORT._rewrite_object_pointer_aliases(
            "DreamProjectileSpawn", body
        )
        self.assertIn("c.S0 + 0xAu", converted)
        self.assertIn("c.S0 + 0x8u", converted)
        self.assertIn("c.StoreWord(2, m, (c.S0 + 0x4u));", converted)
        self.assertIn("c.StoreWord(19, m, (c.S0 + 0x0u));", converted)

    def test_ordinary_call_does_not_seed_object_return(self) -> None:
        body = (
            "        Vigilante82PC.func_8002AC5C(c, m);\n"
            "        c.CopyRegister(17, 2);\n"
            "        m.WriteU16((c.S1 + 0x40u), (ushort)c.V0);\n"
        )
        converted = PORT._rewrite_object_pointer_aliases("Canon", body)
        self.assertIn("c.S1 + 0x40u", converted)

    def test_native_call_invalidates_caller_saved_object_alias(self) -> None:
        body = (
            "        c.LoadWord(4, m, (c.S0 + 0x10u));\n"
            "        Vigilante82PC.func_8002AC5C(c, m);\n"
            "        m.WriteU16((c.A0 + 0x40u), (ushort)c.V0);\n"
        )
        converted = PORT._rewrite_object_pointer_aliases("DBridge", body)
        self.assertIn("c.A0 + 0x40u", converted)


if __name__ == "__main__":
    unittest.main()
