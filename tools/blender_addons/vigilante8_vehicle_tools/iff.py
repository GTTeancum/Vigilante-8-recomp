"""Lossless EA-IFF/FORM reader and writer used by V8 EXP archives."""

from __future__ import annotations

from dataclasses import dataclass, field
import struct
from typing import Iterator

from .binary import FormatError, require_range, u32be


def _printable_fourcc(tag: bytes) -> bool:
    return len(tag) == 4 and all(0x20 <= value <= 0x7E for value in tag)


@dataclass
class IffChunk:
    """One IFF chunk.

    Unknown leaf payloads, trailing bytes inside forms, and odd-byte padding
    are retained exactly. Re-serializing an untouched document is therefore
    byte-identical to the source file.
    """

    tag: bytes
    payload: bytes = b""
    form_type: bytes | None = None
    children: list["IffChunk"] = field(default_factory=list)
    tail: bytes = b""
    pad: bytes = b""
    source_offset: int = 0

    @property
    def is_form(self) -> bool:
        return self.tag == b"FORM"

    def content_bytes(self) -> bytes:
        if not self.is_form:
            return self.payload
        if self.form_type is None or len(self.form_type) != 4:
            raise FormatError("FORM chunk is missing its four-byte form type")
        return (
            self.form_type
            + b"".join(child.to_bytes() for child in self.children)
            + self.tail
        )

    def to_bytes(self) -> bytes:
        content = self.content_bytes()
        pad = self.pad if len(content) & 1 else b""
        if len(content) & 1 and len(pad) != 1:
            pad = b"\0"
        return self.tag + struct.pack(">I", len(content)) + content + pad

    def walk(self) -> Iterator["IffChunk"]:
        yield self
        for child in self.children:
            yield from child.walk()

    def forms(self, form_type: bytes | None = None) -> Iterator["IffChunk"]:
        for node in self.walk():
            if node.is_form and (form_type is None or node.form_type == form_type):
                yield node

    def leaves(self, tag: bytes | None = None) -> Iterator["IffChunk"]:
        for node in self.walk():
            if not node.is_form and (tag is None or node.tag == tag):
                yield node


@dataclass
class IffDocument:
    chunks: list[IffChunk]
    trailing: bytes = b""

    def to_bytes(self) -> bytes:
        return b"".join(chunk.to_bytes() for chunk in self.chunks) + self.trailing

    def walk(self) -> Iterator[IffChunk]:
        for chunk in self.chunks:
            yield from chunk.walk()

    def forms(self, form_type: bytes | None = None) -> Iterator[IffChunk]:
        for node in self.walk():
            if node.is_form and (form_type is None or node.form_type == form_type):
                yield node


def _parse_chunk(data: bytes, offset: int, end: int) -> tuple[IffChunk, int]:
    require_range(data, offset, 8)
    if offset + 8 > end:
        raise FormatError(f"truncated IFF header at 0x{offset:X}")
    tag = data[offset : offset + 4]
    if not _printable_fourcc(tag):
        raise FormatError(f"invalid IFF fourcc {tag!r} at 0x{offset:X}")
    size = u32be(data, offset + 4)
    body = offset + 8
    content_end = body + size
    if content_end > end:
        raise FormatError(
            f"{tag!r} at 0x{offset:X} ends at 0x{content_end:X}, "
            f"past parent end 0x{end:X}"
        )
    next_offset = content_end + (size & 1)
    if next_offset > end:
        raise FormatError(f"missing odd-byte pad after {tag!r} at 0x{offset:X}")
    pad = data[content_end:next_offset]

    if tag != b"FORM":
        return (
            IffChunk(
                tag=tag,
                payload=data[body:content_end],
                pad=pad,
                source_offset=offset,
            ),
            next_offset,
        )

    if size < 4:
        raise FormatError(f"FORM at 0x{offset:X} is too small for a form type")
    form_type = data[body : body + 4]
    if not _printable_fourcc(form_type):
        raise FormatError(f"invalid FORM type {form_type!r} at 0x{body:X}")

    children: list[IffChunk] = []
    cursor = body + 4
    while cursor + 8 <= content_end and _printable_fourcc(data[cursor : cursor + 4]):
        child_size = u32be(data, cursor + 4)
        child_end = cursor + 8 + child_size + (child_size & 1)
        if child_end > content_end:
            break
        child, cursor = _parse_chunk(data, cursor, content_end)
        children.append(child)

    return (
        IffChunk(
            tag=b"FORM",
            form_type=form_type,
            children=children,
            tail=data[cursor:content_end],
            pad=pad,
            source_offset=offset,
        ),
        next_offset,
    )


def parse(data: bytes) -> IffDocument:
    chunks: list[IffChunk] = []
    cursor = 0
    while cursor + 8 <= len(data) and _printable_fourcc(data[cursor : cursor + 4]):
        size = u32be(data, cursor + 4)
        if cursor + 8 + size + (size & 1) > len(data):
            break
        chunk, cursor = _parse_chunk(data, cursor, len(data))
        chunks.append(chunk)
    if not chunks:
        raise FormatError("buffer does not contain an IFF chunk")
    return IffDocument(chunks=chunks, trailing=data[cursor:])


def read(path: str) -> IffDocument:
    with open(path, "rb") as stream:
        return parse(stream.read())

