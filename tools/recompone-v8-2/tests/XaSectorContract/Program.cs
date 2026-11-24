// Synthetic sectors only. Golden digests come from the unchanged v5 XA decoder.
using System.Diagnostics;
using System.Buffers.Binary;
using System.Security.Cryptography;
using RecompOne.Runtime;

static byte[] Sector(uint seed)
{
    byte[] sector = new byte[2336];
    for (int i = 0; i < sector.Length; i++)
    {
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        sector[i] = (byte)(seed >> 24);
    }
    return sector;
}

static void Feed(int seed, byte coding) =>
    XaAudio.DecodeSector(Sector(checked((uint)seed * 997u + 1)), 8, coding, seed, 3, 5);

static byte[] Constant(byte packedNibbles, byte shift)
{
    byte[] sector = new byte[2336];
    for (int group = 0; group < 18; group++)
    {
        int offset = 8 + group * 128;
        Array.Fill(sector, shift, offset + 4, 8); // filter zero: output independent of predictor history
        Array.Fill(sector, packedNibbles, offset + 16, 112);
    }
    return sector;
}

static void Expect(string name, string observed, string expected)
{
    Console.WriteLine($"{name}: {observed}");
    if (observed != expected) throw new Exception($"{name}: expected {expected}");
}

static string Playback(bool overflow)
{
    XaAudio.Reset();
    using var bytes = new MemoryStream();
    using var w = new BinaryWriter(bytes);
    void State()
    {
        w.Write(XaAudio.Playing);
        w.Write(XaAudio.BufferedSamples);
        w.Write(XaAudio.SourceRate);
    }
    bool Next()
    {
        bool valid = XaAudio.Next(out short left, out short right);
        w.Write(valid); w.Write(left); w.Write(right);
        return valid;
    }
    void Take(int n) { for (int i = 0; i < n; i++) Next(); State(); }
    void Finish()
    {
        int i = 0;
        while (Next()) if (++i > 800000) throw new Exception("XA never stopped");
        State();
    }

    // Reset intentionally leaves SourceRate unchanged. Feed before comparing it.
    Next();
    if (!overflow)
    {
        foreach (byte coding in new byte[] { 0, 1, 4, 5, 0, 5 })
        {
            Feed(1 + coding * 29, coding);
            State(); Take(311 + coding);
        }
        // Refill from an underrun without resetting interpolation or predictors.
        while (XaAudio.BufferedSamples > 0) Next();
        Take(103); Feed(314, 1); Take(257); Finish();
        // Restart from a full stop, then interrupt with reset while samples remain.
        Feed(315, 4); Take(307); XaAudio.Reset(); Take(9);
        Feed(316, 5); Finish();
    }
    else
    {
        // Partly consume, wrap, then overwrite unread frames. Vary the waveform
        // each sector so retaining the wrong old prefix or channel changes the digest.
        ReadOnlySpan<byte> modes = [1, 4, 5, 0];
        for (int i = 1; i <= 181; i++)
        {
            Feed(i, modes[i % 4]);
            if (i % 7 == 0) Take(i * 5 + 1); else State();
        }
        Finish();
    }
    return Convert.ToHexString(SHA256.HashData(bytes.GetBuffer().AsSpan(0, (int)bytes.Length)));
}

static byte[] Capture(Action feed)
{
    string wav = Path.Combine(Path.GetTempPath(), $"xa-sector-{Guid.NewGuid():N}.wav");
    string? old = Environment.GetEnvironmentVariable("RECOMPONE_XA_CAPTURE");
    try
    {
        XaAudio.Reset();
        Environment.SetEnvironmentVariable("RECOMPONE_XA_CAPTURE", wav);
        XaAudio.BeginDiagnosticCapture("XA_CONTRACT");
        feed();
        XaAudio.EndDiagnosticCapture();
        return File.ReadAllBytes(wav);
    }
    finally
    {
        XaAudio.EndDiagnosticCapture();
        Environment.SetEnvironmentVariable("RECOMPONE_XA_CAPTURE", old);
        File.Delete(wav);
    }
}

static void Concurrent()
{
    // DecodeSector has callers on both the CD-XA and CD-stream threads. Check
    // that simultaneously decoded sectors cannot overwrite each other's scratch.
    const int pairs = 36, frames = 2016, plus = (2 << 16) | 1, minus = (-5 << 16) | 0xfff9;
    byte[] wav = Capture(() =>
    {
        using var barrier = new Barrier(2);
        void Producer(byte[] input)
        {
            for (int i = 0; i < pairs; i++)
            {
                if (!barrier.SignalAndWait(TimeSpan.FromSeconds(10))) throw new Exception("producer stalled");
                XaAudio.DecodeSector(input, 8, 1);
            }
        }
        var peer = new Thread(() => Producer(Constant(0xb9, 12)));
        peer.Start(); Producer(Constant(0x21, 12)); peer.Join();
    });
    if (wav.Length != 44 + pairs * 2 * frames * 4) throw new Exception("partial sector");
    int positives = 0;
    for (int off = 44; off < wav.Length; off += frames * 4)
    {
        int value = BinaryPrimitives.ReadInt32LittleEndian(wav.AsSpan(off));
        if (value == plus) positives++; else if (value != minus) throw new Exception("wrong channels");
        for (int j = 1; j < frames; j++)
            if (BinaryPrimitives.ReadInt32LittleEndian(wav.AsSpan(off + j * 4)) != value)
                throw new Exception("overwritten sector");
    }
    if (positives != pairs) throw new Exception("lost sectors");
    Console.WriteLine("concurrent producers: 72 intact sectors");
}

static void TracePeaks()
{
    if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_AUDIO") != "1")
        throw new Exception("Run --trace-peaks with RECOMPONE_TRACE_AUDIO=1");
    XaAudio.Reset();
    TextWriter old = Console.Error;
    using var log = new StringWriter();
    Console.SetError(log);
    try
    {
        for (int i = 1; i <= 38; i++)
        {
            bool mono = i is 1 or 18;
            byte[] sec = mono ? Constant(0x88, 0) : i == 38 ? Constant(0xf7, 2) : Constant(0x21, 12);
            XaAudio.DecodeSector(sec, 8, mono ? (byte)0 : (byte)1, i, 2, 7);
        }
    }
    finally { Console.SetError(old); }
    Expect("traced diagnostic peaks", log.ToString().ReplaceLineEndings("\n").TrimEnd(),
        "[XA] file=2 channel=7 lba=1 sectors=1 rate=37800 frames=4032 buffered=4032 peak=32768\n" +
        "[XA] file=2 channel=7 lba=19 sectors=19 rate=37800 frames=2016 buffered=42336 peak=2\n" +
        "[XA] file=2 channel=7 lba=38 sectors=38 rate=37800 frames=2016 buffered=80640 peak=7168");
}

static void Bench(byte coding, bool requireZero)
{
    const int warmup = 256, measured = 4096;
    byte[][] inputs = Enumerable.Range(0, 32).Select(i => Sector((uint)i + 1)).ToArray();
    XaAudio.Reset();
    for (int i = 0; i < warmup; i++) XaAudio.DecodeSector(inputs[i & 31], 8, coding);
    long start = Stopwatch.GetTimestamp();
    long allocated = GC.GetAllocatedBytesForCurrentThread();
    for (int i = 0; i < measured; i++)
    {
        // Avoid benchmarking just the permanently-full ring path.
        if ((i & 31) == 0) XaAudio.Reset();
        XaAudio.DecodeSector(inputs[i & 31], 8, coding);
    }
    allocated = GC.GetAllocatedBytesForCurrentThread() - allocated;
    double ms = Stopwatch.GetElapsedTime(start).TotalMilliseconds;
    Console.WriteLine($"coding={coding} sectors={measured} bytes/sector={allocated / measured} ns/sector={ms * 1e6 / measured:F0}");
    if (requireZero && allocated != 0) throw new Exception($"Steady-state sector decoding allocated {allocated} bytes");
}

if (args.Contains("--trace-peaks")) TracePeaks();
else if (args.Contains("--bench"))
{
    if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_AUDIO") == "1")
        throw new Exception("Disable audio tracing for the allocation/time probe");
    bool zero = args.Contains("--require-zero");
    Bench(1, zero); Bench(0, zero);
}
else
{
    Expect("transitions/playback", Playback(false), "699AA661A6A87A917141FE8505018A132EB4178E168C1F31A499C4236B959BD6");
    Expect("wrap/overwrite/playback", Playback(true), "9EC5B1E3C47B27552733D58C8CE774608D86E3FEAF89CFBD348F3E89AB278109");
    byte[] wav = Capture(() => { foreach (byte coding in new byte[] { 0, 5, 1, 4 }) Feed(100 + coding, coding); });
    Expect("capture/WAV", Convert.ToHexString(SHA256.HashData(wav)), "6BE7309EF4AFD55A219A65F9A9B7E212C3C4E4113324F825E50D84812C61B58E");
    Concurrent();
}
