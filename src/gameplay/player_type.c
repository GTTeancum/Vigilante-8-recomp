/* player_type.c -- classify a player slot by the first character of its name.
 *
 * Source: SLUS_005.10  FUN_80011f8c.
 *
 * The per-slot record at DAT_80066458 + slot*0x22 is 0x22 (34) bytes
 * wide -- typical pad-name structure with a "occupied?" flag at +0x00
 * and a name byte at +0x01. The first name character encodes the
 * player kind:
 *
 *   '#' (0x23) -- AI bot         -> 3
 *   'A' (0x41) -- ???            -> 2
 *   'S' (0x53) -- secret /split? -> 4
 *   's' (0x73) -- lowercase var  -> 5
 *   <anything else>              -> 1   (regular human player)
 *   if slot is empty             -> 0
 *
 * MED confidence: the mapping is the original code's exactly, but the
 * NAME of each numeric class isn't proven from this site alone. Pass 2
 * should cross-reference every caller's switch on the return value.
 */
#include <stdint.h>

extern uint8_t DAT_80066458[];   /* per-slot record base */

uint32_t Player_GetType(int slot)
{
    const uint8_t *r = DAT_80066458 + slot * 0x22;
    if (r[0] != 0) return 0;       /* slot empty -- 0 means "no player" */
    uint8_t c = r[1];
    switch (c) {
        case '#': return 3;
        case 'A': return 2;
        case 'S': return 4;
        case 's': return 5;
        default:  return 1;
    }
}
