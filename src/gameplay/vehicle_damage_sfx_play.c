/* vehicle_damage_sfx_play.c -- play crash/scrape SFX on damage trigger.
 *
 * Source: SLUS_005.10  FUN_8002c4bc.
 *
 * Trigger conditions: caller has accumulated enough damage so that the
 * "play-damage-SFX-once" flag (0x8000000) is clear AND the vehicle is
 * not in state 0x0c (already wrecked).
 *
 * Behaviour:
 *   1. Set flag 0x8000000 so subsequent damage in the same window
 *      doesn't re-spawn the SFX.
 *   2. Pitch-shift the SFX by a random amount: pitch = 0xb4 +
 *      (rand * 0xb4 >> 15)   -- range [0xb4, 0x168] (16-bit RTZ).
 *      Stored at +0xb0 (i16) as the per-voice pitch override.
 *   3. Set fade param at +0xa6 (i16) = 0xff88 (signed = -120) for
 *      the SFX envelope.
 *   4. Acquire a voice (FUN_8004410c) if the cached voice byte at
 *      +0x05 is zero, then play SFX bank entry 0x20 at the world pos
 *      cache (+0x24 = param_1 + 9 uint32-indexed) via FUN_8004483c.
 *   5. If the vehicle is a real entity (+0x06 short >= 0), play a
 *      second SFX (0x21) on a freshly-acquired voice and remember
 *      its index in +0x05; otherwise clear +0x05.
 *
 * Audio calls (FUN_8004410c/FUN_8004483c/FUN_800443c8) are panic stubs
 * pending audio engine wiring -- gameplay-side state is bit-exact.
 *
 * HIGH confidence (direct Ghidra port).
 */
#include <stdint.h>

extern int   FUN_80017160(void);
extern int   FUN_8004410c(void);
extern void  FUN_8004483c(int voice, uint32_t a1, int sfxId, uint32_t *pos);
extern int   FUN_800443c8(int voice, uint32_t a1, int sfxId, int a3);

extern uint32_t uRam000005f8;

void FUN_8002c4bc(uint32_t *param_1)
{
    char cVar1;
    int  iVar2, iVar3;

    if (((*param_1 & 0x8000000u) == 0) &&
        ((char)param_1[0x34] != '\x0c'))
    {
        *param_1 |= 0x8000000u;
        iVar2 = FUN_80017160();
        iVar3 = (int)*(int8_t *)((uint8_t *)param_1 + 5);

        *(int16_t *)(param_1 + 0x2c) = (int16_t)((iVar2 * 0xb4 >> 0xf) + 0xb4);
        *(uint16_t *)((uint8_t *)param_1 + 0xa6) = 0xff88u;

        if (iVar3 == 0) {
            iVar3 = FUN_8004410c();
        }
        FUN_8004483c(iVar3, uRam000005f8, 0x20, param_1 + 9);

        if (*(int16_t *)((uint8_t *)param_1 + 6) < 0) {
            *(uint8_t *)((uint8_t *)param_1 + 5) = 0;
        } else {
            cVar1 = (char)FUN_8004410c();
            *(char *)((uint8_t *)param_1 + 5) = cVar1;
            FUN_800443c8((int)cVar1, uRam000005f8, 0x21, 0);
        }
    }
}
