# Application icon

`v8-icon.png` is the generated V8 emblem based on the game's title logo,
with orange/red metal lettering, gold edging and the original flame motif.
The background is transparent, with real PNG alpha preserved in all icon sizes.

`v8.ico` packages 16, 24, 32, 48, 64, 128 and 256 pixel Windows icons.
`window-icon-64.rgba` supplies tightly packed RGBA pixels for the native
window icon. Both derive from the PNG using Pillow's Lanczos resizing:

```python
from pathlib import Path
from PIL import Image
p = Path('tools/recompone-v8-2/reference-host/assets')
im = Image.open(p / 'v8-icon.png').convert('RGBA')
im.save(p / 'v8.ico', sizes=[(n, n) for n in (16, 24, 32, 48, 64, 128, 256)])
(p / 'window-icon-64.rgba').write_bytes(
    im.resize((64, 64), Image.Resampling.LANCZOS).tobytes())
```

The host project embeds the ICO into its Windows executable and the RGBA
resource into its assembly. The shared window host uses the optional
`RecompOne.WindowIcon.64.rgba` resource without game-specific branches.
Keep these assets: they are application artwork, not proof screenshots.
