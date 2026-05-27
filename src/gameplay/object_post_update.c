/* object_post_update.c -- "fast insert" pool helpers + projectile post-tick.
 *
 * Source: SLUS_005.10
 *   FUN_8001fe50  -- ObjList_FastInsert
 *   FUN_80020744  -- Object_RegisterPostUpdate
 *   FUN_8003eab0  -- Projectile_GravityTick (universal callback for the
 *                    XOBF-spawned dropped projectiles across DLLs).
 *
 * ObjList_FastInsert: O(1) head insert into the chain rooted at
 * `listHead`, pulling a free node from the global scratch pool
 * piRam0000076c.
 *
 * Object_RegisterPostUpdate: set the "back-buf pending" bit (0x80)
 * on the object's flags and link it into the back-buf list at
 * DAT_80065a60. Called by every projectile spawner after the new
 * object is configured.
 *
 * Projectile_GravityTick (FUN_8003eab0): the universal "I'm a
 * gravity-affected projectile" callback used by every non-AI
 * projectile across DLLs. Each tick:
 *   1. integrate position by velocity (no scale -- already in world units)
 *   2. apply rotation via Object_ApplyAngularVelocity (small-angle GTE)
 *   3. apply gravity to vy (+0x5a/tick)
 *   4. if below terrain at (posX, posZ), return 0 to despawn
 *
 * HIGH-MED.
 */
#include <stdint.h>
#include <stdio.h>

extern int32_t **piRam0000076c;
extern uint8_t  DAT_80065a70[];
extern uint8_t  DAT_80065a60[];
extern void Object_ApplyAngularVelocity(uint32_t *m, int pitchRate, int yawRate, int rollRate);  /* FUN_800439b8 */
extern int  Terrain_HeightAndProbe(intptr_t self, int *posXyz, void *normalOut, uintptr_t *materialOut);    /* FUN_8001d748 */
extern void *FUN_8001f5a0(intptr_t self, intptr_t query);   /* SAT_SelectAxis */
extern void  FUN_800176f8(intptr_t obj, int32_t *vec, int32_t *pos); /* Object_ApplyImpulseAtPoint */
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);
extern void  FUN_80012068(int idx, int a, int b, int c);    /* HudFlash_SetEntry */
extern void  FUN_800205f8(int obj);                         /* Damage_Apply */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static inline int32_t mips_sll_i32(int32_t value, unsigned shift)
{
    return (int32_t)((uint32_t)value << (shift & 31));
}

/* HIGH: head-insert `payload` into `listSentinel`'s chain. */
typedef struct HostObjListNode {
    struct HostObjListNode *next;
    struct HostObjListNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} HostObjListNode;

static HostObjListNode s_host_obj_nodes[4096];
static uint32_t s_host_obj_node_next;

void ObjList_HostResetPool(void)
{
    s_host_obj_node_next = 0;
    for (uint32_t i = 0; i < (uint32_t)(sizeof s_host_obj_nodes / sizeof s_host_obj_nodes[0]); i++) {
        s_host_obj_nodes[i].next = NULL;
        s_host_obj_nodes[i].prev = NULL;
        s_host_obj_nodes[i].payload = 0;
        s_host_obj_nodes[i].deadline = 0;
    }
}

static HostObjListNode *host_list_node_alloc(void)
{
    if (s_host_obj_node_next >= (uint32_t)(sizeof s_host_obj_nodes / sizeof s_host_obj_nodes[0]))
        return NULL;
    return &s_host_obj_nodes[s_host_obj_node_next++];
}

void ObjList_FastInsert(uintptr_t listSentinel, uintptr_t payload)
{
    HostObjListNode *sentinel = (HostObjListNode *)listSentinel;
    HostObjListNode *node = NULL;

    if (piRam0000076c != NULL && *piRam0000076c != NULL) {
        node = (HostObjListNode *)*piRam0000076c;
        piRam0000076c = (int32_t **)node;
    } else {
        node = host_list_node_alloc();
    }
    if (node == NULL)
        return;

    if (sentinel->prev == NULL) {
        sentinel->next = NULL;
        sentinel->prev = sentinel;
        sentinel->payload = 0;
    }

    node->payload = payload;
    node->next = NULL;
    node->prev = sentinel->prev;
    if (node->prev == sentinel) {
        sentinel->next = node;
    } else {
        node->prev->next = node;
    }
    sentinel->prev = node;
}

/* HIGH: PSX symbol alias for ObjList_FastInsert.  Several decompiled level
 * callbacks still call the original address directly, so this must not be a
 * platform stub. */
void FUN_8001fe50(uintptr_t listSentinel, uintptr_t payload)
{
    ObjList_FastInsert(listSentinel, payload);
}

void Object_RegisterPostUpdate(uint32_t *obj)
{
    obj[0] |= 0x80u;
    ObjList_FastInsert((uintptr_t)DAT_80065a60, (uintptr_t)obj);
}

/* HIGH: gravity, terrain bounce, and collision response per tick. */
uint32_t Projectile_GravityTick(uint8_t *obj, int mode, intptr_t arg)
{
    if (mode == 0) {
        *(int32_t *)(obj + 0x24) = mips_addu_i32(*(int32_t *)(obj + 0x24),
                                                 *(int32_t *)(obj + 0x88));
        *(int32_t *)(obj + 0x28) = mips_addu_i32(*(int32_t *)(obj + 0x28),
                                                 *(int32_t *)(obj + 0x8c));
        *(int32_t *)(obj + 0x2c) = mips_addu_i32(*(int32_t *)(obj + 0x2c),
                                                 *(int32_t *)(obj + 0x90));

        Object_ApplyAngularVelocity((uint32_t *)(obj + 0x10),
                                    (int)*(int16_t *)(obj + 0x80),
                                    (int)*(int16_t *)(obj + 0x82),
                                    (int)*(int16_t *)(obj + 0x84));

        int32_t vy = mips_addu_i32(*(int32_t *)(obj + 0x8c), 0x5a);
        *(int32_t *)(obj + 0x8c) = vy;
        if (vy > 0) {
            int32_t terrainY = Terrain_HeightAndProbe((intptr_t)obj, (int *)(obj + 0x24),
                                                      NULL, NULL);
            if (*(int32_t *)(obj + 0x28) <= terrainY) return 0;

            *(int32_t *)(obj + 0x28) = terrainY;
            int8_t bounceCount = (int8_t)(*(int8_t *)(obj + 0x87) - 1);
            *(int8_t *)(obj + 0x87) = bounceCount;
            *(int32_t *)(obj + 0x8c) = mips_subu_i32(0, *(int32_t *)(obj + 0x8c)) / 2;
            if (bounceCount == 0) {
                FUN_800205f8((int)(uintptr_t)obj);
                return 0xffffffffu;
            }
        }
        return 0;
    }

    if (mode != 3) return 0;

    uint32_t *query = (uint32_t *)(uintptr_t)arg;
    intptr_t hitObj = (intptr_t)Collision_QueryHostWord(query, 0);
    if (*(int8_t *)(uintptr_t)(hitObj + 4) != 2) {
        return 0;
    }

    FUN_8001f5a0((intptr_t)obj, (intptr_t)query);
    int32_t dot = mips_addu_i32(
        mips_addu_i32(mips_mult_lo_i32(*(int32_t *)(obj + 0x88),
                                       (int32_t)*(int16_t *)(query + 8)),
                      mips_mult_lo_i32(*(int32_t *)(obj + 0x8c),
                                       (int32_t)*(int16_t *)((uint8_t *)query + 0x22))),
        mips_mult_lo_i32(*(int32_t *)(obj + 0x90),
                         (int32_t)*(int16_t *)(query + 9)));
    if (dot < 0) dot = mips_addu_i32(dot, 0x7ff);
    dot >>= 11;
    if (dot >= 0) return 0;

    int32_t impulse[3];
    impulse[0] = mips_sll_i32(*(int32_t *)(obj + 0x88), 7);
    impulse[1] = mips_sll_i32(*(int32_t *)(obj + 0x8c), 7);
    impulse[2] = mips_sll_i32(*(int32_t *)(obj + 0x90), 7);
    FUN_800176f8(hitObj, impulse, (int32_t *)(obj + 0x24));

    if (*(int16_t *)(uintptr_t)(hitObj + 6) < 0) {
        FUN_80012068(~(int)*(int16_t *)(uintptr_t)(hitObj + 6), 0xff, 2, 0x80);
    }

    int32_t adjust = mips_mult_lo_i32(dot, (int32_t)*(int16_t *)(query + 8));
    if (adjust < 0) adjust = mips_addu_i32(adjust, 0xfff);
    *(int32_t *)(obj + 0x88) = mips_subu_i32(*(int32_t *)(obj + 0x88), adjust >> 12);

    adjust = mips_mult_lo_i32(dot, (int32_t)*(int16_t *)((uint8_t *)query + 0x22));
    if (adjust < 0) adjust = mips_addu_i32(adjust, 0xfff);
    *(int32_t *)(obj + 0x8c) = mips_subu_i32(*(int32_t *)(obj + 0x8c), adjust >> 12);

    adjust = mips_mult_lo_i32(dot, (int32_t)*(int16_t *)(query + 9));
    if (adjust < 0) adjust = mips_addu_i32(adjust, 0xfff);
    *(int32_t *)(obj + 0x90) = mips_subu_i32(*(int32_t *)(obj + 0x90), adjust >> 12);
    return 0;
}

/* FUN_8003eab0 -- hex alias; weapon_split.c stores this as the tick callback ptr. */
uint32_t FUN_8003eab0(uint8_t *obj, int mode, intptr_t arg)
{
    return Projectile_GravityTick(obj, mode, arg);
}

/* ================================================================
 * FUN_8001dc1c -- Object_RecomputeBoundingRadius
 *
 * Walk the child hierarchy (sibling chain via +0x34) computing the
 * max of (child_self_radius + Vec3_Length(child->pos_at+0x24)) across
 * all children, then store the result at obj+0x54.
 *
 * The "self" radius starts from obj->boneBank[+0x24] shifted left by
 * (0x10 - boneBank[+0x26]) bits (i.e., a 4.12 fixed-point bank-local
 * size scaled into world coords).
 *
 * HIGH confidence (direct Ghidra port).
 * ================================================================ */
extern int FUN_80016a20(const int32_t *v);   /* Vec3_Length */
int FUN_8001dc1c(int param_1)
{
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int iVar5;

    iVar3 = *(int *)(uintptr_t)(param_1 + 0x30);
    iVar5 = 0;
    if (iVar3 != 0) {
        iVar5 = (int)((uint32_t)*(uint16_t *)(uintptr_t)(iVar3 + 0x24)
                      << (0x10 - *(uint16_t *)(uintptr_t)(iVar3 + 0x26) & 0x1f));
    }
    for (iVar3 = *(int *)(uintptr_t)(param_1 + 0x38);
         iVar3 != 0;
         iVar3 = *(int *)(uintptr_t)(iVar3 + 0x34))
    {
        iVar1 = FUN_8001dc1c(iVar3);
        iVar2 = FUN_80016a20((const int32_t *)(uintptr_t)(iVar3 + 0x24));
        iVar4 = iVar1 + iVar2;
        if (iVar1 + iVar2 < iVar5) {
            iVar4 = iVar5;
        }
        iVar5 = iVar4;
    }
    *(int *)(uintptr_t)(param_1 + 0x54) = iVar5;
    return iVar5;
}

/* ================================================================
 * FUN_8002036c -- Object_PostUpdate2
 *
 * Post-update hook: rebuild bone matrix, recompute bounding radius,
 * dispatch event 1 to the per-object callback. If the callback returns
 * non-negative AND the object has flag bit 3 with no current binding,
 * attach scenery via FUN_8003e730. Finally re-register the object in
 * all active scene lists.
 * ================================================================ */
extern void  FUN_8001d708(intptr_t obj);            /* BuildBoneMatrix */
extern void  FUN_8003e730(uint32_t *obj);           /* SceneryAttach */
extern void  FUN_800202f4(uint32_t *obj);           /* Object_RegisterInScene */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);
uint32_t FUN_8002036c(uint32_t *param_1);

uint32_t FUN_8002036c(uint32_t *param_1)
{
    int      iVar1;
    uint32_t uVar2;

    /* Ghidra: FUN_8001d708() with no args -- $a0 still holds param_1. */
    FUN_8001d708((intptr_t)param_1);
    FUN_8001dc1c((int)(uintptr_t)param_1);
    uintptr_t callback = Object_CallbackFromPsxSlot(param_1);
    if (callback == 0) {
        iVar1 = 0;
    } else {
        typedef int (*EventFn)(uint32_t *, int, intptr_t);
        iVar1 = ((EventFn)callback)(param_1, 1, 0);
    }
    uVar2 = 0;
    if (-1 < iVar1) {
        if (((*param_1 & 8u) != 0) && (param_1[0x1c] == 0)) {
            FUN_8003e730(param_1);
        }
        FUN_800202f4(param_1);
        /* Source return is the non-zero node pointer left in v0 by the final
         * FUN_8001fe50 insert.  Host list insertion is void, but callers such
         * as LOAD.DLL case 2/3/4 only need the source truth value. */
        uVar2 = 1;
    }
    return uVar2;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001fe50  (from analysis/SLUS_005.10/decomp/8001fe50.c) --- */
// addr: 0x8001fe50  name: FUN_8001fe50

void FUN_8001fe50(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = piRam0000076c;
  iVar3 = *piRam0000076c;
  *(undefined **)(iVar3 + 4) = &DAT_80065a70;
  piVar2 = piRam0000076c + 2;
  piRam0000076c = (int *)iVar3;
  *piVar2 = param_2;
  piVar2 = *(int **)(param_1 + 8);
  *(int **)(param_1 + 8) = piVar1;
  *piVar2 = (int)piVar1;
  piVar1[1] = (int)piVar2;
  *piVar1 = param_1 + 4;
  return;
}

/* --- SLUS_005.10 FUN_80020744  (from analysis/SLUS_005.10/decomp/80020744.c) --- */
// addr: 0x80020744  name: FUN_80020744

void FUN_80020744(uint *param_1)

{
  *param_1 = *param_1 | 0x80;
  FUN_8001fe50(&DAT_80065a60);
  return;
}

/* --- SLUS_005.10 FUN_8003eab0  (from analysis/SLUS_005.10/decomp/8003eab0.c) --- */
// addr: 0x8003eab0  name: FUN_8003eab0

undefined4 FUN_8003eab0(int param_1,int param_2,int *param_3)

{
  int iVar1;
  char cVar2;
  int iVar3;
  int local_28;
  int local_24;
  int local_20;
  
  if (param_2 == 0) {
    *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + *(int *)(param_1 + 0x88);
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x8c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + *(int *)(param_1 + 0x90);
    FUN_800439b8(param_1 + 0x10,(int)*(short *)(param_1 + 0x80),(int)*(short *)(param_1 + 0x82),
                 (int)*(short *)(param_1 + 0x84));
    iVar3 = *(int *)(param_1 + 0x8c) + 0x5a;
    *(int *)(param_1 + 0x8c) = iVar3;
    if (0 < iVar3) {
      iVar3 = FUN_8001d748(param_1,param_1 + 0x24,0,0);
      if (*(int *)(param_1 + 0x28) <= iVar3) {
        return 0;
      }
      *(int *)(param_1 + 0x28) = iVar3;
      cVar2 = *(char *)(param_1 + 0x87) + -1;
      *(char *)(param_1 + 0x87) = cVar2;
      *(int *)(param_1 + 0x8c) = -*(int *)(param_1 + 0x8c) / 2;
      if (cVar2 == '\0') {
        FUN_800205f8(param_1);
        return 0xffffffff;
      }
    }
  }
  else {
    if (param_2 != 3) {
      return 0;
    }
    iVar3 = *param_3;
    if (*(char *)(iVar3 + 4) != '\x02') {
      return 0;
    }
    FUN_8001f5a0(param_1,param_3);
    iVar1 = *(int *)(param_1 + 0x88) * (int)(short)param_3[8] +
            *(int *)(param_1 + 0x8c) * (int)*(short *)((int)param_3 + 0x22) +
            *(int *)(param_1 + 0x90) * (int)(short)param_3[9];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x7ff;
    }
    iVar1 = iVar1 >> 0xb;
    if (-1 < iVar1) {
      return 0;
    }
    local_28 = *(int *)(param_1 + 0x88) << 7;
    local_24 = *(int *)(param_1 + 0x8c) << 7;
    local_20 = *(int *)(param_1 + 0x90) << 7;
    FUN_800176f8(iVar3,&local_28,param_1 + 0x24);
    if (*(short *)(iVar3 + 6) < 0) {
      FUN_80012068(~(int)*(short *)(iVar3 + 6),0xff,2,0x80);
    }
    iVar3 = iVar1 * (short)param_3[8];
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - (iVar3 >> 0xc);
    iVar3 = iVar1 * *(short *)((int)param_3 + 0x22);
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) - (iVar3 >> 0xc);
    iVar1 = iVar1 * (short)param_3[9];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xfff;
    }
    *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) - (iVar1 >> 0xc);
  }
  return 0;
}

/* --- SLUS_005.10 FUN_800439b8  (from analysis/SLUS_005.10/decomp/800439b8.c) --- */
// addr: 0x800439b8  name: FUN_800439b8

void FUN_800439b8(undefined4 *param_1,int param_2,int param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldR11R12(*param_1);
  gte_ldR13R21(param_1[1]);
  gte_ldR22R23(param_1[2]);
  gte_ldR31R32(param_1[3]);
  gte_ldR33(param_1[4]);
  gte_ldIR1(0x1000);
  gte_ldIR2(param_4);
  gte_ldIR3(-param_3);
  gte_rtir_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(-param_4,0x1000,param_2);
  gte_rtir_b();
  *(short *)param_1 = (short)uVar1;
  *(short *)((int)param_1 + 6) = (short)uVar2;
  *(short *)(param_1 + 3) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(param_3,-param_2,0x1000);
  gte_rtir_b();
  *(short *)((int)param_1 + 2) = (short)uVar1;
  *(short *)(param_1 + 2) = (short)uVar2;
  *(short *)((int)param_1 + 0xe) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *(short *)(param_1 + 1) = (short)uVar1;
  *(short *)((int)param_1 + 10) = (short)uVar2;
  *(short *)(param_1 + 4) = (short)uVar3;
  return;
}

/* --- SLUS_005.10 FUN_8001d748  (from analysis/SLUS_005.10/decomp/8001d748.c) --- */
// addr: 0x8001d748  name: FUN_8001d748

int FUN_8001d748(int param_1,undefined4 *param_2,SVECTOR *param_3,undefined4 *param_4)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = FUN_80025400(*param_2,param_2[2]);
  if ((*(int *)(param_1 + 0x74) == 0) ||
     ((iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x74),iVar1,param_2,param_3), iVar2 == 0 &&
      ((*(int *)(param_1 + 0x78) == 0 ||
       (iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x78),iVar1,param_2,param_3), iVar2 == 0)))))) {
    if (param_3 != (SVECTOR *)0x0) {
      FUN_80025648(*param_2,param_2[2],param_3);
      VectorNormalSS(param_3,param_3);
    }
    if (param_4 != (undefined4 *)0x0) {
      uVar3 = FUN_800255f4(*param_2,param_2[2]);
      *param_4 = uVar3;
    }
  }
  else {
    iVar1 = iVar2;
    if (param_4 != (undefined4 *)0x0) {
      *param_4 = 0;
    }
  }
  return iVar1;
}

#endif  /* GHIDRA REF */
