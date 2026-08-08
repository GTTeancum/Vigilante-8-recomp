"""Extract one exact video frame through Blender's bundled FFmpeg decoder."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import bpy


def arguments() -> argparse.Namespace:
    raw = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("video", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--frame", type=int, default=1)
    return parser.parse_args(raw)


def main() -> None:
    args = arguments()
    video = args.video.resolve()
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    clip = bpy.data.movieclips.load(str(video), check_existing=False)
    width, height = clip.size
    if width <= 0 or height <= 0:
        raise RuntimeError(f"Could not resolve video dimensions: {video}")

    scene = bpy.context.scene
    scene.render.resolution_x = width
    scene.render.resolution_y = height
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(output)
    scene.render.film_transparent = False
    scene.use_nodes = True

    nodes = scene.node_tree.nodes
    nodes.clear()
    movie = nodes.new("CompositorNodeMovieClip")
    movie.clip = clip
    composite = nodes.new("CompositorNodeComposite")
    scene.node_tree.links.new(movie.outputs["Image"], composite.inputs["Image"])

    scene.frame_set(max(1, args.frame))
    bpy.ops.render.render(write_still=True)
    print(f"{video} frame={args.frame} {width}x{height} -> {output}")


if __name__ == "__main__":
    main()
