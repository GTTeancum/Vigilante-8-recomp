using NVorbis;

namespace RecompOne.Runtime.Cdrom;

internal sealed class LooseCdda
{
    private const int FramesPerSector = 588;
    private readonly string _root;
    private readonly IReadOnlyList<V8LooseTrack> _tracks;
    private V8LooseTrack? _decodedTrack;
    private short[] _decodedSamples = [];

    public LooseCdda(string root, IReadOnlyList<V8LooseTrack> tracks)
    {
        _root = root;
        _tracks = tracks;
        foreach (var track in tracks.Where(candidate => candidate.Number > 1))
        {
            string path = SourcePath(track);
            if (!File.Exists(path))
                throw new FileNotFoundException(
                    $"Loose music track {track.Number:00} is missing: {path}", path);
        }
    }

    public bool TryReadSector(
        int lba,
        out byte[] sector,
        out int trackNumber,
        out int trackEndLba)
    {
        var track = _tracks.FirstOrDefault(candidate =>
            candidate.Number > 1 && lba >= candidate.Index0Lba && lba < candidate.EndLba);
        if (track == null)
        {
            sector = [];
            trackNumber = 0;
            trackEndLba = 0;
            return false;
        }

        sector = new byte[2352];
        trackNumber = track.Number;
        trackEndLba = track.EndLba;
        if (lba < track.StartLba)
            return true;

        EnsureDecoded(track);
        int sourceFrame = checked((lba - track.StartLba) * FramesPerSector);
        int sourceSample = checked(sourceFrame * 2);
        int available = Math.Min(sector.Length / 2, _decodedSamples.Length - sourceSample);
        if (available <= 0)
            return true;
        for (int i = 0; i < available; i++)
        {
            short sample = _decodedSamples[sourceSample + i];
            sector[i * 2] = (byte)sample;
            sector[i * 2 + 1] = (byte)(sample >> 8);
        }
        return true;
    }

    private void EnsureDecoded(V8LooseTrack track)
    {
        if (ReferenceEquals(_decodedTrack, track)) return;

        string path = SourcePath(track);
        using var reader = new VorbisReader(path);
        if (reader.SampleRate != 44100 || reader.Channels is < 1 or > 2)
            throw new InvalidDataException(
                $"Loose music must be 44100 Hz mono or stereo: {path} " +
                $"({reader.SampleRate} Hz, {reader.Channels} channels)");

        int expectedFrames = checked((track.EndLba - track.StartLba) * FramesPerSector);
        var decoded = new short[checked(expectedFrames * 2)];
        var buffer = new float[16384 * reader.Channels];
        int destinationFrame = 0;
        while (destinationFrame < expectedFrames)
        {
            int read = reader.ReadSamples(buffer, 0, buffer.Length);
            if (read == 0) break;
            int frames = read / reader.Channels;
            int copyFrames = Math.Min(frames, expectedFrames - destinationFrame);
            for (int frame = 0; frame < copyFrames; frame++)
            {
                float left = buffer[frame * reader.Channels];
                float right = reader.Channels == 1 ? left : buffer[frame * 2 + 1];
                decoded[(destinationFrame + frame) * 2] = FloatToPcm(left);
                decoded[(destinationFrame + frame) * 2 + 1] = FloatToPcm(right);
            }
            destinationFrame += copyFrames;
        }

        _decodedTrack = track;
        _decodedSamples = decoded;
        Console.WriteLine(
            $"[CDDA] loose track={track.Number} source={track.Source} " +
            $"frames={destinationFrame}/{expectedFrames}");
    }

    private string SourcePath(V8LooseTrack track)
    {
        if (string.IsNullOrWhiteSpace(track.Source))
            throw new InvalidDataException(
                $"Loose music source is missing for track {track.Number:00}");
        string path = Path.GetFullPath(Path.Combine(
            _root, track.Source.Replace('/', Path.DirectorySeparatorChar)));
        string rootPrefix = Path.GetFullPath(_root).TrimEnd(
            Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar;
        if (!path.StartsWith(rootPrefix, StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException(
                $"Loose music path escapes the asset root: {track.Source}");
        return path;
    }

    private static short FloatToPcm(float sample)
    {
        sample = Math.Clamp(sample, -1f, 1f);
        return sample <= -1f
            ? short.MinValue
            : (short)MathF.Round(sample * short.MaxValue);
    }
}
