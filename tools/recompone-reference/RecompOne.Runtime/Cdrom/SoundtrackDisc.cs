using NVorbis;
using RecompOne.Runtime.Config;

namespace RecompOne.Runtime.Cdrom;

/// <summary>
/// An audio-only TOC. File/data-sector addresses always use the original manifest.
/// Keeping audio on its own timeline also keeps the combined album below the
/// CD protocol's 100-minute BCD limit. No PCM is stretched to fit a retail slot.
/// </summary>
internal sealed class SoundtrackDisc : IDisposable
{
    readonly string root;
    readonly V8LooseTrack[] original;
    readonly V8LooseTrack[] second;
    readonly Dictionary<string, int> lengths = new(StringComparer.OrdinalIgnoreCase);
    LooseCdda? audio;
    public IReadOnlyList<V8LooseTrack> Tracks { get; private set; } = [];
    public int LeadOutLba => Tracks[^1].EndLba;
    public SoundtrackMode Mode { get; private set; }

    public SoundtrackDisc(string root, IReadOnlyList<V8LooseTrack> retail)
    {
        this.root = root;
        original = V8LooseManifest.LoadEmbedded("V8LooseManifest.json").Tracks
            .Where(t => t.Number > 1).ToArray();
        second = retail.Where(t => t.Number > 1).ToArray();
        Apply(ConfigManager.Game.Soundtrack);
    }

    public void Apply(SoundtrackMode requested)
    {
        var mode = SoundtrackSettings.Normalize(requested);
        IEnumerable<V8LooseTrack> selected = mode switch
        {
            SoundtrackMode.Vigilante8 => original,
            SoundtrackMode.SecondOffense => second,
            _ => original.Concat(second),
        };
        // Original music is optional in older installations. Retain the saved
        // preference and report the fallback; do not silently rewrite it.
        var available = selected.Where(t => File.Exists(Path.Combine(root, t.Source!))).ToArray();
        if (available.Length == 0)
        {
            available = second.Where(t => File.Exists(Path.Combine(root, t.Source!))).ToArray();
            Console.Error.WriteLine($"[Soundtrack] {SoundtrackSettings.Label(mode)} unavailable; using Second Offense");
        }
        if (available.Length == 0)
            throw new FileNotFoundException("No soundtrack OGG files are installed.");

        var tracks = new List<V8LooseTrack>();
        // A dummy data-track entry supplies the native CD command convention.
        tracks.Add(new V8LooseTrack { Number = 1, EndLba = 150 });
        int start = 150;
        foreach (var source in available)
        {
            if (!lengths.TryGetValue(source.Source!, out int sectors))
            {
                using var reader = new VorbisReader(Path.Combine(root, source.Source!));
                if (reader.SampleRate != 44100 || reader.Channels is < 1 or > 2)
                    throw new InvalidDataException($"Soundtrack must be 44100 Hz mono/stereo: {source.Source}");
                sectors = checked((int)((reader.TotalSamples + 587) / 588));
                if (sectors <= 0) throw new InvalidDataException($"Empty soundtrack: {source.Source}");
                lengths.Add(source.Source!, sectors);
            }
            tracks.Add(new V8LooseTrack {
                Number = tracks.Count + 1, Index0Lba = start, StartLba = start,
                EndLba = checked(start + sectors), Source = source.Source,
            });
            start += sectors;
        }
        if (start + 150 >= 100 * 60 * 75)
            throw new InvalidDataException("Soundtrack exceeds the CD time-code range.");
        var next = new LooseCdda(root, tracks);
        audio?.Dispose();
        audio = next;
        Tracks = tracks;
        Mode = mode;
        Console.Error.WriteLine($"[Soundtrack] {SoundtrackSettings.Label(mode)}: {tracks.Count - 1} tracks, original 44100 Hz playback");
    }

    public string? Source(int index) => index >= 0 && index + 1 < Tracks.Count
        ? Tracks[index + 1].Source : null;

    public bool TryReadSector(int lba, out byte[] sector, out int number, out int end) =>
        audio!.TryReadSector(lba, out sector, out number, out end);

    public void Dispose() => audio?.Dispose();
}
