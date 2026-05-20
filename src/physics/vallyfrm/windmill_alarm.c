/* windmill_alarm.c -- Valley Farms windmill alarm dispatcher.
 *
 * Source: VALLYFRM.DLL  FUN_8010031c.
 *
 * Same dispatcher idiom as Hoover Dam siren_strobe.c (ring-buffer of
 * pose snapshots at self+0x22 -> 8 cells x 0x80 bytes), applied to
 * the windmill. Drains one ring slot per countdown into shared
 * display cell DAT_8008f660; on schedule fires an alarm-sprite
 * (0x184/0x185/0x186 by flag bits) at the impact point with target
 * yaw, then binds FX channel.
 *
 * mode dispatch (fall-through chain):
 *   0    -- per-frame: drain ring buffer + reseed DAT_80101304
 *   0x11 -- if windmill (self+6==0x61), arm path slot 0x61 at offset
 *           0x8f80
 *   1    -- post-spawn: pick path waypoint (Path_Pick @ 0x80065a50)
 *   2    -- schedule first 240-frame fire
 *   9    -- every 16 frames, spawn alarm sprite at impact
 *   0xb  -- bind FX channel
 *   7    -- retire FX
 *   4    -- free ring buffer
 *
 * MED.
 */
#include <stdint.h>

extern int Rand255(void);
extern uint32_t Path_Pick(uint32_t bank, int seed);
extern int Path_AttachToObj(uint32_t flag, uint32_t path);
extern void Damage_StandardVehicleAlt(uint32_t *self, uint32_t *imp);
extern void VF_QueueDamage(void);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern uint32_t Spawner_FindSlot(int idx);
extern void Path_ArmAt(uint32_t pathSlot, uint32_t offset);
extern uint32_t *Object_FetchPrevHead(uint32_t *self);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern int  Math_Atan2_Pos(int xy);
extern void Object_Suspend(void);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux);
extern void Object_RetireFX(int port);
extern void Heap_Free(uint32_t p);

extern uint32_t DAT_80101304, DAT_80101308;
extern uint8_t  DAT_8008f660;
extern uint32_t _DAT_80065a10, _DAT_800659fc;

uint32_t VF_WindmillAlarm(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    switch (mode) {
    case 0:
        if (arg) {
            uint32_t v = self[0x20];
            self[0x20] = v - (uintptr_t)arg;
            if ((int)(v - (uintptr_t)arg) < 0) {
                do {
                    uint32_t idx = self[0x21];
                    uint32_t *src = (uint32_t *)((idx & 7) * 0x80 + self[0x22]);
                    self[0x20] += 6;
                    uint32_t *dst = (uint32_t *)&DAT_8008f660;
                    for (int i = 0; i < 8; i++) {
                        dst[0] = src[0]; dst[1] = src[1];
                        dst[2] = src[2]; dst[3] = src[3];
                        src += 4; dst += 4;
                    }
                    self[0x21] = idx + 1;
                } while ((int)self[0x20] < 0);
            }
        }
        DAT_80101304 = (uint32_t)Rand255();
        /* fall through */
    case 0x11:
        if (*(int16_t *)((char *)self + 6) == 0x61) {
            uint32_t slot = Spawner_FindSlot(0x61);
            if (slot != 0) Path_ArmAt(slot + 0xc, 0x8f80);
        }
        Damage_StandardVehicleAlt(self, arg);
        /* fall through */
    case 1: {
        *self = 0x80u;
        self[0x22] = 0x80u;
        uint32_t path = Path_Pick(0x80065a50u, 0x100);
        _DAT_80065a10 = (Path_AttachToObj(0x7f000000u, path) != 0);
        DAT_80101308 = 0;
    }
    /* fall through */
    case 2:
        VF_QueueDamage();
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0xf0);
        /* fall through */
    case 9:
        if ((char)self[1] == 8 &&
            (DAT_80101304 = DAT_80101304 + 1, (DAT_80101304 & 0xf) == 0)) {
            uint32_t *head = Object_FetchPrevHead(self);
            uint32_t kind  = 0x186u;
            if (0xee5 < *(int *)((uintptr_t)head + 0x8c)) {
                kind = 0x185u;
                if (*self & 0x80000) kind = 0x184u;
            }
            uint32_t *p = (uint32_t *)Object_SpawnFromBank(
                *(uint32_t *)(_DAT_800659fc + 0x58), kind, 0x80, 8);
            *(uint8_t *)(p + 1) = 1;
            *p = 0x24u;
            p[0x12] = arg[0]; p[0x13] = arg[1]; p[0x14] = arg[2];
            *(uint16_t *)((char *)p + 0x42) =
                (uint16_t)Math_Atan2_Pos((int)(uintptr_t)((uintptr_t)head + 0x10));
            p[0x19] = 0x8003e80cu;
            Object_Suspend();
            self = (uint32_t *)1;
        case 0xb:
            *(uint8_t *)self = Pool_AllocSFX();
            Pool_BindFXOnObject((uintptr_t)self & 0xff,
                                *(uint32_t *)(*(int *)(_DAT_800659fc + 0x58) + 8), 4, 0);
        case 7:
            Object_RetireFX(0x8c);
        case 4:
            Heap_Free(self[0x22]);
        }
        break;
    default:
        break;
    }
    return 0;
}
