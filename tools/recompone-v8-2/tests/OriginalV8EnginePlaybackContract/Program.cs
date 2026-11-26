// These are synthesized ADPCM blocks, not disc material. No device/game/UI.
using RecompOne.Runtime;

const uint Registers = 0x1F801C00u;
int checks = 0;
void Check(bool ok, string name)
{
    if (!ok) throw new InvalidOperationException(name);
    checks++;
}
void Voice(Spu s, int slot, uint field, ushort value) =>
    s.WriteReg16(Registers + ((uint)slot << 4) + field, value);
void Key(Spu s, uint mask, bool start = true)
{
    uint address = Registers + (start ? 0x188u : 0x18Cu);
    s.WriteReg16(address, (ushort)mask);
    s.WriteReg16(address + 2, (ushort)(mask >> 16));
}
Spu Setup(int slot, uint logicalStart, ReadOnlySpan<byte> bytes)
{
    var s = new Spu();
    s.DmaWrite(logicalStart, bytes);
    s.WriteReg16(Registers + 0x180u, 0x3FFF);
    s.WriteReg16(Registers + 0x182u, 0x3555);
    Voice(s, slot, 0, 0x1900); Voice(s, slot, 2, 0x2500);
    Voice(s, slot, 4, 1024); Voice(s, slot, 6, (ushort)(logicalStart >> 3));
    Voice(s, slot, 8, 15); Voice(s, slot, 10, 6);
    return s;
}
short[] Mix(Spu s, int frames)
{
    var pcm = new short[frames * 2];
    s.Mix(pcm, frames);
    return pcm;
}
Spu.VoiceDebug Snapshot(Spu s, int voice)
{
    var voices = new Spu.VoiceDebug[24];
    s.CaptureDebug(voices, out _);
    return voices[voice];
}
void Equal(Spu expected, Spu variant, int slot, int frames, string label)
{
    Check(Mix(expected, frames).SequenceEqual(Mix(variant, frames)), label + " exact stereo PCM");
    Check(Snapshot(expected, slot).Equals(Snapshot(variant, slot)), label + " exact native cursor and voice registers");
}
static byte[] Clip(int seed, bool loop)
{
    var bytes = new byte[64];
    new Random(seed).NextBytes(bytes);
    for (int i = 0; i < bytes.Length; i += 16)
    {
        bytes[i] = (byte)(i % 32 == 0 ? 0x22 : 0x08);
        bytes[i + 1] = 0;
    }
    bytes[17] = loop ? (byte)6 : (byte)1;
    bytes[49] = loop ? (byte)3 : (byte)7; // One-shot trailer is unreachable.
    return bytes;
}

byte[] legacyEffect = Clip(47, false);
byte[] donor = Clip(29, true), donorBefore = (byte[])donor.Clone();
var immutable = new Spu.EngineAdpcm(donor);
donor.AsSpan().Fill(0); // Neither caller aliasing nor RAM uploads may change donor bytes.
Check(immutable.Blocks.SequenceEqual(donorBefore), "owned engine bytes are immutable");

foreach (var (slot, start) in new[] { (0, 0x4000u), (16, 0xA000u), (23, (uint)Spu.RamSize - 8u) })
{
    uint mask = 1u << slot;
    var reference = Setup(slot, start, donorBefore);
    var silentLegacy = Setup(slot, start, legacyEffect);
    var candidate = Setup(slot, start, legacyEffect);
    byte[] ramBefore = (byte[])candidate.Ram.Clone();
    Key(reference, mask); Key(silentLegacy, mask);
    Check(candidate.StartWithEngineAdpcm(slot, immutable, () => Key(candidate, mask)), "expected voice consumed source");
    Equal(reference, candidate, slot, 4096, "entry, wrap and repeat");
    Check(Snapshot(candidate, slot).RepeatAddr == (ushort)((start + 16) >> 3), "native repeat register retained");
    Check(Mix(silentLegacy, 4096).Skip(4000).All(x => x == 0), "legacy effect ends at first no-repeat flag");
    Check(Mix(candidate, 4096).Skip(4000).Any(x => x != 0), "same voice sustains beyond one-shot");
    _ = Mix(reference, 4096);
    Voice(candidate, slot, 4, 743); Voice(reference, slot, 4, 743);
    Voice(candidate, slot, 0, 0x1000); Voice(reference, slot, 0, 0x1000);
    Voice(candidate, slot, 2, 0x2B90); Voice(reference, slot, 2, 0x2B90);
    Equal(reference, candidate, slot, 719, "native live pitch and gain writes");
    Check(ramBefore.SequenceEqual(candidate.Ram), "original source writes zero SPU RAM bytes");

    Key(reference, mask, false); Key(candidate, mask, false);
    Equal(reference, candidate, slot, 16384, "native key-off and release");
    Check(Snapshot(candidate, slot).Phase == Spu.AdsrPhase.Off, "ended after release");

    reference.DmaWrite(start, legacyEffect);
    Key(reference, mask); Key(candidate, mask);
    Equal(reference, candidate, slot, 4096, "ordinary key-on after release uses native RAM again");
    Check(candidate.StartWithEngineAdpcm(slot, immutable, () => Key(candidate, mask)), "rematch may attach a fresh source");
    reference.DmaWrite(start, donorBefore); Key(reference, mask);
    Equal(reference, candidate, slot, 811, "same voice can start donor again");

    reference.DmaWrite(start, legacyEffect);
    Key(reference, mask); Key(candidate, mask);
    Equal(reference, candidate, slot, 4096, "ordinary key-on clears active binding immediately");
}

var mixed = Setup(16, 0xA000, legacyEffect);
var nativeOnly = Setup(16, 0xA000, legacyEffect);
// A no-op native call consumes nothing and must not leak to the next event.
int calls = 0;
Check(!mixed.StartWithEngineAdpcm(16, immutable, () => calls++) && calls == 1, "missing key-on ran native method once");
Key(mixed, 1u << 16); Key(nativeOnly, 1u << 16);
Equal(nativeOnly, mixed, 16, 400, "missing key-on leaves no pending sample");
try { mixed.StartWithEngineAdpcm(16, immutable, () => throw new IOException("fixture")); }
catch (IOException) { calls++; }
Check(calls == 2, "native exception preserved");
Key(mixed, 1u << 16); Key(nativeOnly, 1u << 16);
Equal(nativeOnly, mixed, 16, 400, "exception leaves no pending sample");
Check(mixed.StartWithEngineAdpcm(16, immutable, () =>
{
    Key(mixed, 1u << 16);
    Key(mixed, 1u << 16); // Second event in same native scope is an ordinary rekey.
}), "first event consumed private source exactly once");
Key(nativeOnly, 1u << 16);
Equal(nativeOnly, mixed, 16, 400, "same-scope second event does not inherit the donor");

var wrongSlot = Setup(0, 0x4000, legacyEffect);
var expectedWrongSlot = Setup(0, 0x4000, legacyEffect);
Check(!wrongSlot.StartWithEngineAdpcm(16, immutable, () => Key(wrongSlot, 1)), "key-on for different slot cannot consume pending source");
Key(expectedWrongSlot, 1);
Equal(expectedWrongSlot, wrongSlot, 0, 2048, "unrelated slot and native clip untouched");

var bounded = Setup(0, 0x4000, legacyEffect);
Check(bounded.StartWithEngineAdpcm(0, immutable, () => Key(bounded, 1)), "bounds control started");
Voice(bounded, 0, 0xE, 0x2222); // Explicitly point native repeat outside the clip.
var pcm = Mix(bounded, 4096);
Check(pcm.Skip(5000).All(x => x == 0) && Snapshot(bounded, 0).Phase == Spu.AdsrPhase.Off,
    "corrupt repeat stops only that voice instead of decoding unrelated RAM");

// All 13 identities may use unique private clip instances simultaneously;
// an ordinary spell of audio can even share the same physical start address.
var crowded = Setup(0, 0x5000, legacyEffect);
var nativeControl = Setup(0, 0x5000, legacyEffect);
var beforeCrowded = (byte[])crowded.Ram.Clone();
for (int slot = 0; slot < 14; slot++)
{
    uint raw = slot == 13 ? 0x5000u : 0x6000u + (uint)slot * 0x100u;
    byte[] authored = Clip(538 + slot, true);
    if (slot != 13) nativeControl.DmaWrite(raw, authored);
    foreach (var s in new[] { crowded, nativeControl })
    {
        Voice(s, slot, 0, 0x600); Voice(s, slot, 2, 0xB00);
        Voice(s, slot, 4, (ushort)(743 + 21 * slot));
        Voice(s, slot, 6, (ushort)((s == crowded ? 0x5000 : raw) >> 3));
        Voice(s, slot, 8, 0x000F); Voice(s, slot, 10, 6);
    }
    uint mask = 1u << slot;
    Key(nativeControl, mask);
    if (slot == 13) Key(crowded, mask);
    else Check(crowded.StartWithEngineAdpcm(slot, new Spu.EngineAdpcm(authored), () => Key(crowded, mask)),
        "independent engine keyed in crowded mixer");
}
Check(Mix(crowded, 8192).SequenceEqual(Mix(nativeControl, 8192)),
    "13 private voices and one native voice at shared logical address match independent native clips");
Check(beforeCrowded.SequenceEqual(crowded.Ram), "crowded match consumes zero SPU RAM bytes");
Key(crowded, 0xFFFFFF, false); Key(nativeControl, 0xFFFFFF, false);
Check(Mix(crowded, 16384).SequenceEqual(Mix(nativeControl, 16384)), "all voices release identically on teardown");

var ready = Setup(0, 0x4000, legacyEffect);
var readyLegacy = Setup(0, 0x4000, legacyEffect);
foreach (var s in new[] { ready, readyLegacy })
{
    Voice(s, 0, 0, 0); Voice(s, 0, 2, 0);
}
Check(ready.StartWithEngineAdpcm(0, immutable, () => Key(ready, 1)), "quiet pre-game engine key-on");
Key(readyLegacy, 1);
Check(Mix(ready, 8192).Concat(Mix(readyLegacy, 8192)).All(x => x == 0), "same zero-volume ready interval");
foreach (var s in new[] { ready, readyLegacy })
{
    Voice(s, 0, 0, 0x2000); Voice(s, 0, 2, 0x2000);
}
Check(Mix(ready, 512).Any(x => x != 0) && Mix(readyLegacy, 512).All(x => x == 0),
    "normal gain rise makes donor audible later but cannot revive terminated legacy effect");

var atomic = Setup(16, 0xA000, legacyEffect);
var atomicNative = Setup(16, 0xA000, donorBefore);
using var startRequested = new ManualResetEventSlim();
using var mixAttempted = new ManualResetEventSlim();
short[]? mixedTogether = null;
var mixer = new Thread(() =>
{
    if (!startRequested.Wait(TimeSpan.FromSeconds(5))) return;
    mixAttempted.Set();
    mixedTogether = Mix(atomic, 1024);
});
mixer.Start();
Check(atomic.StartWithEngineAdpcm(16, immutable, () =>
{
    startRequested.Set();
    if (!mixAttempted.Wait(TimeSpan.FromSeconds(5))) throw new InvalidOperationException("mixer did not attempt lock");
    Key(atomic, 1u << 16);
}), "mix and native key-on share one lock");
Check(mixer.Join(TimeSpan.FromSeconds(5)) && mixedTogether != null, "mixer saw complete binding");
Key(atomicNative, 1u << 16);
Check(Mix(atomicNative, 1024).SequenceEqual(mixedTogether!), "first concurrently mixed block is exact");

Console.WriteLine($"[OriginalV8EnginePlaybackContract] PASS checks={checks}");

namespace RecompOne.Runtime.Sdk
{
    internal static class LibCd
    {
        internal static void MixCdInput(short left, short right, out int mixL, out int mixR) =>
            throw new InvalidOperationException("Engine lifetime fixture must not use CD audio");
    }
}
