/* audio_init.c -- SPU initialization (OUT OF SCOPE for 1:1).
 *
 * Source: SLUS_005.10
 *   FUN_80043ef0  -- Audio_Init
 *   FUN_80043ff0  -- Audio_KeyOnOff
 *   FUN_80044054  -- Audio_StopAllVoices
 *   FUN_80044080  -- Audio_SetMasterVolume
 *
 * Per CLAUDE.md the audio engine is out of scope -- it will be rewritten.
 * The original code is preserved here as a reference (commented) so the
 * new audio layer can mirror the SPU state the rest of the game expects.
 *
 * The seam contract:
 *   - SPU is initialised exactly once at boot.
 *   - 0x18 (24) voice slots are managed; the priority byte at
 *     DAT_800a3008 sets per-voice allocation priority for SpuInitMalloc.
 *   - Common-attr mask 0x2c3 means MVOL + CD_VOLUME + CD_MIX bits are
 *     written; cd.mix=1 enables redbook CD audio through the SPU mixer.
 *   - Default voice attrs: pitch=0x400, room=3, vol=0x3fff, attack=0,
 *     decay=0, sustain=0, release=0, sustain-level=0xf.
 *
 * These constants ARE the contract -- match them in the new audio layer
 * so the master mix sounds identical even though playback is replaced.
 */
#include <stdint.h>

#if 0
/* Original (preserved verbatim for reference, gated out of build):
 *
 * void Audio_Init(void)
 * {
 *     SsUtReverbOff();
 *     SpuInitMalloc(0x10, &DAT_800a3008);
 *     for (int i = 0x17; i >= 0; i--) DAT_800a3008[i] = (char)i;
 *     CdMix((CdlATV *)&DAT_800658b4);
 *     SpuCommonAttr a = { .mask = 0x2c3, .cd.volume = {0x3fff, 0x3fff},
 *                         .mvol = {0x3fff, 0x3fff}, .cd.mix = 1 };
 *     SpuSetCommonAttr(&a);
 *     Audio_SetMasterVolume(0, 0x2ccc, 0x2ccc);
 *     SpuVoiceAttr v = { .voice = 0xffffff, .mask = 0xff13, .pitch = 0x400,
 *                        .r_mode = 3, .volume = {0x3fff, 0x3fff},
 *                        .a_mode = 1, .s_mode = 1, .sl = 0xf };
 *     SpuSetVoiceAttr(&v);
 *     uRam000008fc = 0;
 *     Audio_RefreshKeyState();
 * }
 */
#endif
