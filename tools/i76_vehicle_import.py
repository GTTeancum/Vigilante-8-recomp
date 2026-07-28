#!/usr/bin/env python3
"""Read Interstate '76 vehicle geometry and VQ-compressed materials."""

from __future__ import annotations

import math
import re
import struct
from dataclasses import dataclass
from pathlib import Path


GEO_HEADER = struct.Struct("<4si16siii")
GEO_FACE = struct.Struct("<iiBBBffffi3s13sii")
GEO_CORNER = struct.Struct("<iiff")
VGEO_RECORD = struct.Struct("<8s12f8s7fif")


def normalized_name(value: str) -> str:
    return re.sub(r"[^a-z0-9]", "", Path(value).stem.lower())


@dataclass(frozen=True)
class PackedEntry:
    pack_stem: str
    name: str
    offset: int
    size: int


@dataclass(frozen=True)
class GeoCorner:
    vertex: int
    normal: int
    u: float
    v: float


@dataclass(frozen=True)
class GeoFace:
    color: tuple[int, int, int]
    material: str
    render_flags: tuple[int, int, int]
    corners: tuple[GeoCorner, ...]


@dataclass(frozen=True)
class GeoMesh:
    name: str
    vertices: tuple[tuple[float, float, float], ...]
    normals: tuple[tuple[float, float, float], ...]
    faces: tuple[GeoFace, ...]


@dataclass(frozen=True)
class Placement:
    name: str
    parent: str
    matrix: tuple[float, ...]
    geo_type: int
    geo_flags: int


@dataclass(frozen=True)
class WheelLocation:
    index: int
    matrix: tuple[float, ...]


@dataclass(frozen=True)
class Hardpoint:
    name: str
    index: int
    mount_class: int
    weapon_class: int
    matrix: tuple[float, ...]


@dataclass(frozen=True)
class VehicleConfig:
    label: str
    vehicle_definition: str
    appearance_definition: str
    wheel_definitions: tuple[str | None, str | None, str | None]
    weapons: tuple[str, ...]


class AssetStore:
    def __init__(self, root: Path):
        self.root = root.resolve()
        self._pack_data: dict[str, bytes] = {}
        self._entries: dict[tuple[str, str], list[PackedEntry]] = {}
        for pix_path in sorted(self.root.glob("*.pix")):
            pack_path = pix_path.with_suffix(".pak")
            if not pack_path.is_file():
                continue
            try:
                lines = pix_path.read_text(encoding="ascii").splitlines()
                declared_count = int(lines[0])
            except (UnicodeDecodeError, ValueError, IndexError):
                continue
            entries: list[PackedEntry] = []
            for line in lines[1:]:
                name, offset, size = line.rsplit(" ", 2)
                entries.append(
                    PackedEntry(
                        pix_path.stem,
                        name,
                        int(offset),
                        int(size),
                    )
                )
            if len(entries) != declared_count:
                raise ValueError(
                    f"{pix_path.name}: declared {declared_count} entries, "
                    f"found {len(entries)}"
                )
            data = pack_path.read_bytes()
            self._pack_data[pix_path.stem.lower()] = data
            for entry in entries:
                if entry.offset + entry.size > len(data):
                    raise ValueError(f"{pack_path.name}: bad entry {entry.name}")
                key = (
                    normalized_name(entry.name),
                    Path(entry.name).suffix.lower(),
                )
                self._entries.setdefault(key, []).append(entry)

    def find(self, name: str) -> PackedEntry | None:
        key = (normalized_name(name), Path(name).suffix.lower())
        matches = self._entries.get(key, ())
        return matches[0] if matches else None

    def payload(self, entry: PackedEntry) -> bytes:
        data = self._pack_data[entry.pack_stem.lower()]
        return data[entry.offset : entry.offset + entry.size]

    def named_payload(self, name: str) -> bytes | None:
        entry = self.find(name)
        return self.payload(entry) if entry is not None else None


def parse_geo(data: bytes) -> GeoMesh:
    if len(data) < GEO_HEADER.size:
        raise ValueError("GEO is shorter than its header")
    magic, _unknown, raw_name, vertex_count, face_count, _unknown2 = (
        GEO_HEADER.unpack_from(data)
    )
    if magic != b"OEG.":
        raise ValueError(f"unexpected GEO magic {magic!r}")
    name = raw_name.split(b"\0", 1)[0].decode("ascii")
    position = GEO_HEADER.size
    vertices = tuple(
        struct.unpack_from("<fff", data, position + index * 12)
        for index in range(vertex_count)
    )
    position += vertex_count * 12
    normals = tuple(
        struct.unpack_from("<fff", data, position + index * 12)
        for index in range(vertex_count)
    )
    position += vertex_count * 12
    faces: list[GeoFace] = []
    for _index in range(face_count):
        values = GEO_FACE.unpack_from(data, position)
        position += GEO_FACE.size
        corners = tuple(
            GeoCorner(*GEO_CORNER.unpack_from(data, position + corner * 16))
            for corner in range(values[1])
        )
        position += values[1] * GEO_CORNER.size
        material = values[11].split(b"\0", 1)[0].decode("ascii").strip()
        faces.append(
            GeoFace(
                (values[2], values[3], values[4]),
                material,
                tuple(values[10]),
                corners,
            )
        )
    if position != len(data):
        raise ValueError(f"{name}: consumed {position}, expected {len(data)}")
    return GeoMesh(name, vertices, normals, tuple(faces))


def load_geo_pack(store: AssetStore, pack_stem: str) -> dict[str, GeoMesh]:
    meshes: dict[str, GeoMesh] = {}
    for (norm, extension), entries in store._entries.items():
        if extension != ".geo":
            continue
        for entry in entries:
            if entry.pack_stem.lower() == pack_stem.lower():
                meshes[norm] = parse_geo(store.payload(entry))
    return meshes


def _placement_from_values(values: tuple[object, ...]) -> Placement | None:
    name = values[0].split(b"\0", 1)[0].decode("ascii")
    if not name or name.upper().startswith("NULL"):
        return None
    parent = values[13].split(b"\0", 1)[0].decode("ascii")
    return Placement(
        name,
        parent,
        tuple(values[1:13]),
        values[21],
        int(values[22]),
    )


def parse_vgeo_bank(vdf_path: Path, bank: int = 0) -> list[Placement]:
    data = vdf_path.read_bytes()
    offset = data.find(b"VGEO")
    if offset < 0:
        raise ValueError(f"{vdf_path.name}: VGEO section not found")
    _tag, section_size, geo_count = struct.unpack_from("<4sIi", data, offset)
    bank_count, remainder = divmod(
        section_size - 12, geo_count * VGEO_RECORD.size
    )
    if remainder or not 0 <= bank < bank_count:
        raise ValueError(f"{vdf_path.name}: invalid VGEO dimensions")
    placements: list[Placement] = []
    position = offset + 12 + bank * geo_count * VGEO_RECORD.size
    for index in range(geo_count):
        values = VGEO_RECORD.unpack_from(
            data, position + index * VGEO_RECORD.size
        )
        placement = _placement_from_values(values)
        if placement is not None:
            placements.append(placement)
    return placements


def parse_geometry_chunk(path: Path, tag: bytes) -> list[Placement]:
    """Parse a count-prefixed WGEO/GGEO/OGEO placement array."""
    data = path.read_bytes()
    offset = data.find(tag)
    if offset < 0:
        return []
    section_size = struct.unpack_from("<I", data, offset + 4)[0]
    payload = data[offset + 8 : offset + section_size]
    if len(payload) < 4 or (len(payload) - 4) % VGEO_RECORD.size:
        raise ValueError(f"{path.name}: invalid {tag.decode()} dimensions")
    placements: list[Placement] = []
    for position in range(4, len(payload), VGEO_RECORD.size):
        placement = _placement_from_values(
            VGEO_RECORD.unpack_from(payload, position)
        )
        if placement is not None:
            placements.append(placement)
    return placements


def parse_wheel_locations(vdf_path: Path) -> list[WheelLocation]:
    """Read the four wheel transforms around WLOC's central physics block."""
    data = vdf_path.read_bytes()
    offset = data.find(b"WLOC")
    if offset < 0:
        raise ValueError(f"{vdf_path.name}: WLOC section not found")
    section_size = struct.unpack_from("<I", data, offset + 4)[0]
    payload = data[offset + 8 : offset + section_size]
    if len(payload) != 336:
        raise ValueError(f"{vdf_path.name}: unexpected WLOC size")
    locations: list[WheelLocation] = []
    for position in (0, 56, 224, 280):
        values = struct.unpack_from("<i12fI", payload, position)
        locations.append(WheelLocation(values[0], tuple(values[1:13])))
    return locations


def parse_hardpoints(vdf_path: Path) -> list[Hardpoint]:
    data = vdf_path.read_bytes()
    hardpoints: list[Hardpoint] = []
    position = 0
    while True:
        offset = data.find(b"HLOC", position)
        if offset < 0:
            break
        section_size = struct.unpack_from("<I", data, offset + 4)[0]
        payload = data[offset + 8 : offset + section_size]
        if len(payload) != 80:
            raise ValueError(f"{vdf_path.name}: unexpected HLOC size")
        name = payload[:16].split(b"\0", 1)[0].decode("ascii")
        index, mount_class, weapon_class = struct.unpack_from("<iii", payload, 16)
        matrix = struct.unpack_from("<12f", payload, 28)
        hardpoints.append(
            Hardpoint(name, index, mount_class, weapon_class, matrix)
        )
        position = offset + section_size
    return hardpoints


def parse_vehicle_weapons(vcf_path: Path) -> list[str]:
    data = vcf_path.read_bytes()
    weapons: list[str] = []
    position = 0
    while True:
        offset = data.find(b"WEPN", position)
        if offset < 0:
            break
        section_size = struct.unpack_from("<I", data, offset + 4)[0]
        name = (
            data[offset + 12 : offset + section_size]
            .split(b"\0", 1)[0]
            .decode("ascii")
        )
        weapons.append(name)
        position = offset + section_size
    return weapons


def parse_vehicle_config(vcf_path: Path) -> VehicleConfig:
    """Read the model, appearance, wheel, and weapon references from VCFC."""
    data = vcf_path.read_bytes()
    offset = data.find(b"VCFC")
    if offset < 0:
        raise ValueError(f"{vcf_path.name}: VCFC section not found")
    section_size = struct.unpack_from("<I", data, offset + 4)[0]
    limit = offset + section_size
    if limit > len(data) or section_size < 24:
        raise ValueError(f"{vcf_path.name}: invalid VCFC size")

    payload = data[offset + 8 : limit]
    label = (
        payload[:16]
        .split(b"\0", 1)[0]
        .decode("ascii")
    )
    vehicle_match = re.search(rb"([A-Za-z0-9_]+\.vdf)\0", payload)
    appearance_match = re.search(rb"([A-Za-z0-9_]+\.vtf)\0", payload)
    wheel_matches = re.findall(
        rb"((?:[A-Za-z0-9_]+\.wdf)|null)\0", payload
    )
    if vehicle_match is None or appearance_match is None:
        raise ValueError(f"{vcf_path.name}: missing VCFC asset reference")
    if len(wheel_matches) != 3:
        raise ValueError(
            f"{vcf_path.name}: expected three VCFC wheel references, "
            f"found {len(wheel_matches)}"
        )
    vehicle_definition = vehicle_match.group(1).decode("ascii")
    appearance_definition = appearance_match.group(1).decode("ascii")
    front_wheel, middle_wheel, rear_wheel = (
        value.decode("ascii") for value in wheel_matches
    )

    def wheel_or_none(value: str) -> str | None:
        return None if value.lower() == "null" else value

    return VehicleConfig(
        label,
        vehicle_definition,
        appearance_definition,
        (
            wheel_or_none(front_wheel),
            wheel_or_none(middle_wheel),
            wheel_or_none(rear_wheel),
        ),
        tuple(parse_vehicle_weapons(vcf_path)),
    )


def parse_vehicle_name(vdf_path: Path) -> str:
    """Read the retail display name from the VDFC header chunk."""
    data = vdf_path.read_bytes()
    offset = data.find(b"VDFC")
    if offset < 0:
        raise ValueError(f"{vdf_path.name}: VDFC section not found")
    section_size = struct.unpack_from("<I", data, offset + 4)[0]
    payload = data[offset + 8 : offset + section_size]
    if not payload:
        raise ValueError(f"{vdf_path.name}: empty VDFC section")
    return payload.split(b"\0", 1)[0].decode("ascii").strip()


def _tmt_vqm_name(data: bytes) -> str | None:
    if len(data) < 72:
        return None
    name = data[64:72].split(b"\0", 1)[0].decode("ascii").strip()
    return name or None


def resolve_material_vqm(
    store: AssetStore,
    material: str,
    vehicle_prefix: str,
    appearance: int = 1,
) -> tuple[str, bytes] | None:
    if not material:
        return None
    direct = store.find(material + ".vqm")
    if direct is not None:
        return direct.name, store.payload(direct)

    material_path = Path(material)
    if material_path.suffix.lower() == ".map":
        direct = store.find(material_path.stem + ".vqm")
        if direct is not None:
            return direct.name, store.payload(direct)

    if material_path.suffix.lower() == ".tmt":
        tmt = store.named_payload(material_path.name)
        if tmt is None:
            standalone_tmt = store.root / material_path.name
            if standalone_tmt.is_file():
                tmt = standalone_tmt.read_bytes()
        if tmt is not None:
            vqm_stem = _tmt_vqm_name(tmt)
            if vqm_stem:
                vqm_name = vqm_stem + ".vqm"
                payload = store.named_payload(vqm_name)
                if payload is not None:
                    return vqm_name, payload

    normalized = normalized_name(material)
    body = re.fullmatch(r"v(\d+)body", normalized)
    if body:
        vqm_name = f"{vehicle_prefix.lower()}11bdy1.vqm"
        payload = store.named_payload(vqm_name)
        return (vqm_name, payload) if payload is not None else None

    mapped = re.fullmatch(r"v(\d+)([a-z0-9]{2})([a-z0-9]{2})", normalized)
    if mapped:
        bank, section, side = mapped.groups()
        if side == "lf":
            side = "lt"
        tmt_name = (
            f"{appearance}{vehicle_prefix.upper()}{bank}"
            f"{section.upper()}{side.upper()}.tmt"
        )
        tmt = store.named_payload(tmt_name)
        if tmt is None:
            standalone_tmt = store.root / tmt_name
            if standalone_tmt.is_file():
                tmt = standalone_tmt.read_bytes()
        if tmt is not None:
            vqm_stem = _tmt_vqm_name(tmt)
            if vqm_stem:
                vqm_name = vqm_stem + ".vqm"
                payload = store.named_payload(vqm_name)
                if payload is not None:
                    return vqm_name, payload
    return None


def decode_vqm(
    root: Path,
    data: bytes,
    palette_name: str = "p02.act",
    transparent_palette_index: int | None = None,
) -> tuple[int, int, bytes, str]:
    if len(data) < 24:
        raise ValueError("VQM is shorter than its header")
    width, height = struct.unpack_from("<II", data)
    codebook_name = data[8:24].split(b"\0", 1)[0].decode("ascii")
    block_width = math.ceil(width / 4)
    block_height = math.ceil(height / 4)
    block_count = block_width * block_height
    if len(data) != 24 + block_count * 2:
        raise ValueError(
            f"VQM dimensions require {block_count} blocks, "
            f"but payload length is {len(data)}"
        )
    codebook_path = root / codebook_name.lower()
    codebook = codebook_path.read_bytes()
    if len(codebook) < 4:
        raise ValueError(f"truncated codebook: {codebook_path}")
    codebook_entry_count = struct.unpack_from("<I", codebook)[0]
    if len(codebook) != 4 + codebook_entry_count * 16:
        raise ValueError(f"unexpected codebook size: {codebook_path}")
    palette = (root / palette_name).read_bytes()
    if len(palette) != 256 * 3:
        raise ValueError(f"unexpected ACT palette size: {palette_name}")

    indices = struct.unpack_from(f"<{block_count}H", data, 24)
    result = bytearray(width * height * 4)
    blocks = memoryview(codebook)[4:]
    for block_y in range(block_height):
        for block_x in range(block_width):
            token = indices[block_y * block_width + block_x]
            solid_palette_index = token & 0xFF
            if not token & 0x8000 and token >= codebook_entry_count:
                raise ValueError(
                    f"{codebook_name}: VQM token {token} exceeds "
                    f"{codebook_entry_count} entries"
                )
            block = (
                None
                if token & 0x8000
                else blocks[token * 16 : (token + 1) * 16]
            )
            for pixel_y in range(4):
                y = block_y * 4 + pixel_y
                if y >= height:
                    continue
                for pixel_x in range(4):
                    x = block_x * 4 + pixel_x
                    if x >= width:
                        continue
                    palette_index = (
                        solid_palette_index
                        if block is None
                        else block[pixel_y * 4 + pixel_x]
                    )
                    source = palette_index * 3
                    target = (y * width + x) * 4
                    result[target : target + 3] = palette[source : source + 3]
                    result[target + 3] = (
                        0
                        if transparent_palette_index == palette_index
                        else 255
                    )
    return width, height, bytes(result), codebook_name
