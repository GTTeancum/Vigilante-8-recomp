# Decomp Rules — Naming, Confidence, Structs

## Naming Conventions

### Functions

Format: `Subsystem_ActionTarget` (PascalCase, underscore-separated subsystem).

Examples:
- `Vehicle_UpdatePhysics`
- `Vehicle_ApplySuspensionForce`
- `Collision_VehicleVsTerrain`
- `Asset_LoadTIM`
- `Level_ParseGeometry`
- `Weapon_FireProjectile`
- `AI_SelectTarget`
- `Match_UpdateState`

Known PSY-Q library functions keep their original names exactly: `CdRead`, `VSync`, `LoadImage`, etc.

Functions called from many places that look like utilities get a `Util_` or `Math_` prefix:
- `Math_FixedMul`
- `Math_Sqrt16`
- `Util_MemCopy`

### Variables

Local variables: `camelCase`. Be descriptive once purpose is known:
- `int frameCounter;`
- `Vehicle *targetVehicle;`
- `fixed16_t suspensionCompression;`

Globals: `g_camelCase`:
- `Vehicle g_vehicles[MAX_VEHICLES];`
- `int g_frameCounter;`
- `MatchState g_currentMatch;`

Static module-locals: `s_camelCase`:
- `static int s_rngSeed;`

Constants: `UPPER_SNAKE_CASE`:
- `#define MAX_VEHICLES 16`
- `#define PHYSICS_TICK_HZ 60`

### Structs

PascalCase, descriptive:
- `Vehicle`, `Weapon`, `Projectile`, `Level`, `MatchState`, `AIBrain`, `InputCommand`

Field names: `camelCase`, descriptive.

### Files

snake_case, mirroring subsystem:
- `src/physics/vehicle_dynamics.c`
- `src/physics/collision.c`
- `src/assets/tim_loader.c`
- `src/gameplay/ai.c`

## Confidence Tags

Every named function, struct, and field carries a confidence tag in a comment.

### Tags

- **HIGH** — Name is verified. The function matches a known PSY-Q signature, or the name comes from a string in the binary, or behavior was traced end-to-end and is unambiguous.
- **MED** — Name reflects clear behavior but is inferred. The function does what the name says, but the exact name is the decompiler's choice.
- **LOW** — Partially evidenced. Behavior is not fully resolved. Names must state the known evidence without inventing purpose.
- **UNKNOWN** — Couldn't determine purpose. Keep an `sub_<address>` style name. Document what is known in a comment.

### Format

```c
// HIGH: matches PSY-Q CdRead signature exactly
int CdRead(int sector, void *buffer, int mode);

// MED: called every frame from main loop, modifies vehicle state, calls GTE matrix ops
void Vehicle_UpdatePhysics(Vehicle *v, int deltaTime);

// LOW: fires a projectile based on context, weapon type inference uncertain
void Weapon_FireProjectile_Unresolved(Vehicle *shooter, int weaponSlot);

// UNKNOWN: called from main loop, modifies global at 0x800a1240, no other context
void sub_8001f200(int *p1, int p2);
```

For structs:
```c
typedef struct {
    fixed16_t posX;          // HIGH: read by GTE transform setup
    fixed16_t posY;          // HIGH
    fixed16_t posZ;          // HIGH
    fixed16_t velX;          // HIGH: added to posX each tick
    fixed16_t velY;          // HIGH
    fixed16_t velZ;          // HIGH
    int health;              // MED: decremented on damage events, compared against 0
    int unknownField0x40;    // UNKNOWN: read once in collision code, never written
    // ...
} Vehicle;
```

### Promotion Rules

A name's confidence rises as more code references it consistently. After pass 2 and pass 3, re-evaluate every LOW and UNKNOWN tag. If new context resolves it, promote and update `rename_log.md`.

## Struct Recovery

### Detection

A struct exists when:
- A pointer is passed to multiple functions that access different offsets from it.
- An allocation site (`malloc3`, fixed buffer, or static array) has a consistent size.
- The same offset is read/written across many functions on the same pointer type.

### Sizing

Determine struct size from:
1. Allocation calls (malloc with literal size).
2. Array stride (`vehicle = &g_vehicles[i * 0x200]` → 0x200 byte stride).
3. Highest accessed offset + accessed field size.

If size differs across these signals, log the conflict in `decisions.log` and use the largest.

### Field Recovery

For each offset accessed:
1. Note read/write pattern (read-only, write-only, read-write).
2. Note value range if comparisons are present.
3. Note what other functions do with the value (passed to GTE → likely 3D coord; compared to 0 → likely flag or counter).
4. Name based on pattern. Use `unknownField0xNN` for offsets that are accessed but purpose is unclear.

### Padding and Unions

PS1 structs often have explicit padding. If an offset is read but always zero / never written, mark it `padding_0xNN` or `reserved_0xNN`.

If the same offset has different access patterns in different contexts, it may be a union. Document both interpretations in a comment.

## Multi-Pass Refinement

### Pass 1 — Decomp
- Convert MIPS / Ghidra pseudo-C to clean C.
- First-pass naming. LOW confidence is fine.
- Struct shells created (size known, some fields named).

### Pass 2 — Struct + Context
- Cluster struct field accesses across entire codebase.
- Finalize struct layouts.
- Propagate types: if a function takes `void *p` and is always called with `Vehicle *`, retype the parameter.
- Re-name based on cross-function context. Promote LOW → MED where justified.

### Pass 3 — Polish
- Final naming pass.
- Resolve as many UNKNOWN tags as possible.
- Tighten comments. Remove TODO markers that have been addressed.
- Ensure every function in `src/` has at least a one-line purpose comment.

After each pass, update `rename_log.md` with every name change so audit is possible.

## Ghidra Output Hygiene

Ghidra's pseudo-C is a starting point, not the output. Always:

- Remove `undefined`, `undefined4`, etc. — pick real types.
- Replace `param_1`, `param_2` with named parameters.
- Replace `iVar1`, `uVar2` with named locals.
- Replace `*(int *)(buffer + 0x10)` with `buffer->fieldName` once struct is known.
- Replace magic numbers with `#define`d constants.
- Remove Ghidra artifacts (`bool` returns that are actually int, etc.) and use idiomatic C.
- Reformat for readability: K&R braces, sensible line breaks.

The bar: a programmer who has never seen the binary should be able to read the function and understand what it does.

## What Not to Do

- **Do not "fix" original bugs.** If the original code has an off-by-one or a misused variable, preserve it. Note it in a comment. The port is 1:1, including bugs.
- **Do not "modernize" idioms.** Don't replace `memset(p, 0, sizeof(*p))` with C++ default initialization. Don't introduce `const` where the original had none. Match the era's style.
- **Do not invent code.** If a function is unclear, mark it UNKNOWN. Never fill in plausible-looking code that wasn't in the binary.
- **Do not skip functions because they're hard.** Mark them UNKNOWN with a description of what is known, and move on. They get another pass later.
