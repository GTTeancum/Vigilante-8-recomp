"""Asset-free original-engine conversion and old-roster upgrade regressions."""
from __future__ import annotations

import base64
from dataclasses import replace
from pathlib import Path
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest
from zipfile import ZipFile
from unittest.mock import patch

from build_v8_to_v82_guest_roster import VEHICLES, original_engine_sounds
from test_native_formats import DonorFreeCompilerTests as FixtureFactory
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "recompone-v8-2"))
from build_guest_engine_updater import build as build_zipapp
from apply_vehicle_engine_patches import patch_ground_engine
from upgrade_guest_engine_audio import main as upgrade_cli, upgrade
from vigilante8_vehicle_tools import compiler, iff, project, registry, stats

# The base fixture class has its own unrelated tests; don't collect those twice.
fixture_vehicle = FixtureFactory._project
del FixtureFactory


def clip(marker: int, looping: bool) -> dict[str, object]:
    # Synthetic PS1 ADPCM, two 28-sample blocks. Only engines repeat.
    payload = (bytes((2, 4 if looping else 0)) + bytes([marker]) * 14
               + bytes((2, 3 if looping else 1)) + bytes([marker ^ 0xFF]) * 14)
    return {"pitch": 1024 + marker, "adpcm_base64": base64.b64encode(payload).decode("ascii")}


def base():
    return fixture_vehicle("V8_2")


def arms_archive(omit_last: bool = False) -> bytes:
    forms = []
    for index, _, _, _ in VEHICLES[:(-1 if omit_last else None)]:
        marker = index + 1
        snd = compiler.compile_sounds(replace(base(), sounds=(clip(marker, True),)))
        forms.append(iff.IffChunk(tag=b"FORM", form_type=b"XOBF", children=[
            iff.IffChunk(tag=b"SND ", payload=snd)]).to_bytes())
    return b"".join(forms)


def synthetic_disc(arms: bytes, title: str = "SLUS_005.10") -> bytes:
    """Minimal MODE2/2352 ISO with only the three file-system entries read by the updater."""
    def record(name: bytes, lba: int, size: int, directory=False) -> bytes:
        n = (33 + len(name) + 1) & ~1
        rec = bytearray(n)
        rec[0] = n
        for off, value in ((2, lba), (10, size)):
            struct.pack_into("<I", rec, off, value)
            struct.pack_into(">I", rec, off + 4, value)
        rec[25] = 2 if directory else 0
        rec[32] = len(name)
        rec[33:33 + len(name)] = name
        return bytes(rec)

    boot = f"BOOT = cdrom:\\{title};1\r\n".encode()
    raw = bytearray(2352 * (22 + (len(arms) + 2047) // 2048))
    def put(lba: int, data: bytes) -> None:
        for pos in range(0, len(data), 2048):
            payload = data[pos:pos + 2048]
            start = (lba + pos // 2048) * 2352 + 24
            raw[start:start + len(payload)] = payload
    pvd = bytearray(2048); pvd[:7] = b"\x01CD001\x01"
    root = record(b"\x00", 18, 2048, True); pvd[156:156 + len(root)] = root
    put(16, pvd)
    put(18, record(b"SYSTEM.CNF;1", 20, len(boot)) + record(b"HUD", 19, 2048, True))
    put(19, record(b"ARMS.EXP;1", 21, len(arms)))
    put(20, boot); put(21, arms)
    return bytes(raw)


def originals():
    # Deliberately non-roster order and with an independent mod alongside.
    seed = base()
    return (replace(seed, stable_id=VEHICLES[7][1], sounds=(clip(70, False), clip(71, False))),
            replace(seed, stable_id=VEHICLES[0][1], sounds=(clip(72, False),)),
            replace(seed, stable_id="test.stock_style", sounds=(clip(73, False),)))


def write_fixture(directory: Path):
    old = registry.compile_package(originals())
    archive, records = upgrade(old.archive, old.registry, arms_archive())
    directory.mkdir(exist_ok=True, parents=True)
    (directory / "CUSTOM.EXP").write_bytes(archive)
    (directory / "VEHICLES.V8R").write_bytes(records)
    for i in range(3):
        (directory / f"SELECTOR_{i:02}.PPM").write_bytes(b"P6\n260 422\n255\n" + bytes(260*422*3))
    # Pass exactly the sound chunk from the updated on-disk body to the
    # real registry/SPU contract, not a separately built expected table.
    form = next(iff.parse(archive).forms(b"XOBF"))
    (directory / "SYNTHETIC.SND").write_bytes(next(form.leaves(b"SND ")).payload)


class GuestEngineAudioTests(unittest.TestCase):
    def test_upgrade_and_json_registry_round_trip(self):
        original = originals()
        old = registry.compile_package(original)
        baseline = registry.decompile_package(old.archive, old.registry)
        arms = arms_archive()
        a, r = upgrade(old.archive, old.registry, arms)
        migrated = registry.decompile_package(a, r)
        for i, source_index in enumerate((7, 0)):
            self.assertEqual(len(original[i].sounds), migrated[i].ground_engine_sample)
            self.assertEqual(original[i].sounds, migrated[i].sounds[:-1])  # original specials unchanged
            self.assertEqual(clip(source_index + 1, True), migrated[i].sounds[-1])
            self.assertEqual(migrated[i], project.VehicleProject.from_dict(project.to_dict(migrated[i])))
            self.assertEqual(baseline[i], replace(migrated[i], sounds=migrated[i].sounds[:-1], ground_engine_sample=0))
        self.assertEqual(baseline[2], migrated[2])  # unrelated mod not remapped, even in same package
        self.assertEqual((a, r), upgrade(a, r, arms))

    def test_original_usa_stats_layouts_are_unique_and_edits_stay_in_the_table(self):
        load = 0x80010000
        original, rev1 = stats.PROFILES["V8"], stats.RETAIL_LAYOUTS[-1]
        def image(*layouts):
            buf = bytearray(0x51000)
            buf[:8] = b"PS-X EXE"
            struct.pack_into("<II", buf, 0x18, load, len(buf) - 0x800)
            for layout in layouts:
                for i in range(layout.record_count):
                    buf[0x800 + layout.table_address - load + i * layout.record_size + 0xD] = i
            return bytes(buf)
        for layout in (original, rev1):
            for selected_game in ("V8", None):
                source = image(layout)
                parsed = stats.StatsFile(source, selected_game)
                self.assertEqual(layout.table_address, parsed.profile.table_address)
                parsed.record(2).set("health", 1)
                self.assertEqual(1, sum(a != b for a,b in zip(parsed.data, source)))
        for bad in (image(), image(original, rev1), image(rev1)[:-0x1000]):
            with self.assertRaisesRegex(ValueError, "uniquely identify"):
                stats.StatsFile(bad, "V8")

    def test_missing_driver_asset_rejected(self):
        with self.assertRaisesRegex(ValueError, "y_the_alien"):
            original_engine_sounds(arms_archive(omit_last=True))

    def test_rejects_wrong_archive_or_nonloop(self):
        arms = arms_archive()
        with self.assertRaisesRegex(ValueError, "not a vehicle archive"):
            original_engine_sounds(arms + arms)
        forms = tuple(iff.parse(arms).forms(b"XOBF"))
        forms[3].children[0].payload = compiler.compile_sounds(
            replace(base(), sounds=(clip(44, False),)))
        with self.assertRaisesRegex(ValueError, "not a playable loop"):
            original_engine_sounds(b"".join(form.to_bytes() for form in forms))

    def test_only_native_engine_call_is_mapped_and_patch_reapplies_safely(self):
        header = "    public static void func_80038408(CpuContext c, IMemory m)\n    {\n"
        call = "        Vigilante82PC.func_8001E098(c, m);"
        other = "    public static void func_80000008(CpuContext c, IMemory m)\n    {\n" + call + "\n    }"
        source = header + "        c.A0 = 1u;\n" + call + "\n    }\n" + other
        patched = patch_ground_engine(source)
        self.assertEqual(patched, patch_ground_engine(patched))
        self.assertTrue(patched.endswith(other))
        self.assertLess(patched.index("uint groundEngineVehicle = c.A0;"), patched.index("c.A0 = 1u;"))
        for bad in (source.replace(call, "", 1), source.replace(call, call+"\n"+call, 1),
                    patched.replace("uint groundEngineVehicle = c.A0;", ""),
                    patched.replace(call, "        c.A2 = 0u;\n" + call, 1)):
            with self.assertRaises(RuntimeError):
                patch_ground_engine(bad)

    def test_distributable_updates_from_disc_without_repo_or_blender(self):
        with TemporaryDirectory() as temp:
            folder = Path(temp); source = folder / "old"; source.mkdir()
            tool = folder / "OriginalEngineAudio.pyz"
            build_zipapp(tool)
            with ZipFile(tool) as z:
                self.assertEqual(11, len(z.namelist()))
                for name in z.namelist():
                    self.assertTrue(name.endswith(".py"))
                    compile(z.read(name), name, "exec")
            arms = arms_archive()
            disc = folder / "Track 01.bin"; disc.write_bytes(synthetic_disc(arms))
            old = registry.compile_package(originals())
            (source / "CUSTOM.EXP").write_bytes(old.archive)
            (source / "VEHICLES.V8R").write_bytes(old.registry)
            output = folder / "pair"
            command = [sys.executable, "-I", str(tool), "--package", str(source),
                       "--disc-bin", str(disc), "--output", str(output)]
            subprocess.run(command, cwd=folder, capture_output=True, check=True)
            self.assertEqual(upgrade(old.archive, old.registry, arms),
                             ((output / "CUSTOM.EXP").read_bytes(), (output / "VEHICLES.V8R").read_bytes()))
            disc.write_bytes(synthetic_disc(arms, "SLUS_008.68"))
            rejected = folder / "refused"
            run = subprocess.run(command[:-1] + [str(rejected)], cwd=folder, capture_output=True)
            self.assertNotEqual(0, run.returncode); self.assertIn(b"wrong disc", run.stderr)
            self.assertFalse(rejected.exists())
            run = subprocess.run(command[:-1] + [str(source / "nested")], cwd=folder, capture_output=True)
            self.assertNotEqual(0, run.returncode); self.assertIn(b"outside the input package", run.stderr)
            self.assertFalse((source / "nested").exists())

    def test_out_of_bounds_engine_rejected(self):
        v = originals()[0]
        with self.assertRaisesRegex(ValueError, "ground_engine_sample"):
            replace(v, ground_engine_sample=len(v.sounds)).validate()

    def test_existing_engine_mapping_cannot_be_replaced_silently(self):
        v = replace(originals()[0], ground_engine_sample=1)
        pkg = registry.compile_package((v,))
        with self.assertRaisesRegex(ValueError, "different engine"):
            upgrade(pkg.archive, pkg.registry, arms_archive())

    def test_cli_never_modifies_input_directory(self):
        with TemporaryDirectory() as temp:
            folder = Path(temp); src = folder / "source"; src.mkdir()
            arms = folder / "ARMS.EXP"; arms.write_bytes(arms_archive())
            out = folder / "built"; old = registry.compile_package(originals())
            (src / "CUSTOM.EXP").write_bytes(old.archive)
            (src / "VEHICLES.V8R").write_bytes(old.registry)
            with patch.object(sys, "argv", ["upgrade", "--package", str(src), "--arms", str(arms), "--output", str(out)]):
                upgrade_cli()
                with self.assertRaises(SystemExit):
                    upgrade_cli()
            self.assertEqual(old.archive, (src/"CUSTOM.EXP").read_bytes())
            self.assertEqual(old.registry, (src/"VEHICLES.V8R").read_bytes())
            self.assertEqual({"CUSTOM.EXP","VEHICLES.V8R"}, {p.name for p in out.iterdir()})


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--write-fixture":
        write_fixture(Path(sys.argv[2]))
    else:
        unittest.main()
