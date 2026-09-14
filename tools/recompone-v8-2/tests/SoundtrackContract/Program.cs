using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using NVorbis;
using RecompOne.Runtime;
using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

void Check(bool condition, string label)
{
    if (!condition) throw new Exception(label);
    Console.WriteLine("PASS " + label);
}
if (args.Length == 1 && args[0] == "--config-only")
{
    string path = Path.Combine(Runtime.ExecutableDirectory, "settings.json");
    byte[]? before = File.Exists(path) ? File.ReadAllBytes(path) : null;
    try
    {
        Runtime.ConfigureGameTitle("Vigilante 8: 2nd Offense PC");
        foreach (var mode in Enum.GetValues<SoundtrackMode>())
        {
            File.WriteAllText(path, "{\"InputBindingsVersion\":2,\"MasterVolume\":0.37,\"Soundtrack\":" + (int)mode + "}");
            ConfigManager.Load();
            Check(ConfigManager.Game.Soundtrack == mode, $"{mode} loads through runtime JSON context");
            ConfigManager.SaveGame();
            ConfigManager.Game.Soundtrack = (SoundtrackMode)999;
            ConfigManager.Load();
            Check(ConfigManager.Game.Soundtrack == mode && ConfigManager.Game.MasterVolume == 0.37f,
                $"{mode} survives save/reload with volume intact");
        }
        File.WriteAllText(path, "{\"InputBindingsVersion\":2,\"MasterVolume\":0.37,\"Soundtrack\":999}");
        ConfigManager.Load();
        Check(ConfigManager.Game.Soundtrack == SoundtrackMode.Vigilante8 && ConfigManager.Game.MasterVolume == 0.37f,
            "invalid saved mode repairs only soundtrack");
    }
    finally
    {
        if (before != null) File.WriteAllBytes(path, before);
        else File.Delete(path);
    }
    return;
}
string root = Path.GetFullPath(args[0]);
Check(new GameConfig().Soundtrack == SoundtrackMode.Vigilante8, "fresh config defaults to V8");
Check(JsonSerializer.Deserialize<GameConfig>("{\"MasterVolume\":0.37}")!.Soundtrack == SoundtrackMode.Vigilante8,
    "existing config without soundtrack defaults to V8");
foreach (var mode in Enum.GetValues<SoundtrackMode>())
{
    var config = new GameConfig { Soundtrack = mode, MasterVolume = 0.37f };
    var loaded = JsonSerializer.Deserialize<GameConfig>(JsonSerializer.Serialize(config))!;
    Check(loaded.Soundtrack == mode && loaded.MasterVolume == 0.37f, $"{mode} serialization preserves volume");
}
Check(SoundtrackSettings.Normalize((SoundtrackMode)999) == SoundtrackMode.Vigilante8, "invalid choice normalized");
using var fs = CueFs.OpenLoose(root);
Check(fs.HasSoundtrackSelection && fs.LastTrackNumber == 13, "default runtime album has 12 V8 songs");
var reports = new List<object>();
foreach (var (mode, count) in new[] {
    (SoundtrackMode.Vigilante8, 12), (SoundtrackMode.SecondOffense, 16), (SoundtrackMode.Both, 28) })
{
    fs.ApplySoundtrack(mode);
    Check(fs.LastTrackNumber == count + 1, $"{mode} exposes {count} songs");
    Check(Enumerable.Range(0, count).Select(fs.SoundtrackSource).Distinct().Count() == count,
        $"{mode} has no duplicates");
    if (mode != SoundtrackMode.Both) continue;
    int previousEnd = 150;
    for (int i = 0; i < count; i++)
    {
        string source = fs.SoundtrackSource(i)!;
        fs.TryGetTrackStartLba(i + 2, out int start);
        int end = fs.TryGetTrackStartLba(i + 3, out int next) ? next : fs.LeadOutLba;
        using var reader = new VorbisReader(Path.Combine(root, source));
        Check(start == previousEnd && (end - start) == (reader.TotalSamples + 587) / 588,
            $"{source} full duration and contiguous boundary");
        previousEnd = end;
        using var expectedHash = IncrementalHash.CreateHash(HashAlgorithmName.SHA256);
        using var actualHash = IncrementalHash.CreateHash(HashAlgorithmName.SHA256);
        float[] samples = new float[588 * reader.Channels];
        byte[] expected = new byte[2352];
        for (int lba = start; lba < end; lba++)
        {
            Array.Clear(expected);
            int read = 0;
            while (read < samples.Length)
            {
                int n = reader.ReadSamples(samples, read, samples.Length - read);
                if (n == 0) break;
                read += n;
            }
            for (int frame = 0; frame < read / reader.Channels; frame++)
            {
                for (int channel = 0; channel < 2; channel++)
                {
                    float sample = Math.Clamp(samples[frame * reader.Channels + Math.Min(channel, reader.Channels - 1)], -1f, 1f);
                    short pcm = sample <= -1f ? short.MinValue : (short)MathF.Round(sample * short.MaxValue);
                    BinaryPrimitives.WriteInt16LittleEndian(expected.AsSpan(frame * 4 + channel * 2, 2), pcm);
                }
            }
            if (!fs.TryReadAudioSector(lba, out byte[] actual, out int number, out int actualEnd) ||
                number != i + 2 || actualEnd != end)
                throw new Exception($"wrong source/boundary at {source}/{lba}");
            expectedHash.AppendData(expected);
            actualHash.AppendData(actual);
        }
        string digest = Convert.ToHexString(actualHash.GetHashAndReset());
        Check(digest == Convert.ToHexString(expectedHash.GetHashAndReset()), $"{source} every PCM sample matches source at 44100 Hz");
        reports.Add(new { source, frames = reader.TotalSamples, sectors = end - start, sha256 = digest });
    }
}
File.WriteAllText(args[1], JsonSerializer.Serialize(reports, new JsonSerializerOptions { WriteIndented = true }));
Check(fs.LeadOutLba + 150 < 100 * 60 * 75, "combined album fits native BCD clock");
Check(!fs.TryReadAudioSector(fs.LeadOutLba, out _, out _, out _), "leadout does not read unrelated data");
// Exercise the real background CDDA pump, with no window/audio device and no
// host input. Drain one complete short song plus its wrap to verify looping.
var mem = new PSMemory();
var cpu = new CpuContext();
Runtime.Cd = new CdController(fs, mem);
LibCd.PlaySoundtrack(27, true);
fs.TryGetTrackStartLba(29, out int lastStart);
int framesToRead = (fs.LeadOutLba - lastStart + 2) * 588;
long consumed = 0;
var timer = System.Diagnostics.Stopwatch.StartNew();
while (consumed < framesToRead && timer.Elapsed.TotalSeconds < 60)
{
    if (CddaAudio.Next(out _, out _)) consumed++;
    else Thread.Sleep(1);
}
Check(consumed == framesToRead, "last song loops past leadout without stalling");
LibCd.ChangeSoundtrack(SoundtrackMode.Vigilante8);
Check(CddaAudio.BufferedFrames == 0 && fs.LastTrackNumber == 13, "live switch clears old queued audio and changes album");
LibCd.PlaySoundtrack(0, false);
Thread.Sleep(30);
Check(CddaAudio.BufferedFrames > 0, "new album starts playing after live switch");
cpu.A0 = 9; cpu.A1 = 0; cpu.A2 = 0;
LibCd.CdControl(cpu, mem);
Thread.Sleep(10);
Console.WriteLine("Soundtrack contract complete.");
