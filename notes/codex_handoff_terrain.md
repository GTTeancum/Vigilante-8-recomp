# Codex Handoff: Terrain & Object Placement Bug

## What the user sees right now

Screenshot taken at commit HEAD (2026-05-23):
- Orange vehicle is visible at roughly screen-centre
- The vehicle sits on an invisible floor (physics grounding WORKS -- vehicle does not fall)
- Dark angular shapes fill the upper portion of the screen, coming from above
- NO green flat terrain visible anywhere
- The user reports being "under a bunch of objects"

This regressed from a prior working state where:
- Green ZONE heightmap tiles rendered correctly as flat ground
- Vehicle drove on flat terrain
- No 3-D structures were visible (XOBF mesh was disabled)

---

## Coordinate system reference (do not get this wrong)

PSX physics: Y-DOWN (positive Y = deeper underground)
OpenGL renderer: Y-UP (positive Y = higher in the air)
Conversion: `gl_y = -(physics_y / 131072.0f)`

Key heights:
- Terrain surface: `physics_y = 294912` -> `gl_y = -2.25 m`
- TERR_Y_ORIGIN = -2.25 f (the OpenGL Y of BIN raw-Y == 0)
- TERR_SCALE = 1/16 (BIN units to metres)
- BIN vertex transform: `gl_x = bin_x/16 + 608`, `gl_y = -bin_y/16 + (-2.25)`, `gl_z = bin_z/16 + 464`

---

## Two terrain systems

### 1. ZONE heightmap mesh (platform/renderer.c, ~lines 140-200)
- Built from 6 x 64x64 cell chunks covering X=[576..640m], Z=[416..512m]
- Rendered as a GL_TRIANGLES index mesh with a solid green tint
- `g_terr_vao`, `g_terr_idxCount`
- This is what produced the green flat terrain the user was happy with

### 2. XOBF BIN mesh (platform/terrain_mesh.c)
- Loaded from `TERRAIN/OILFIELD.EXP`, first FORM XOBF -> BIN chunk
- 165 bones, 4114 CPU triangles, 12342 GL vertices
- World extents: X=[525..694m], Y=[-83..+123m], Z=[374..554m]
- Geometry includes: flat ground, oil derricks (up to 122m tall), pipes, buildings
- `g_terrainmesh_vao`, `g_terrainmesh_vtx`

### Render decision (renderer.c ~line 371)
```c
if (g_terrainmesh_vao && g_terrainmesh_vtx > 0) {
    // Draw XOBF only
    glDrawArrays(GL_TRIANGLES, 0, g_terrainmesh_vtx);
} else if (g_terr_idxCount > 0) {
    // Fallback: ZONE mesh
    glDrawElements(...);
}
```
**XOBF completely replaces ZONE when loaded. There is no blending or fallback.**

---

## What was changed recently (the commits that broke the visual)

### TERR_Y_ORIGIN corrected: 12.0 -> -2.25
- Previously at +12.0, the XOBF mesh floated 14.25m above the vehicle (above camera
  eye height), so it was invisible. Now it is at the correct height.
- This change made the XOBF mesh suddenly appear at vehicle level.

### Polygon winding fix in tm_parse_bin() (terrain_mesh.c ~line 280)
The fix negates the face normal and emits vertices in A,C,B order instead of A,B,C:
```c
float tnx = ey*fz-ez*fy, tny = ez*fx-ex*fz, tnz = ex*fy-ey*fx;
// ... normalise ...
tnx=-tnx; tny=-tny; tnz=-tnz;   // negate

// Emit A, C, B
vbuf[nvtx++] = {vx[0],vy[0],vz[0],...};  // A
vbuf[nvtx++] = {vx[2],vy[2],vz[2],...};  // C
vbuf[nvtx++] = {vx[1],vy[1],vz[1],...};  // B
```
The stored TmTri normal (used by physics HeightAt) has ny > 0 for upward-facing terrain,
and the GL winding A,C,B is supposed to produce CCW-from-above faces.

The physics HeightAt WORKS (vehicle does not fall), which means at least some upward-facing
ground triangles do pass the `ny >= 0.1` filter.

---

## Root cause hypotheses (priority order)

### Hypothesis 1 (MOST LIKELY): Spawn is still inside 3-D structure geometry

The "industrial complex" was estimated to be X=[588..639m], Z=[432..484m] based on HEAD
chunk object positions. But the XOBF BIN itself has geometry in a larger footprint.

Bone analysis showed:
- bone[60,61,62]: Y=[-2.2..+8.8m], X=[588..639], Z=[432..484] -- main platform
- bone[113]: Y=[-2.2..+33.8m], X=[544..694], Z=[377..551] -- FULL LEVEL span, 33m tall
- bone[129]: Y=[-2.2..+33.8m], X=[525..694], Z=[374..554] -- also full span

Bones 113 and 129 span the ENTIRE level (including the "open" south area at Z=420m)
and reach up to 33m tall. The camera at Y=5.75m is INSIDE these bones' Y range.
The spawn at Z=420m may still be surrounded by XOBF geometry from these wide bones.

**Action: Print the actual triangles that the renderer draws at the camera position
(eye_x=610, eye_y=5.75, eye_z=402). Count how many face the camera vs. face away.
Or: temporarily move spawn to level extreme edges (Z=380m) and see if the view clears.**

### Hypothesis 2: XOBF ground faces are back-face culled (winding still wrong)

`glEnable(GL_CULL_FACE)` is active (renderer.c ~line 298). Default front-face is GL_CCW.

If the flat ground triangles in the BIN have PSX winding that is actually CW in GL space
after the Y-flip AND the A,C,B reorder makes them CW-from-above (back face), they would
be culled. The physics works because TmTri stores triangles regardless of GL winding.

To test: add `glDisable(GL_CULL_FACE)` temporarily before the XOBF draw call. If the
ground suddenly appears, winding is the culprit for the missing ground.

### Hypothesis 3: XOBF ground is dark/unlit and invisible against the dark sky

The XOBF tint is `glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f)` -- no boost.
The BIN vertex colours for ground are typically dark brown/grey (from PSX palette).
The sky/background clear colour may be black or very dark.

The ground IS rendering but is so dark it's indistinguishable from background.
ZONE terrain used a green tint which made it obviously visible.

**Action: Temporarily set XOBF tint to (0.0, 3.0, 0.0) (bright green). If the terrain
becomes visible, colour/lighting is the problem, not geometry/culling.**

### Hypothesis 4: Winding fix is correct for ground but inverts building walls

The building walls visible in the screenshot may be INSIDE faces (camera is inside a
building and sees interior walls, which are front-facing from inside). The exterior walls
of the building (front-face from outside) are culled when viewed from inside.

This would mean the spawn is INSIDE a building, not just near it.

---

## Camera setup (renderer.c ~line 335)

```c
float chase_back = 18.0f, chase_up = 8.0f;
float eye_x = vx - sinf(yaw_rad) * chase_back;
float eye_z = vz - cosf(yaw_rad) * chase_back;
float eye_y = vy + chase_up;   // vy = -2.25m + 8m = 5.75m
// clamped upward if camera is below terrain + 3m
float ctr[3] = { vx, vy + 0.5f, vz };  // looks at vehicle centre
```

Vehicle at spawn: X=610m, Y=-2.25m, Z=420m, yaw=0 (facing +Z)
Camera: eye=(610, 5.75, 402), looking toward (610, -1.75, 420)
Camera is 18m south of vehicle, 8m above vehicle's physics ground.

---

## Current spawn position (host_vehicle.c ~line 238)

```c
int32_t spawn_x = 0x4C40000;  // 610m
int32_t spawn_z = 0x3480000;  // 420m
```

The spawn was moved here to be "south of the complex" but this is still inside the
footprint of bones 113/129 which span the full level. Try Z=375m (near south level edge)
to get completely clear of all XOBF geometry and confirm the camera view.

---

## Recommended fix sequence

1. **Confirm camera is in clear space**: Move spawn to Z=375m (just inside south level
   edge). If view is still blocked, the geometry covers the full level south to north.

2. **Test with glDisable(GL_CULL_FACE)**: Before the XOBF glDrawArrays call, disable
   culling. If ground appears, fix the winding. If ground still doesn't appear, it's
   either invisible (dark) or the camera is inside geometry.

3. **Test with green tint**: Set XOBF tint to (0,3,0) to confirm ground is rendering
   vs. being invisible due to colour.

4. **Re-enable ZONE rendering alongside XOBF**: In renderer.c remove the `else if`
   and make both draw always. ZONE provides guaranteed-correct flat ground with a
   distinctive green colour. Z-fighting between ZONE and XOBF at the same Y level
   is acceptable for debugging.

5. **Consider disabling XOBF structural bones**: Only some bones represent driveable
   terrain (flat ground). Bones with large Y ranges (>5m) are structural/decorative.
   Filter them from rendering (keep them in physics). This would let the ZONE terrain
   show through without Z-fighting.

---

## Key files

| File | Purpose |
|---|---|
| `platform/terrain_mesh.c` | XOBF loader, tm_parse_bin() winding, TerrainMesh_HeightAt() |
| `platform/renderer.c` | Render loop, ZONE/XOBF draw decision, camera |
| `platform/host_vehicle.c` | Spawn position |
| `tools/analyze_bin_structure.py` | Run to re-examine BIN bone extents |
| `tools/analyze_head_transforms.py` | Object instance positions (gameplay only, not visual) |

---

## What is NOT broken

- Vehicle physics grounding: works correctly via TerrainMesh_HeightAt
- Vehicle model rendering (orange vehicle visible)
- Steering and movement
- Asset loading from `PS1 game/` directory (fixed this session)
- Gravity and terrain snap
