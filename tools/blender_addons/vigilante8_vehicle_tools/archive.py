"""High-level editable vehicle archive facade."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Iterator

from . import iff, xobf
from .binary import FormatError


@dataclass
class VehicleEntry:
    index: int
    form: iff.IffChunk
    bin_chunk: iff.IffChunk
    animation_chunk: iff.IffChunk | None
    sound_chunk: iff.IffChunk | None

    def model(self, dialect: str | None = None) -> xobf.Model:
        return xobf.Model(self.bin_chunk.payload, dialect)

    def commit_model(self, model: xobf.Model) -> None:
        self.bin_chunk.payload = model.data

    def animation(self, slot_count: int | None = None) -> xobf.AnimationBank | None:
        if self.animation_chunk is None:
            return None
        if slot_count is None:
            slot_count = self.model().slot_count
        return xobf.AnimationBank(self.animation_chunk.payload, slot_count)

    def commit_animation(self, animation: xobf.AnimationBank) -> None:
        if self.animation_chunk is None:
            raise ValueError("vehicle entry has no ANM chunk")
        self.animation_chunk.payload = animation.data


class VehicleArchive:
    def __init__(self, data: bytes):
        self.document = iff.parse(data)
        self._entries = []
        for form in self.document.forms(b"XOBF"):
            bins = [child for child in form.children if child.tag == b"BIN "]
            if not bins:
                continue
            animations = [child for child in form.children if child.tag == b"ANM "]
            sounds = [child for child in form.children if child.tag == b"SND "]
            self._entries.append(
                VehicleEntry(
                    index=len(self._entries),
                    form=form,
                    bin_chunk=bins[0],
                    animation_chunk=animations[0] if animations else None,
                    sound_chunk=sounds[0] if sounds else None,
                )
            )
        if not self._entries:
            raise FormatError("EXP archive contains no XOBF/BIN vehicle entries")

    @property
    def data(self) -> bytes:
        return self.document.to_bytes()

    def entry(self, index: int) -> VehicleEntry:
        return self._entries[index]

    def entries(self) -> Iterator[VehicleEntry]:
        yield from self._entries

    @classmethod
    def read(cls, path: str | Path) -> "VehicleArchive":
        return cls(Path(path).read_bytes())

