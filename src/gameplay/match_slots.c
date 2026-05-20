/* match_slots.c -- per-slot record setters used by character select.
 *
 * Source: SLUS_005.10  FUN_80012028, FUN_80012050, FUN_80012068.
 *
 * The match-slot table at DAT_80065944 is an array of 8-byte records.
 * The setters expose offsets 0..3 of each record by name. The slot
 * size of 8 lines up with g_playerSlotIdx (also 8 entries) but the
 * layout per slot is different:
 *
 *   +0 character       (the vehicle / pilot id)
 *   +1 color           (paint / palette index)
 *   +2 secondaryA
 *   +3 secondaryB
 *
 * HIGH on layout (offset 0 vs 1..3 is unambiguous from FUN_80012050 vs
 * FUN_80012068). MED on field names -- pass 2 should cross-reference
 * with the char-select UI.
 */
#include <stdint.h>

extern uint8_t DAT_80065944[];  /* slot array base, 8 bytes per slot */

void MatchSlot_SetAll(int slot, uint8_t character, uint8_t color,
                      uint8_t secondaryA, uint8_t secondaryB)
{
    uint8_t *r = DAT_80065944 + slot * 8;
    r[0] = character;
    r[1] = color;
    r[2] = secondaryA;
    r[3] = secondaryB;
}

void MatchSlot_SetCharacter(int slot, uint8_t character)
{
    DAT_80065944[slot * 8] = character;
}

void MatchSlot_SetTrim(int slot, uint8_t color, uint8_t secondaryA, uint8_t secondaryB)
{
    uint8_t *r = DAT_80065944 + slot * 8;
    r[1] = color;
    r[2] = secondaryA;
    r[3] = secondaryB;
}
