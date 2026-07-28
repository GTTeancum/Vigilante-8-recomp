# Vigilante 8 Native Vehicle Tools

Blender 4.2+ add-on for creating and editing independent native vehicles for
both Vigilante 8 games. V8: 2nd Offense is the lead target; Vigilante 8 uses
the same strict project format with the sequel-only fields omitted.

The production workflow does not use a retail vehicle as a donor, replace a
roster entry, embed a source archive, or retain opaque binary records. Every
native byte is rebuilt from named project fields.

## Native output

The compiler writes the formats the games already use:

- EA-IFF `CUSTOM.EXP` with owned `FORM XOBF` entries;
- `BIN` render groups, slots, hierarchy, collision, and PS1 textures;
- V8 and widened V8:2 polygon packets;
- V8:2 native 2 KiB-window texture compression;
- native `ANM` animation streams;
- append-only `VEHICLES.V8R` identity, stat, powerup, and transformation data.

No glTF, FBX, PNG, or other modern runtime format is introduced.

## Install

Install `vigilante8_vehicle_tools-0.3.1.zip` through **Edit > Preferences >
Add-ons > Install from Disk**. The controls appear under **Scene Properties >
Vigilante 8 Native Vehicles** and in Blender's File menus.

## Create an independent vehicle

1. Choose **V8: 2nd Offense** or **Vigilante 8**.
2. Press **New Original Vehicle**.
3. Give the collection a unique lowercase stable ID and a display name.
4. Edit its meshes, object-slot empties, collision objects, textures,
   animation records, and vehicle-data panel.
5. Export a strict JSON project for source control.
6. Choose **Compile Native Vehicle Package** to write `CUSTOM.EXP` and
   `VEHICLES.V8R`.

The generated starter is structurally complete and owns its body mesh,
collision, attachment hierarchy, stats, and wheel object. The V8:2 starter
also owns transformation objects for Hover, Float, and Ski modes and contains
all five vehicle powerup values.

Multiple authored collections for the same game compile into one package.
Stable identity is independent of archive order. The engines append custom
vehicle types instead of consuming or overwriting a retail slot.

## Import an authored native package

Choose **File > Import > Vigilante 8 Native Vehicle Package** and select
`VEHICLES.V8R`; the importer reads the sibling `CUSTOM.EXP` automatically.
Every entry becomes an editable Blender collection. Re-exporting an unchanged
package is byte-exact.

Import is deliberately strict. It accepts native packages produced by this
authoring system only when every model, texture, animation, stat, powerup, and
transformation record can be decoded into named fields. It never preserves a
source archive, passthrough chunk, donor record, or opaque payload.

## Scene representation

All data remains decoded and editable:

- render groups are Blender mesh objects with native scale shifts;
- native normal tables remain ordered, decoded mesh datablocks owned by their
  render objects; they are shading directions and are not drawn as point
  clouds or inserted into the object hierarchy;
- face materials carry exact native color and texture identity; native
  environment packets identify their bank texture or the engine-global
  `0x3FFE` gloss/`0x3FFF` arena-reflection source;
- UV layers carry native 0–255 coordinates;
- slots are parented empties with render class, attachment key, collision
  reference, flags, fixed-point position, and PS1 rotation;
- AABB and convex collision records are named objects and plane children;
- textures are Blender images with explicit native storage: 4-bit or 8-bit
  indexed BGR555 palettes, or 16-bit direct-color BGR555 pixels;
- animation streams, frames, texture bindings, scales, and mesh vectors are
  named objects with editable properties;
- stats, V8:2 powerups, and four six-wheel transformation modes are collection
  properties exposed in the Scene panels.

Retail meshes can store the same triangle twice as ordered render passes: an
environment or color base followed by an indexed diffuse/decal texture. The
PS1 packet stream makes the visible surface and gloss layer coexist. Blender
cannot reliably depth-sort the two coplanar faces, so the importer identifies
each exact pair and hides only its environment polygon in the default view.
The hidden polygon remains decoded and editable (`Alt-H` in Edit Mode), and
export always includes both native packets without changing their coordinates
or order. Ordinary textured surfaces, including wheels, are not blended.

Native BGR555 and packet colors are converted to Blender's linear working
space and displayed with unlit authoring materials, so Material Preview shows
the palette rather than tinting it with Blender's studio environment. Decoded
native corner normals are applied to the visible mesh as custom loop normals;
the independently owned normal table remains available for exact editing and
export.

Some retail CLUT rectangles contain more uploaded entries than their pixel
depth can address. The importer preserves every native CLUT entry, while
edited 4-bit pixels can select only entries 0–15 and edited 8-bit pixels only
0–255, matching the PS1 packet format. V8:2's depth-2 records are editable
16-bit direct-color images rather than opaque renderer data; unchanged values
round-trip exactly and edited pixels quantize back to native BGR555.

The Outliner keeps only the two genuine native archive boundaries:
`Body Object Bank (native XOBF)` and, for the sequel,
`Transformation Object Bank (native XOBF)`. The native slot parent/child tree
is shown directly below those collections; semantic category buckets are not
inserted. A render-bearing native slot is the editable mesh object itself,
not an empty pointing to a second “model” object. Placement, weapon, wheel,
effect, and transform-only slots remain named nulls. Each mesh object has its
own Blender mesh datablock. If native objects originally shared one model,
unchanged copies remain one native model record; editing a copy independently
causes export to emit a new native model record and update only that object's
reference. An edit to one apparent object therefore never silently changes
another.

## Authoring view

A newly imported vehicle opens in a clean `Intact` view. Mutually exclusive
damage replacements, collision and animation records, debris, LOD and upgrade
models, attachment guides, and the V8:2 transformation library remain present
for export but are hidden from the normal modeling view. `Native Object
Hierarchy > Authoring View` can reveal each category, show named attachment
guides, or select a different body-damage state. These are viewport-only
controls; they never change the literal hierarchy or compiled package.
V8:2 also shows a textured, assembled Standard-wheel preview by default.
The `Mounted wheel mode` selector swaps that preview among Standard, Hover,
Float, Ski, and body-only views. Each preview copy owns its mesh data, is
locked against accidental editing, is mounted at the actual body wheel
anchor, and is excluded from export. Explanations are carried by the compact
Scene-property controls as hover tooltips instead of permanent viewport text.

## Automatic collision

Authors do not need to build or manipulate Blender collision objects. Export
calculates conservative native AABB primitives from the intact render-mesh
hierarchy in each collision owner's local space. A newly created original
vehicle always regenerates those boxes. An imported retail or custom vehicle
keeps its decoded native collision byte-exactly until the corresponding mesh
bounds change; export then replaces that collision stream with the calculated
box automatically. Manual convex planes and collision records remain
available only through `Advanced engine objects` for expert work.

## Vehicle stats

Every authorable stat shows a plain-language explanation, native unit or
scale, hard binary storage bound, safe authoring bound where physics makes the
full binary range unsafe, retail-tested range, and the effect of lower and
higher values. Spring, damping, and angular-response controls reject negative
coefficients; mass and health reject zero even though their integer storage
could encode those nonphysical values.

V8:2 repair amount and all four duration settings use the same treatment:
unit, safe non-negative MIPS-immediate bound, retail value, and explicit
lower/higher behavior. Object selectors and bit masks explicitly say that
their numeric ordering is not a strength scale; lower/higher means selecting
a different object or clearing/setting particular bits.

For example, spring stiffness is the force pushing compressed suspension back
toward ride height. Higher stiffness makes the axle firmer. Suspension
damping resists suspension movement and stops repeated bouncing after a bump;
higher damping settles faster, while lower damping allows more bounce.

Unknown values are preserved as individually named native fields but disabled
in the normal authoring UI. They are not exposed as generic “handling”
controls. JSON exports use the semantic names shown in Blender, including
`front_spring_stiffness`, `speed_drag_coefficient`,
`yaw_angular_response`, and `mass`.

V8:2 has one unusual native overlap which the UI exposes explicitly: record
`+0x08` is both front damping and rear spring stiffness. Its retail rear
damping word is formed from the enabled-wheels byte at `+0x0C` and the
engine-assigned vehicle type at `+0x0D`. Authored packages therefore store an
explicit `rear_suspension_damping` extension in registry version 3. The V8:2
runtime reapplies the authored spring and damping coefficients to all three
wheel pairs after the native constructor, so rear damping no longer depends
on roster identity. Retail records retain their original overlap unchanged.

JSON loading rejects unknown fields. Unsupported quest, unlock, and sound
records are rejected until those native structures are decoded; arbitrary
payloads are never accepted as a substitute.

## Game differences

V8 projects contain the shared body-bank, collision, animation, texture, and
stat data. They reject V8:2 transformation banks, transformation modes, and
per-vehicle powerup profiles.

V8:2 projects require:

- six wheel attachment identities;
- an independently owned transformation bank;
- Standard, Hover, Float, and Ski wheel-object tables;
- repair, weapon-upgrade, radar-jammer, shield, and transformation values.

The transformation panel never asks an author for a native object number.
Each Hover, Float, and Ski row names the replacement object used at Front
Left, Front Right, Middle Left, Middle Right, Rear Left, and Rear Right. The
name selects and frames that object; **Use Selected** assigns the selected
top-level transformation object. Standard mode explicitly uses the six normal
wheel anchors and has no replacement object. Transformation objects also
carry a readable assignment summary in their custom properties.

This keeps one authoring system while making V8:2 the richer lead
implementation.

The packaged `conversion` module also supplies the decoded-data operations
used by the reproducible V8-to-V8:2 guest build: packet-dialect translation,
complete owned-root extraction, bank merging, V8:2's additional flamethrower
mount, geometry-bound standard-wheel matching, and stat translation. These
operations consume and return strict editable project objects; they do not
accept a retail roster slot, source-archive passthrough, or opaque payload.

## Native object hierarchy and attachment points

The Outliner and Native Object Hierarchy panel mirror the engine's literal
parent/child structure. A render-bearing native object is the Blender mesh
itself; placement-only records are empties. Mesh datablocks are unique, so
editing one object never silently changes another native object that happened
to share retail geometry.

Names come from the engine path that consumes each record:

- `Front/Rear Body - Intact`, `Damaged`, `Heavily Damaged`, and `Destroyed`
  describe the actual replacement chain;
- `Vehicle Distance LOD` is the class-C distance replacement;
- `HUD Vehicle Icon Texture Source` is the class-B planar source;
- acceleration, top-speed, armor, and handling upgrade appearances use their
  exact V8:2 upgrade roles;
- destruction debris models, collision volumes, particle emitters, sound
  emitters, and effect origins remain children of the damage state that owns
  them;
- the special weapon is shown as Mount, Base, Articulated Section, Emitter,
  and its placement pivots;
- wheel, machine-gun, rocket, missile, cannon, mortar, mine-dripper,
  flamethrower, headlight, tail-light, auxiliary, and destruction anchors use
  their runtime roles.

V8 stores an eight-bit model field plus an eight-bit object class. V8:2 uses
an eleven-bit model field plus five class bits. Class-8 and class-9 records
are effect/sound opcodes whose low bits are parameters, not models; the
importer therefore creates named empties for them even when those bits happen
to equal a valid model number. Class-B, C, D, E, and F records retain their
owned model where the engine actually resolves one.

Native indices are internal export identities and are not shown as object
names or as the selected-object identity in the authoring panel. Exact keys,
classes, role IDs, transforms, hierarchy links, render ownership, collision
ownership, inferred role, and confidence remain advanced metadata for
diagnosis. Renaming an object does not change its export identity.

## V8:2 transformation semantics

The sequel does not bone-deform or vertex-morph the main body. Its generic
vehicle code reads a per-vehicle table containing four modes by six wheel
positions. Standard uses the body's normal wheel roots; Hover, Float, and Ski
select root object indices from the independently owned transformation bank.
The engine swaps those wheel-root submodels and interpolates their ordinary
slot translation/rotation during the transition.

The bank's stored coordinates are library data, not an assembled-on-vehicle
preview. Retail tables can select the same root for several wheel positions
(for example one Hover-left root for the front, middle, and rear positions),
so the raw root offset cannot be interpreted as the final mount point. The
engine applies the selected replacement through each vehicle wheel object and
its anchor. Blender therefore hides the raw library by default and labels its
optional diagnostic view explicitly.

The mounted preview follows the decoded sequel routines rather than visual
alignment. Wheel update `0x8003E4A8` selects the root index and copies its
three rotation fields at native slot offsets `0x10` through `0x14`; it does
not apply the library root position at `0x04` through `0x0C`. Standard mode
writes zero rotation for even/left positions and native Y rotation `2048` for
odd/right positions. Object construction `0x8002C17C` treats signed-negative
children such as `0xC000` LOD records as traversal controls. The wheel-child
call also sets construction flag `0x20`, which skips child object IDs above
`0x00FF` after a signed 16-bit cast. The retail positive `0x0100` children are
therefore not additional wheels, while `0xAAAA` is the valid signed value
`-21846` and its hover/float/ski subassemblies are constructed. Those records
remain editable in the raw bank, with only the engine-skipped records omitted
from the mounted preview.

The importer names every referenced root by mode and placement, for example
`Hover - Front Left Wheel`, while retaining table identities only
as internal export metadata. Native ANM flag `0x10` records are
texture-animation bindings (`render target`, `texture index`), not skeleton
channels. Mesh-vector animation records remain decoded separately.

This makes both directions explicit:

- a V8 vehicle can gain sequel transformations by supplying six compatible
  wheel anchors, a transformation bank, and the four-mode table;
- a V8:2 vehicle can compile for V8 after sequel-only powerup and
  transformation data is intentionally omitted or mapped. The base model,
  collision, textures, animations, attachments, and shared stats do not need
  a donor vehicle.

## Native memory contract

The compiler rejects banks above the decoded retail-derived limits: 256 native
textures, 262,144 texels, 128 KiB of indexed texture/palette data, or 256 KiB
of BIN plus ANM source data per bank. A selected V8 vehicle therefore expands
to at most 1 MiB of RGBA texture data; a V8:2 body plus transformation bank
uses at most 2 MiB. The Blender panel reports both native and expanded texture
costs while editing.

Engine registries scan `CUSTOM.EXP` as file-backed native XOBF slices and read
only a selected vehicle's owned bank(s). Adding roster entries increases
package storage, not steady-state vehicle RAM. No glTF, PNG, or other runtime
conversion is introduced.

At match load, each engine derives the selected bank's exact native PS1 CLUT
and image allocation requests. It reserves those rectangles before COMMON and
arena banks consume VRAM, then hands each reservation to the original texture
loader only while that bank's decoded texture source is active. V8:2 retains
the transformation-bank reservations until the corresponding mode first
loads; it does not preload or borrow retail textures. V8 uses the same
mechanism around its earlier allocator and single body bank. This preserves
the games' native texture descriptors while preventing a late guest load from
depending on spare fragmented VRAM.

## Validation

From the repository root:

```powershell
python -m unittest tools.blender_addons.tests.test_native_formats -v
& 'C:\Program Files\Blender Foundation\Blender 4.5\blender.exe' `
  --background --factory-startup `
  --python tools\blender_addons\tests\blender_integration.py
```

The native tests compile packages without source data. The Blender integration
test creates both V8 and V8:2 vehicles from scratch, compiles them, imports the
native packages into new editable scenes, and verifies byte-exact re-export.
It also asserts that no authored or imported scene contains embedded source
text or opaque passthrough data.
