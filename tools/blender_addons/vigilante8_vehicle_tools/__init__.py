"""Vigilante 8 native vehicle tools for Blender.

The format modules are importable without Blender, which keeps retail
round-trip tests independent of a particular Blender installation.
"""

from __future__ import annotations

bl_info = {
    "name": "Vigilante 8 Native Vehicle Tools",
    "author": "Vigilante 8 Recompilation Project",
    "version": (0, 3, 1),
    "blender": (4, 2, 0),
    "location": "File > Import-Export and Scene Properties",
    "description": "Edit native V8 and V8: 2nd Offense vehicle assets",
    "category": "Import-Export",
}


try:
    import bpy  # type: ignore
except ImportError:
    bpy = None


def register() -> None:
    if bpy is None:
        raise RuntimeError("The add-on can only be registered inside Blender")
    from . import authored_ui

    authored_ui.register()


def unregister() -> None:
    if bpy is None:
        return
    from . import authored_ui

    authored_ui.unregister()
