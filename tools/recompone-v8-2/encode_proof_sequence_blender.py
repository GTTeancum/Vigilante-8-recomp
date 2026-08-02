"""Encode an ordered PPM/BMP proof sequence with Blender's bundled FFmpeg.

Usage:
  blender --background --python encode_proof_sequence_blender.py -- \
      INPUT_GLOB OUTPUT_MP4 FPS
"""

from __future__ import annotations

import glob
import os
import struct
import sys
import tempfile

import bpy


def read_ppm_dimensions(path: str) -> tuple[int, int]:
    with open(path, "rb") as stream:
        if stream.readline().strip() != b"P6":
            raise ValueError(f"not a binary P6 PPM: {path}")
        dimensions = stream.readline().split()
        while dimensions and dimensions[0].startswith(b"#"):
            dimensions = stream.readline().split()
        if len(dimensions) != 2:
            raise ValueError(f"invalid PPM dimensions: {path}")
        width, height = map(int, dimensions)
        if stream.readline().strip() != b"255":
            raise ValueError(f"only 8-bit PPM input is supported: {path}")
    return width, height


def ppm_to_bmp(input_path: str, output_path: str) -> tuple[int, int]:
    with open(input_path, "rb") as stream:
        if stream.readline().strip() != b"P6":
            raise ValueError(f"not a binary P6 PPM: {input_path}")
        dimensions = stream.readline().split()
        while dimensions and dimensions[0].startswith(b"#"):
            dimensions = stream.readline().split()
        width, height = map(int, dimensions)
        if stream.readline().strip() != b"255":
            raise ValueError(f"only 8-bit PPM input is supported: {input_path}")
        rgb = stream.read(width * height * 3)

    if len(rgb) != width * height * 3:
        raise ValueError(f"truncated PPM pixel payload: {input_path}")

    source_stride = width * 3
    output_stride = (source_stride + 3) & ~3
    pixel_bytes = output_stride * height
    header = bytearray(54)
    header[0:2] = b"BM"
    struct.pack_into("<I", header, 2, len(header) + pixel_bytes)
    struct.pack_into("<I", header, 10, len(header))
    struct.pack_into("<I", header, 14, 40)
    struct.pack_into("<i", header, 18, width)
    struct.pack_into("<i", header, 22, height)
    struct.pack_into("<H", header, 26, 1)
    struct.pack_into("<H", header, 28, 24)
    struct.pack_into("<I", header, 34, pixel_bytes)

    padding = b"\0" * (output_stride - source_stride)
    with open(output_path, "wb") as stream:
        stream.write(header)
        for y in range(height - 1, -1, -1):
            start = y * source_stride
            source = rgb[start : start + source_stride]
            bgr = bytearray(source_stride)
            bgr[0::3] = source[2::3]
            bgr[1::3] = source[1::3]
            bgr[2::3] = source[0::3]
            stream.write(bgr)
            stream.write(padding)
    return width, height


def main() -> None:
    args = sys.argv[sys.argv.index("--") + 1 :]
    if len(args) != 3:
        raise SystemExit(
            "usage: encode_proof_sequence_blender.py INPUT_GLOB OUTPUT_MP4 FPS"
        )

    input_glob, output_path, fps_text = args
    paths = sorted(glob.glob(input_glob))
    if not paths:
        raise SystemExit(f"no input frames matched: {input_glob}")

    fps = int(fps_text)
    if fps <= 0:
        raise SystemExit("FPS must be positive")

    width, height = read_ppm_dimensions(paths[0])
    with tempfile.TemporaryDirectory(prefix="recompone_proof_") as temp_dir:
        sequence_paths = paths
        if os.path.splitext(paths[0])[1].lower() == ".ppm":
            sequence_paths = []
            for index, path in enumerate(paths):
                converted = os.path.join(temp_dir, f"proof_{index:06d}.bmp")
                converted_size = ppm_to_bmp(path, converted)
                if converted_size != (width, height):
                    raise ValueError(
                        f"frame size changed at {path}: {converted_size}"
                    )
                sequence_paths.append(converted)

        scene = bpy.context.scene
        scene.sequence_editor_clear()
        editor = scene.sequence_editor_create()
        strips = getattr(editor, "strips", None)
        if strips is None:
            strips = editor.sequences

        strip = strips.new_image(
            "proof",
            os.path.abspath(sequence_paths[0]),
            channel=1,
            frame_start=1,
        )
        for path in sequence_paths[1:]:
            strip.elements.append(os.path.basename(path))
        strip.frame_final_duration = len(sequence_paths)

        scene.render.resolution_x = width
        scene.render.resolution_y = height
        scene.render.resolution_percentage = 100
        scene.render.fps = fps
        scene.frame_start = 1
        scene.frame_end = len(sequence_paths)
        scene.render.filepath = os.path.abspath(output_path)
        scene.render.image_settings.file_format = "FFMPEG"
        scene.render.ffmpeg.format = "MPEG4"
        scene.render.ffmpeg.codec = "H264"
        scene.render.ffmpeg.constant_rate_factor = "MEDIUM"
        scene.render.ffmpeg.ffmpeg_preset = "GOOD"
        scene.render.ffmpeg.audio_codec = "NONE"
        scene.render.use_file_extension = True
        scene.render.use_sequencer = True

        bpy.ops.render.render(animation=True)
    print(
        f"encoded {len(paths)} frames at {width}x{height}, "
        f"{fps} fps -> {scene.render.filepath}"
    )


if __name__ == "__main__":
    main()
