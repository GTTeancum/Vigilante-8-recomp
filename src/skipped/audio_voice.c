/* audio_voice.c -- SPU voice allocation / control (OUT OF SCOPE).
 *
 * Source: SLUS_005.10  (FUN_8004410c, FUN_800443c8, FUN_800441c8,
 *                       FUN_80044054, FUN_80044574).
 *
 * Per CLAUDE.md the audio engine is rewritten; we keep these here
 * only as a documented contract. The new audio layer must satisfy:
 *
 *   Audio_VoiceAlloc -- finds the first SPU voice (1..24) whose
 *     status meets ALL of: bit-flag-clear at uRam000008fc, idle
 *     pitch (==0 in voice register), and recently-released
 *     (frame counter delta >= 2).
 *
 *   Audio_PlaySfxAtPos(chan, bank, sfxId, posXyz):
 *     - vNum = chan - 1
 *     - sample = bank + sfxId*4
 *     - SpuSetVoicePitch (vNum, *(u16*)(sample+6))
 *     - SpuSetVoiceVolume(vNum, posXyz_lo, posXyz_hi)
 *     - SpuSetVoiceStartAddr(vNum, *(u16*)(sample+4) << 3)
 *     - Audio_KeyOnOff(1, 1 << vNum)
 *     - latch the per-voice frame counter
 *
 *   Audio_VoiceStop(chan):  Audio_KeyOnOff(0, 1 << (chan-1))
 *
 *   Audio_StopAll:          clears uRam000008fc and KEY-OFF's all 24
 *
 *   SPU_VoiceVolume_Set(chan, lr):
 *     *(u32*)(0x1f801bf0 + chan*16) = lr   -- direct register write
 *
 * Bit-exact: the per-voice timing (the "recently released" gate)
 * affects which voice gets picked for new sfx, which can shift
 * apparent pan timing. The new audio layer should mimic the same
 * round-robin pattern.
 */
