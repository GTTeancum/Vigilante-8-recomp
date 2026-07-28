using NVorbis;

namespace RecompOne.Runtime.Cdrom;

internal sealed class LooseCdda : IDisposable
{
    private const int FramesPerSector = 588;
    private readonly string _root;
    private readonly IReadOnlyList<V8LooseTrack> _tracks;
    private V8LooseTrack? _openTrack;
    private VorbisReader? _reader;
    private float[] _decodeBuffer = [];

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

        EnsureOpen(track);
        long sourceFrame = checked((long)(lba - track.StartLba) * FramesPerSector);
        if (_reader!.SamplePosition != sourceFrame)
            Seek(sourceFrame, track);

        int channels = _reader.Channels;
        int wantedSamples = FramesPerSector * channels;
        int decodedSamples = 0;
        while (decodedSamples < wantedSamples)
        {
            int read = _reader.ReadSamples(
                _decodeBuffer, decodedSamples, wantedSamples - decodedSamples);
            if (read == 0) break;
            decodedSamples += read;
        }

        int decodedFrames = decodedSamples / channels;
        for (int frame = 0; frame < decodedFrames; frame++)
        {
            float left = _decodeBuffer[frame * channels];
            float right = channels == 1 ? left : _decodeBuffer[frame * 2 + 1];
            WritePcm(sector, frame * 4, FloatToPcm(left));
            WritePcm(sector, frame * 4 + 2, FloatToPcm(right));
        }
        return true;
    }

    private void EnsureOpen(V8LooseTrack track)
    {
        if (ReferenceEquals(_openTrack, track)) return;

        _reader?.Dispose();
        string path = SourcePath(track);
        _reader = new VorbisReader(path);
        if (_reader.SampleRate != 44100 || _reader.Channels is < 1 or > 2)
        {
            int sampleRate = _reader.SampleRate;
            int channels = _reader.Channels;
            _reader.Dispose();
            _reader = null;
            throw new InvalidDataException(
                $"Loose music must be 44100 Hz mono or stereo: {path} " +
                $"({sampleRate} Hz, {channels} channels)");
        }

        _decodeBuffer = new float[FramesPerSector * _reader.Channels];
        _openTrack = track;
        Console.WriteLine(
            $"[CDDA] loose track={track.Number} source={track.Source} streaming");
    }

    private void Seek(long sourceFrame, V8LooseTrack track)
    {
        try
        {
            _reader!.SamplePosition = sourceFrame;
            return;
        }
        catch (InvalidDataException exception)
        {
            // NVorbis can reject the final seek-table page produced by
            // libvorbis even though sequential decoding remains valid. CDDA
            // track switches commonly land in that last page, so reopen and
            // advance without consulting the damaged granule index.
            Console.Error.WriteLine(
                $"[CDDA] indexed seek failed for track {track.Number} " +
                $"frame={sourceFrame}; decoding forward: {exception.Message}");
        }

        _reader.Dispose();
        _reader = new VorbisReader(SourcePath(track));
        int channels = _reader.Channels;
        float[] discard = new float[65536 / channels * channels];
        long remaining = sourceFrame * channels;
        while (remaining > 0)
        {
            int requested = (int)Math.Min(remaining, discard.Length);
            int read = _reader.ReadSamples(discard, 0, requested);
            if (read == 0)
                break;
            remaining -= read;
        }
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

    private static void WritePcm(byte[] destination, int offset, short sample)
    {
        destination[offset] = (byte)sample;
        destination[offset + 1] = (byte)(sample >> 8);
    }

    private static short FloatToPcm(float sample)
    {
        sample = Math.Clamp(sample, -1f, 1f);
        return sample <= -1f
            ? short.MinValue
            : (short)MathF.Round(sample * short.MaxValue);
    }

    public void Dispose()
    {
        _reader?.Dispose();
        _reader = null;
        _openTrack = null;
    }
}
