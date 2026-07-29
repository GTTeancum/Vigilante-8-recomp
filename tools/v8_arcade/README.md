# Vigilante 8 Arcade vehicle extraction

This directory contains the static extraction path for the Xbox 360 Arcade
release. The game does not need to boot.

## Complete Blender export

`extract_vehicle_blends.py` creates one `.blend` for each authored Xbox 360
skin and a separate `.blend` for each PS1 classic. It reconstructs
scene/node transforms, decodes indices and exact observed vertex layouts,
creates UV and normal attributes, recovers D3DX shader parameters, and
decodes Xbox 360 tiled textures. Referenced images are packed into each
Blender file and retained as PNGs beside the export.

The five native scenes in each standard bundle are full-resolution skin
variants, not LODs. Their geometry counts are identical, and each scene-table
RGB matches the material's decoded `paintColor0`. FartyDog has one native
scene; Saucer has five. The eight standard cars also have a compact final PS1
classic scene. FartyDog and Saucer have no classic.

Output is organized as:

```text
vehicle_blends_corrected/
  Piranha/
    Piranha_360_skin_01_4D6680.blend
    Piranha_360_skin_02_333333.blend
    ...
    Piranha_PS1_classic.blend
    Piranha_auxiliary_unresolved.blend
    manifest.json
```

Every skin/classic file contains one visible `VEHICLE_ASSEMBLY` collection.
The five skins for each of the eight standard Xbox 360 vehicles also contain
a hidden `XBOX360_DAMAGE_ASSEMBLY` collection reconstructed from the
non-Scene secondary part arrays. Toggle that collection on and
`VEHICLE_ASSEMBLY` off to inspect the destroyed-state geometry by itself.
Parts are named from geometry, placement, and repeated instances (for example
`body_shell`, `door_left`, `hood`, and `wheel_front_right`). The stripped
bundles do not retain authored part strings, so every inferred name carries a
`part_name_confidence` and `part_name_basis` custom property.

The per-vehicle auxiliary file is intentional. It retains every valid
`Model3` not reached through an authored skin/classic scene or the recovered
secondary arrays, so damage pieces, detachable parts, collision/shadow
geometry, or other assets are never silently discarded. Across the ten
bundles, the skin/classic files plus auxiliary files cover all 678 discovered
`Model3` allocations.

The numbered lower-complexity native candidates are not distance LODs. Their
Nodes belong to additional 13-18-entry serialized damage-part arrays outside
the public Scene table. They have missing or displaced panels, use a generic
grime/damage material without `paintColor0`, and are retained only as
`XBOX360_DAMAGE_ASSEMBLY`. Static XEX inspection also shows that serialized
`Model3` exposes only bounds and submeshes, with no LOD pointer or chain. The
five intact native Scenes for each standard vehicle are equal-complexity skin
variants. No separate intact Xbox 360 distance LOD has been recovered.

Material nodes use recovered compiled-shader names:

- `diffuse_samp` RGB alone drives Base Color.
- Diffuse alpha is intentionally ignored; it is a paint mask in the Xbox
  shader, not general mesh opacity, and using it as Blender transparency
  produces black dither noise.
- The inspection material is opaque and matte. It does not connect
  `paintColor0`, normal, gloss, clearcoat, environment, shadow, or microflake
  effects.
- Those recovered resources remain packed as labeled, unconnected image nodes
  and custom metadata for later renderer/back-port work.

Run through Blender 4.5 or newer:

```powershell
$vehicles = 'FartyDog','GrooVan','Incarcerator','Jefferson','Leprechaun',
  'Mammoth','Manta','Piranha','Saucer','Stag' |
  ForEach-Object { "V8_ARCADE_WORK\decompressed\$_.ib" }

blender --background --factory-startup `
  --python tools\v8_arcade\extract_vehicle_blends.py `
  -- $vehicles --out V8_ARCADE_WORK\vehicle_blends_corrected
```

Validate UV layouts, finite UV values, part metadata, shader wiring, packed
images, and every generated database:

```powershell
$blends = Get-ChildItem V8_ARCADE_WORK\vehicle_blends_corrected `
  -Filter *.blend -Recurse

blender --background --factory-startup `
  --python tools\v8_arcade\validate_vehicle_blends.py `
  -- $blends.FullName `
  --out V8_ARCADE_WORK\analysis\vehicle_blends_corrected_validation.json
```

Render a neutral Workbench perspective preview:

```powershell
blender --background --factory-startup `
  --python tools\v8_arcade\render_blend_preview.py `
  -- V8_ARCADE_WORK\vehicle_blends_corrected\Piranha\Piranha_360_skin_01_4D6680.blend `
  --collection VEHICLE_ASSEMBLY `
  --out V8_ARCADE_WORK\vehicle_blends_corrected\Piranha\Piranha_preview.png
```

## LOD-only handoff

`export_vehicle_lod_only_blends.py` produces the deliberately minimal
`V8_ARCADE_WORK/vehicle_lods_only` deliverable. It contains only the eight
verified highest authored PS1 classic scenes, each in `PS1_HIGHEST_LOD`.
Xbox 360 files are deliberately not emitted because no separate intact
distance LOD was found; the lowest intact native geometry remains the full
skin Scene.

Validate these isolated databases with `validate_lod_only_blends.py`.

## Package and intermediate extraction

- `extract_stfs.py` lists or extracts the file system in the Xbox LIVE/STFS
  package.
- `decompress_ibz.py` expands size-prefixed zlib `.ibz` files into native
  `ISOP` `.ib` images.
- `extract_vehicle_lods.py` exports every position/index stream to OBJ and
  writes inventory manifests. It remains useful for format debugging.
- `inspect_vehicle_scenes.py` reports authored scene graphs and transforms.
- `inspect_vehicle_textures.py` reports Xenos texture fetch constants and
  material-to-texture references.
- `inspect_vehicle_materials.py` correlates scene colors with decoded D3DX
  material constants.
- `audit_vertex_layouts.py` records raw UV-layout evidence for a selected
  model.

PowerShell, from the repository root:

```powershell
New-Item -ItemType Directory -Force V8_ARCADE_WORK\rar | Out-Null
tar -xf '.\Vigilante 8 Arcade.rar' -C V8_ARCADE_WORK\rar

$package = Get-ChildItem V8_ARCADE_WORK\rar -Recurse -File |
  Where-Object { (Get-Content -AsByteStream -TotalCount 4 $_.FullName) -join ',' -eq '76,73,86,69' } |
  Select-Object -First 1

python tools\v8_arcade\extract_stfs.py $package.FullName `
  --out V8_ARCADE_WORK\extracted `
  --manifest V8_ARCADE_WORK\analysis\stfs_manifest.json

python tools\v8_arcade\inspect_vehicle_scenes.py `
  V8_ARCADE_WORK\extracted `
  --out V8_ARCADE_WORK\analysis\vehicle_scenes.json

python tools\v8_arcade\inspect_vehicle_textures.py `
  V8_ARCADE_WORK\extracted `
  --out V8_ARCADE_WORK\analysis\vehicle_textures.json
```

See [v8_arcade_isop.md](../../notes/formats/v8_arcade_isop.md) for recovered
binary layouts and confidence notes.

## ReXGlue analysis

The persistent ReXGlue seed is
`tools/v8_arcade/rexglue/v8_arcade_manifest.toml`. ReXGlue 0.8.0 translates
the supplied `default.xex` after STFS extraction:

```powershell
Push-Location tools\v8_arcade\rexglue
rexglue codegen v8_arcade_manifest.toml
Pop-Location
```

Generated C++ is kept in ignored `V8_ARCADE_WORK` because it is reproducible
and tied to the user-supplied executable. The `xex_image_dump/` helper uses
the ReXGlue runtime in tool mode to load the XEX and dump its mapped image and
sections for string/reflection analysis. `shader_inspector/` reads the
compiled Xbox shader containers used to recover vertex semantics.
