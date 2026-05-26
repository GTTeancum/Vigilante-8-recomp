/* object_event_queue.c -- Object deferred-event priority queue.
 *
 * Source: SLUS_005.10
 *   FUN_80020890  Object_SchedulePostEvent(obj, delay)  (~30 instr)
 *
 * Inserts `obj` into the sorted event queue with a deadline of
 * (current_tick + delay).  If the object is already queued (bit 0 of
 * obj[0] set), it is removed first via FUN_8001fe8c.
 *
 * Queue structure (doubly-linked, sorted ascending by deadline):
 *   node[0] = prev pointer (int)
 *   node[1] = next pointer (int)
 *   node[2] = object pointer (int)
 *   node[3] = deadline tick (int)
 *
 * piRam0000076c -- pointer to the current free-node head (same layout).
 * piRam000007bc -- pointer to the queue sentinel/head.
 * iRam0000000c  -- current game tick counter.
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t  *piRam000007bc;   /* event-queue sentinel/head */
extern void     *puRam000007c4;   /* event-queue tail/sentinel */
extern int32_t   iRam0000000c;    /* current game tick */

extern uint8_t   DAT_80065ac0[];  /* active-list sentinel (used by FUN_8001fe8c) */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

typedef struct HostEventQueueNode {
    struct HostEventQueueNode *next;
    struct HostEventQueueNode *prev;
    uint32_t *obj;
    uint32_t deadline;
} HostEventQueueNode;

static HostEventQueueNode s_event_nodes[2048];
static uint32_t s_event_next;
static HostEventQueueNode *s_event_free;

void ObjectEventQueue_HostReset(void)
{
    s_event_next = 0;
    s_event_free = NULL;
    for (uint32_t i = 0; i < (uint32_t)(sizeof s_event_nodes / sizeof s_event_nodes[0]); i++) {
        s_event_nodes[i].next = NULL;
        s_event_nodes[i].prev = NULL;
        s_event_nodes[i].obj = NULL;
        s_event_nodes[i].deadline = 0;
    }
}

static HostEventQueueNode *event_node_alloc(void)
{
    HostEventQueueNode *node;

    if (s_event_free != NULL) {
        node = s_event_free;
        s_event_free = s_event_free->next;
        node->next = NULL;
        node->prev = NULL;
        node->obj = NULL;
        node->deadline = 0;
        return node;
    }
    if (s_event_next >= (uint32_t)(sizeof s_event_nodes / sizeof s_event_nodes[0]))
        return NULL;
    return &s_event_nodes[s_event_next++];
}

static void event_node_free(HostEventQueueNode *node)
{
    if (node == NULL)
        return;
    node->prev = NULL;
    node->obj = NULL;
    node->deadline = 0;
    node->next = s_event_free;
    s_event_free = node;
}

static HostEventQueueNode *event_sentinel(void)
{
    return (HostEventQueueNode *)DAT_80065ac0;
}

static inline uint32_t mips_addu_u32(uint32_t a, uint32_t b)
{
    return a + b;
}

static int event_trace_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *env = getenv("V8_TRACE_EVENT_QUEUE");
        cached = (env != 0 && env[0] != 0 && env[0] != '0');
    }
    return cached;
}

static void event_tail_sync(void)
{
    HostEventQueueNode *sentinel = event_sentinel();
    puRam000007c4 = (sentinel->next == NULL) ? (void *)DAT_80065ac0
                                             : (void *)sentinel->prev;
}

static void event_unlink(HostEventQueueNode *node)
{
    HostEventQueueNode *sentinel = event_sentinel();

    if (node == NULL || node == sentinel)
        return;
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;
    if (sentinel->next == node)
        sentinel->next = node->next;
    if (sentinel->prev == node)
        sentinel->prev = (node->prev == sentinel) ? sentinel : node->prev;
    event_tail_sync();
    event_node_free(node);
}

static void event_remove_obj(uint32_t *obj)
{
    HostEventQueueNode *sentinel = event_sentinel();
    HostEventQueueNode *node;

    if (sentinel->prev == NULL)
        return;
    for (node = sentinel->next; node != NULL; node = node->next) {
        if (node->obj == obj) {
            event_unlink(node);
            return;
        }
    }
}

static void event_insert_sorted(HostEventQueueNode *node)
{
    HostEventQueueNode *sentinel = event_sentinel();
    HostEventQueueNode *scan;

    if (sentinel->prev == NULL) {
        sentinel->next = NULL;
        sentinel->prev = sentinel;
        sentinel->obj = NULL;
        sentinel->deadline = 0;
    }
    scan = sentinel->next;
    while (scan != NULL && scan->deadline < node->deadline)
        scan = scan->next;

    if (scan == NULL) {
        node->next = NULL;
        node->prev = sentinel->prev;
        if (node->prev == sentinel)
            sentinel->next = node;
        else
            node->prev->next = node;
        sentinel->prev = node;
    } else {
        node->next = scan;
        node->prev = scan->prev;
        scan->prev = node;
        if (node->prev == sentinel)
            sentinel->next = node;
        else
            node->prev->next = node;
    }
    event_tail_sync();
}

void ObjectEventQueue_Tick(uint32_t tick)
{
    HostEventQueueNode *sentinel = event_sentinel();

    if (sentinel->prev == NULL || sentinel->next == NULL)
        return;
    for (;;) {
        HostEventQueueNode *node = sentinel->next;
        uint32_t *obj;
        uint32_t deadline;

        if (node == NULL || tick < node->deadline)
            break;

        obj = node->obj;
        deadline = node->deadline;
        event_unlink(node);
        if (obj != 0) {
            *obj &= 0xfffffffeu;
            typedef void (*ObjTickFn)(uint32_t *, int, intptr_t);
            ObjTickFn cb = (ObjTickFn)Object_CallbackFromPsxSlot(obj);
            if (event_trace_enabled()) {
                fprintf(stderr,
                        "v8: eventq tick=%u dispatch obj=%p cb=%p flags=0x%x status=%d state=%u deadline=%u\n",
                        (unsigned)tick, (void *)obj, (void *)cb,
                        (unsigned)obj[0],
                        (int)*(int16_t *)((uint8_t *)obj + 0x06),
                        (unsigned)*(uint8_t *)((uint8_t *)obj + 0xd0),
                        (unsigned)deadline);
            }
            if (cb != (ObjTickFn)0)
                cb(obj, 2, 0);
        }
    }
}

void Host_EventQueueTick(uint32_t tick)
{
    (void)tick;
}

/* ================================================================
 * FUN_80020890 -- Object_SchedulePostEvent
 * ================================================================ */
void FUN_80020890(uint32_t *param_1, int param_2)
{
    HostEventQueueNode *node;

    if (piRam000007bc == NULL)
        piRam000007bc = (int32_t *)DAT_80065ac0;
    if ((*param_1 & 1u) != 0)
        event_remove_obj(param_1);
    node = event_node_alloc();
    if (node == NULL)
        return;
    node->obj = param_1;
    node->deadline = mips_addu_u32((uint32_t)iRam0000000c, (uint32_t)param_2);
    *param_1 |= 1u;
    event_insert_sorted(node);
    if (event_trace_enabled()) {
        fprintf(stderr,
                "v8: eventq schedule tick=%d delay=%d obj=%p flags=0x%x status=%d state=%u deadline=%u\n",
                (int)iRam0000000c, param_2, (void *)param_1,
                (unsigned)param_1[0],
                (int)*(int16_t *)((uint8_t *)param_1 + 0x06),
                (unsigned)*(uint8_t *)((uint8_t *)param_1 + 0xd0),
                (unsigned)node->deadline);
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80020890  (from analysis/SLUS_005.10/decomp/80020890.c) --- */
// addr: 0x80020890  name: FUN_80020890

void FUN_80020890(uint *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  undefined4 *puVar6;
  int iVar7;

  if ((*param_1 & 1) != 0) {
    FUN_8001fe8c(&DAT_80065ac0,param_1);
  }
  piVar4 = piRam0000076c;
  iVar3 = iRam0000000c;
  iVar7 = *piRam0000076c;
  *(undefined **)(iVar7 + 4) = &DAT_80065a70;
  piVar2 = piRam0000076c + 2;
  piRam0000076c = (int *)iVar7;
  *piVar2 = (int)param_1;
  *param_1 = *param_1 | 1;
  piVar4[3] = param_2 + iVar3;
  piVar5 = (int *)*piRam000007bc;
  piVar2 = piRam000007bc;
  while ((piVar1 = piVar5, piVar1 != (int *)0x0 && ((uint)piVar2[3] < (uint)(param_2 + iVar3)))) {
    piVar5 = (int *)*piVar1;
    piVar2 = piVar1;
  }
  puVar6 = (undefined4 *)piVar2[1];
  *puVar6 = piVar4;
  piVar2[1] = (int)piVar4;
  *piVar4 = (int)piVar2;
  piVar4[1] = (int)puVar6;
  return;
}

#endif  /* GHIDRA REF */
