/* terrain_damage_tick.c -- terrain area-of-effect damage zone tick (LAB_80033c74).
 *
 * Source: SLUS_005.10  LAB_80033c74  0x80033c74  (~60 instructions).
 *
 * Installed at +0x64 of terrain damage-zone objects (e.g., oil slick, fire pit)
 * by FUN_80033db4 (the zone spawner).
 *
 * Event dispatch:
 *   2  -> decrement grid-occupancy counters for all cells this zone covers.
 *         Calls FUN_800422d8 (QuadTree_Update) unconditionally.
 *         If any counter changed: schedule FUN_80020890(obj, 60).
 *         If no change (already zeroed): FUN_80045088(obj) cleanup.
 *         Return -1 (event handled).
 *   other -> return 0.
 *
 * Object field layout:
 *   +0x48 (72)  : world X position (int32, 16.16 fixed)
 *   +0x50 (80)  : world Z position (int32, 16.16 fixed)
 *   +0x54 (84)  : zone radius (int32, 16.16 fixed)
 *   +0x80 (128) : array of uint16 per-cell slot counters (one per iteration)
 *
 * Grid: DAT_80091120 -- 2D occupancy grid, each cell is a uint16 count.
 *   Addressing: cell[x_cell][z_cell] where x_cell = pos_x >> 16,
 *   z_cell = pos_z >> 16.  The grid is divided into 64×64 super-cells
 *   (each 64 sub-cells per axis).
 *   Inner address = *(int32*)(DAT_80091120 + (x>>6)*128 + (z>>6)*4 + 0x80)
 *                 + (x&63)*128 + (z&63)*2
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_800422d8 -- Proximity_DispatchInRange: walk proximity chain, dispatch
 * triggers within (radius+tolerance) of (posX, posZ). Called after the grid
 * update with the zone's own centre X, Z and radius as tolerance. */
extern void FUN_800422d8(int posX, int posZ, int tolerance);

/* FUN_80020890 -- Object_SchedulePostEvent: queue a post-frame event. */
extern void FUN_80020890(int obj, int timer);

/* FUN_80045088 -- DamageZone_CleanupEmpty: release empty zone. */
extern void FUN_80045088(int obj);

/* DAT_80091120: 2D spatial occupancy grid (runtime RAM, not in EXE).
 * Layout: array of row descriptors; each descriptor holds a pointer at +0x80
 * to a uint16 column array. */
extern uint8_t DAT_80091120[];

/* ------------------------------------------------------------------ */

int LAB_80033c74(int obj, int event, int param3)
{
    (void)param3;
    uint8_t *s1 = (uint8_t *)(uintptr_t)(uint32_t)obj;

    if (event != 2)
        return 0;

    /* Decode position and radius from fixed-point (16.16) world coords. */
    uint32_t pos_x  = *(uint32_t *)(s1 + 0x48);
    uint32_t pos_z  = *(uint32_t *)(s1 + 0x50);
    uint32_t radius = *(uint32_t *)(s1 + 0x54);

    /* Compute grid-cell ranges (RTZ, treating values as unsigned).
     * x_min = (X - R + 0xffff) >> 16  (nearest cell that touches the zone)
     * x_max = (X + R)          >> 16  (farthest cell)
     * Same for Z.
     */
    uint32_t x_min = (pos_x - radius + 0xffffu) >> 16;
    uint32_t x_max = (pos_x + radius)            >> 16;
    uint32_t z_min = (pos_z - radius + 0xffffu) >> 16;
    uint32_t z_max = (pos_z + radius)            >> 16;

    int s0 = 0;  /* changed-cells flag */

    /* Pointer into per-cell slot counter array at obj[0x80].
     * Each iteration reads the 16-bit counter for that cell then advances. */
    uint16_t *t2 = (uint16_t *)(s1 + 0x80);

    /* Validate range. */
    if (x_max >= x_min && z_max >= z_min) {
        /* Cache super-cell row offset for x_min: t4 = (x_min >> 6) * 128 */
        uint32_t t4 = (x_min >> 6) << 7;
        /* Cache sub-row column base for x_min: t3 = (x_min & 63) * 128 */
        uint32_t t3 = (x_min & 63u) << 7;

        /* Grid base */
        uint8_t *t7 = DAT_80091120;

        uint32_t xi = x_min;
        do {
            uint32_t zi = z_min;
            do {
                /* Read and consume slot counter for this (xi, zi) cell. */
                uint16_t *t0 = t2;   /* points to current counter */
                uint16_t slot_cnt = *(t2++);
                if (slot_cnt != 0) {
                    /* Decrement slot counter. */
                    (*t0)--;

                    /* Compute grid cell address.
                     * row_ptr = *(int32*)(t7 + (xi>>6)*128 + (zi>>6)*4 + 0x80)
                     * cell    = row_ptr + (xi&63)*128 + (zi&63)*2 */
                    uint32_t row_off = (zi >> 6) * 4u + t4; /* (zi_hi*4 + xi_hi*128) */
                    uint8_t  *row_desc = t7 + row_off;
                    int32_t   row_ptr  = *(int32_t *)(row_desc + 0x80);
                    uint32_t  col_off  = t3 + (zi & 63u) * 2u; /* xi_lo*128 + zi_lo*2 */
                    uint16_t *cell = (uint16_t *)(uintptr_t)(uint32_t)(row_ptr + col_off);

                    /* Decrement grid cell occupancy. */
                    (*cell)--;
                    s0 = 1;
                }
                zi++;
            } while (zi <= z_max);

            xi++;
            /* Recompute row offset for next x stripe */
            t4 = (xi >> 6) << 7;
            t3 = (xi & 63u) << 7;
        } while (xi <= x_max);
    }

    /* Update quad-tree unconditionally with zone centre and radius.
     * a0 (pos_x), a1 (pos_z), a2 (radius) are preserved through the loop. */
    FUN_800422d8((int)pos_x, (int)pos_z, (int)radius);

    if (s0 == 0) {
        /* No cells modified: zone has already been removed from grid; clean up. */
        FUN_80045088(obj);
        return -1;
    } else {
        /* Cells were modified: schedule a follow-up event. */
        FUN_80020890(obj, 60);
        return 0;  /* FUN_80020890 sets v0; MIPS falls through to epilogue with 0 */
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_80033c74  (manual disassembly reference) ---
 *
 * 80033c74: addiu sp,sp,-32
 * 80033c7c: addu s1,a0,zero       ; s1 = obj
 * 80033c80: addiu v0,zero,2
 * 80033c88: bne a1,v0,0x80033d9c  ; if event != 2 -> return 0
 * 80033c90: lw a0,72(s1)          ; a0 = obj[0x48] = X
 * 80033c94: lw a2,84(s1)          ; a2 = obj[0x54] = radius
 * 80033c98: addiu t2,s1,128       ; t2 = &obj[0x80]
 * 80033c9c: ori v1,zero,0xffff
 * 80033ca0: lw a1,80(s1)          ; a1 = obj[0x50] = Z
 * 80033ca4: subu v0,a0,a2; addu v0,v0,v1; srl t1,v0,16  ; x_min
 * 80033cb0: addu v0,a0,a2; srl t6,v0,16                  ; x_max
 * 80033cb8: subu v0,a1,a2; addu v0,v0,v1; srl t8,v0,16  ; z_min
 * 80033cc4: addu v0,a1,a2; srl t5,v0,16                  ; z_max
 * 80033ccc: sltu v0,t6,t1; bne v0,zero,0x80033d74  ; t6<t1 -> skip
 * 80033cd4: addu s0,zero,zero     ; s0 = 0 (changed flag)
 * 80033cd8: lui v0,0x8009
 * 80033cdc: addiu t7,v0,4384      ; t7 = DAT_80091120
 * 80033ce0: addu a3,t8,zero       ; a3 = zi = z_min
 * 80033ce4: sltu v0,t5,a3; bne v0,zero,0x80033d64  ; t5<a3 -> skip inner
 * 80033cec: srl v0,t1,6; sll t4,v0,7   ; t4 = (xi>>6)*128
 * 80033cf8: andi v0,t1,0x3f; sll t3,v0,7 ; t3 = (xi&63)*128
 * 80033cfc: addiu t0,t2,-2        ; t0 = t2 - 2 (pre-increment trick)
 * 80033d00: lhu v0,0(t2)          ; v0 = slot_cnt
 * 80033d04: addiu t0,t0,2         ; t0 = t2 (the slot counter ptr)
 * 80033d08: beq v0,zero,0x80033d54 ; if == 0 skip
 * 80033d0c: addiu t2,t2,2         ; t2++
 * 80033d10: lhu v0,0(t0); addiu v0,v0,-1; sh v0,0(t0)  ; (*t0)--
 * 80033d20: srl v0,a3,6; sll v0,v0,2; addu v0,v0,t4; addu v0,v0,t7
 * 80033d30: lw v1,128(v0)         ; row_ptr
 * 80033d34: andi v0,a3,0x3f; sll v0,v0,1  ; (zi&63)*2
 * 80033d3c: addu v1,v1,t3; addu v1,v1,v0  ; cell = row_ptr + xi_col + zi_col
 * 80033d44: lhu v0,0(v1); addiu s0,zero,1
 * 80033d4c: addiu v0,v0,-1; sh v0,0(v1)   ; cell--
 * 80033d54: addiu a3,a3,1; sltu v0,t5,a3; beq v0,zero,0x80033d00 ; zi++
 * 80033d64: addiu t1,t1,1; sltu v0,t6,t1; beq v0,zero,0x80033ce4 ; xi++
 * 80033d74: jal FUN_800422d8
 * 80033d7c: bne s0,zero,0x80033d94 ; if changed: schedule
 * 80033d84: jal FUN_80045088(s1)
 * 80033d8c: j 0x80033da0; addiu v0,zero,-1 ; return -1
 * 80033d94: jal FUN_80020890(s1, 60) ; schedule(obj, 60)
 * 80033d9c: addu v0,zero,zero           ; (non-event-2 exit) return 0
 */
#endif /* GHIDRA REF */
