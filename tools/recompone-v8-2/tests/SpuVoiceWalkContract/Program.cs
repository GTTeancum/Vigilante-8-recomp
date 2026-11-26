// Source-only PCM and device-state contract; no game, disc, host audio or external dependencies.
using System.Security.Cryptography;
using RecompOne.Runtime;

const uint reg = 0x1F801C00, master = reg + 0x180;
const uint kon = reg + 0x188, koff = reg + 0x18C;
const uint pmon = reg + 0x190, non = reg + 0x194;
const uint transfer = reg + 0x1A6, control = reg + 0x1AA;

void WriteVoice(Spu s, int i, uint field, ushort value) => s.WriteReg16(reg + (uint)(i * 16) + field, value);
void WriteMask(Spu s, uint address, uint mask)
{
    s.WriteReg16(address, (ushort)mask);
    s.WriteReg16(address + 2, (ushort)(mask >> 16));
}
uint Slot(int i) => i == 23 ? Spu.RamSize - 8u : 0x4000u + (uint)i * 0x100u;
static byte[] Blocks(uint seed, bool loops)
{
    byte[] data = new byte[64];
    for (int i = 0; i < data.Length; i++)
    {
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        data[i] = (byte)(seed >> 24);
    }
    for (int i = 0; i < data.Length; i += 16) data[i + 1] = 0;
    data[1] = 4;                                // loop start
    data[loops ? 49 : 17] = (byte)(loops ? 3 : 1); // loop or one-shot
    return data;
}
Spu Setup()
{
    var s = new Spu();
    s.WriteReg16(master, 0x3FFF); s.WriteReg16(master + 2, 0x392A);
    s.WriteReg16(transfer, 0xCDEF); s.WriteReg16(control, 0x3900);
    for (int i = 0; i < 24; i++)
    {
        s.DmaWrite(Slot(i), Blocks((uint)(i + 1), true));
        WriteVoice(s, i, 0, (ushort)(0x1100 + i * 11));
        WriteVoice(s, i, 2, (ushort)(0x2100 - i * 13));
        WriteVoice(s, i, 4, (ushort)(0x1225 - i * 31));
        WriteVoice(s, i, 6, (ushort)(Slot(i) >> 3));
        WriteVoice(s, i, 8, (ushort)(0x000F - i % 12));
        WriteVoice(s, i, 10, (ushort)(0x2006 + i % 9));
    }
    return s;
}
string WithTrace(Action<Spu, Action<int>> exercise)
{
    var s = Setup();
    using var bytes = new MemoryStream();
    using var w = new BinaryWriter(bytes);
    var voices = new Spu.VoiceDebug[24];
    var pcm = new short[8192];
    exercise(s, frames =>
    {
        s.Mix(pcm, frames);
        for (int i = 0; i < frames * 2; i++) w.Write(pcm[i]);
        s.CaptureDebug(voices, out var state);
        w.Write(state.Endx); w.Write(state.TransferAddr); w.Write(state.Spucnt);
        for (int i = 0; i < voices.Length; i++)
        {
            var v = voices[i];
            w.Write((byte)v.Phase); w.Write(v.AdsrVol);
            w.Write(v.StartAddr); w.Write(v.RepeatAddr); w.Write(v.CurAddr);
            w.Write(v.Pitch); w.Write(v.VolL); w.Write(v.VolR);
            w.Write(v.Pmod); w.Write(v.Noise); w.Write(v.EndX);
            w.Write(s.ReadReg16(reg + (uint)(i * 16) + 0xC));
        }
    });
    return Convert.ToHexString(SHA256.HashData(bytes.GetBuffer().AsSpan(0, (int)bytes.Length)));
}

void Sparse(Spu s, Action<int> record)
{
    // Voices 0/2 and 15/16/18 exercise holes and adjacency across the mask halves.
    const uint sparse = 0x858005;
    WriteMask(s, pmon, 0xFFFFFF);
    record(4096); // Noise timing must still progress while all voices are idle.
    WriteMask(s, non, 1u << 15);
    WriteMask(s, kon, sparse | 0x8000_0000); // High bits do not name a physical voice.
    record(1); record(701);
    WriteVoice(s, 16, 0xE, (ushort)(Slot(16) >> 3));
    record(378);
    WriteMask(s, koff, sparse & ~(1u << 18));
    for (int i = 0; i < 4; i++) record(4096);
    WriteMask(s, kon, 1u << 16); // Reuse a released slot.
    record(1023);
    WriteMask(s, koff, 0xFFFFFF);
    for (int i = 0; i < 4; i++) record(4096);
    // Writes to an off voice's envelope do not start it.
    WriteVoice(s, 0, 0xC, 0x7000);
    record(41);
    WriteMask(s, kon, 1u << 0);
    record(72);
}

void ReleaseAtBlockBoundary(Spu s, Action<int> record)
{
    const uint bit = 1u << 16;
    s.DmaWrite(Slot(16), Blocks(141, false));
    WriteVoice(s, 16, 4, 0x4000);
    WriteMask(s, kon, bit);
    record(6);
    WriteVoice(s, 16, 0xC, 1);
    WriteVoice(s, 16, 0xA, 0);
    WriteMask(s, koff, bit);
    record(1); // ADSR reaches Off but the next non-loop block resets it to Release.
    var debug = new Spu.VoiceDebug[24];
    s.CaptureDebug(debug, out var state);
    if (debug[16].Phase != Spu.AdsrPhase.Release || (state.Endx & bit) == 0)
        throw new Exception("lost final block decode on release");
    record(1);
    s.CaptureDebug(debug, out _);
    if (debug[16].Phase != Spu.AdsrPhase.Off)
        throw new Exception("last release tick was not scheduled");
    WriteMask(s, kon, bit);
    record(12);
}

void Mixed(Spu s, Action<int> record)
{
    uint state = 0x9E3779B9;
    uint R() { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
    WriteMask(s, kon, 0xFFFFFF);
    record(1939);
    for (int i = 0; i < 768; i++)
    {
        int voice = (int)(R() % 24);
        uint bits = (1u << voice) | (1u << (int)(R() % 24));
        switch (i % 12)
        {
            case 0: WriteMask(s, koff, bits); break;
            case 1: WriteMask(s, kon, bits); break;
            case 2: WriteMask(s, pmon, R()); break;
            case 3: WriteMask(s, non, R()); break;
            case 4: WriteVoice(s, voice, 0, (ushort)R()); break;
            case 5: WriteVoice(s, voice, 2, (ushort)R()); break;
            case 6: WriteVoice(s, voice, 4, (ushort)R()); break;
            case 7: WriteVoice(s, voice, 8, (ushort)R()); break;
            case 8: WriteVoice(s, voice, 10, (ushort)R()); break;
            case 9: s.WriteReg16(control, (ushort)R()); break;
            case 10: s.DmaWrite(Slot(voice), Blocks(R(), (voice & 1) == 0)); break;
            case 11: s.WriteReg16(master + 2, (ushort)R()); break;
        }
        record((int)(R() % 117) + 1);
    }
}

string AtomicStart()
{
    // Exercise the same shared mixer lock used by the selector's upload and
    // native key-on. The second thread asks to mix before the key-on can run.
    var s = Setup();
    short[] pcm = new short[1024];
    using var hasLock = new ManualResetEventSlim();
    using var attempted = new ManualResetEventSlim();
    var mixer = new Thread(() =>
    {
        if (!hasLock.Wait(TimeSpan.FromSeconds(10))) throw new Exception("start did not begin");
        attempted.Set();
        s.Mix(pcm, pcm.Length / 2);
    });
    mixer.Start();
    s.LoadRamAndStart(Slot(16), Blocks(17, true), () =>
    {
        hasLock.Set();
        if (!attempted.Wait(TimeSpan.FromSeconds(10))) throw new Exception("mixer did not attempt read");
        WriteMask(s, kon, (1u << 0) | (1u << 16));
    });
    if (!mixer.Join(TimeSpan.FromSeconds(10))) throw new Exception("mixer did not finish");
    if (pcm.All(sample => sample == 0)) throw new Exception("two started voices never reached mixer");
    using var bytes = new MemoryStream();
    using var w = new BinaryWriter(bytes);
    foreach (short sample in pcm) w.Write(sample);
    return Convert.ToHexString(SHA256.HashData(bytes.ToArray()));
}

static void Check(string label, string actual, string expected)
{
    Console.WriteLine($"{label}: {actual}");
    if (actual != expected) throw new Exception($"{label}: expected {expected}");
}

void CountSlots()
{
#if COUNT_SLOT_VISITS
    foreach (int n in new[]{0, 1, 4, 8, 16, 24})
    {
        var s = Setup();
        uint mask = 0;
        // Stride coprime to 24 makes sparse sets cover distant and high voices.
        for (int i = 0; i < n; i++) mask |= 1u << (i * 7 % 24);
        WriteMask(s, kon, mask);
        short[] pcm = new short[512];
        const int buffers = 96, frames = 256;
        Spu.SlotVisits = 0;
        for (int i = 0; i < buffers; i++) s.Mix(pcm, frames);
        var voices = new Spu.VoiceDebug[24];
        s.CaptureDebug(voices, out _);
        if (voices.Count(v => v.Phase != Spu.AdsrPhase.Off) != n)
            throw new Exception($"steady mix lost one of the {n} looping voices");
        Console.WriteLine($"voices={n} frames={frames * buffers} visits={Spu.SlotVisits}");
    }
#else
    throw new InvalidOperationException("Slot census requires an instrumented copy of Spu.cs");
#endif
}

if (args.Contains("--slot-count")) CountSlots();
else
{
    // Digests come from the unchanged maintenance9 SPU source.
    Check("sparse/adjacency/noise", WithTrace(Sparse), "4DF6B277FF8349BF1E5505C77CDB4C6CAD3558A83E4964C6BE9C7D0E4C0DA12A");
    Check("release/final decode/rekey", WithTrace(ReleaseAtBlockBoundary), "DCE6F2F0E29EF6124E34821A8B3EFC5D18ED3B34D280856D4B4E7E17E71F6C1E");
    Check("dense/ADSR/changing voices/wrapped RAM", WithTrace(Mixed), "C4605D94A99738175600D4255303117B16646B04FA528E4853835B101C3E3855");
    Check("locked selector upload and key-on", AtomicStart(), "1D2F3E1C61E2D52C970B2DF1BD2D090F9C396F704F144129A344CCE755E82544");
}

namespace RecompOne.Runtime.Sdk
{
    internal static class LibCd
    {
        // The linked queues stay idle. Fail on any unexpected disc input.
        internal static void MixCdInput(short left, short right, out int mixL, out int mixR) =>
            throw new InvalidOperationException("SPU voice contract must not use CD audio");
    }
}
