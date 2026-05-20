/* pad_input.c -- controller input dispatch (OUT OF SCOPE).
 *
 * Per CLAUDE.md, controls / input handling is to be rewritten, not
 * decompiled. This file documents the seam so the rest of the codebase
 * type-checks; the body is intentionally not 1:1 with the original.
 *
 * The original lives at:
 *   FUN_800120d4  Pad_Tick                -- per-frame input scrape
 *   FUN_80011f8c  Pad_PortStatus          -- port plug/unplug + type
 *   FUN_80012088  Pad_DecodeButtons       -- raw pad word -> game bits
 *   FUN_8001265c  Demo_RecorderShutdown   -- flush demo buffer
 *
 * Notable behavior to preserve when rewritten:
 *   - Reads BIOS pad buffers at iVar8 = pad_index * 0x44, with two
 *     ports per buffer; entries DAT_8006ecb8..+0x44 / +0x88.
 *   - Builds button word into uRam0000062c (P1) / uRam00000630 (P2),
 *     with high half being the "just pressed" mask (XOR against
 *     uRam000005d4 previous-frame state).
 *   - Demo replay mode (iRam00000618==1): substitutes recorded input
 *     from psRam00000610 buffer; one entry = {countdown, buttons}.
 *   - Demo record mode (==2): appends current input to ring at
 *     iRam00000634; collapses consecutive identical samples.
 *   - When pad type byte == 0x23 (Dual Analog), also samples the four
 *     analog axes into a per-port analog block at DAT_80065c58..+0x400.
 *   - GTE LZC abuse: the high nibble of the pad button word is fed
 *     into LZC for jitter-resistant button-hold counters.
 *
 * These details exist so the rewritten controls module can produce
 * the same uRam0000062c bit layout, keeping all gameplay-facing
 * consumers (V8_MainLoop, ResultScreen, PauseMenu, etc.) unchanged.
 */
