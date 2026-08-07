using System.Buffers.Binary;

namespace RecompOne.Runtime;

public static class XaAudio
{
    static readonly int[] Pos = { 0, 60, 115, 98 };
    static readonly int[] Neg = { 0, 0, -52, -55 };

    const int Capacity = 1 << 18;
    const int Mask = Capacity - 1;
    const int PrimeFrames = 1024;
    const int MaxHold = 8192;

    static readonly int[] _ring = new int[Capacity];
    static int _writeIdx, _readIdx, _count;
    static readonly object _gate = new();
    static readonly bool _trace =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_AUDIO") == "1";
    static long _sectorsDecoded;
    static FileStream? _diagnosticCapture;
    static long _diagnosticCaptureBytes;
    static string _diagnosticCaptureSource = "";

    static int _oldL, _olderL, _oldR, _olderR;
    static int _srcRate = 37800;
    static bool _playing;

    static double _pos;
    static short _s0L, _s0R, _s1L, _s1R;
    static int _underrun;

    public static void Reset()
    {
        lock (_gate)
        {
            EndDiagnosticCaptureLocked();
            _oldL = _olderL = _oldR = _olderR = 0;
            _writeIdx = _readIdx = _count = 0;
            _playing = false;
            _pos = 0;
            _s0L = _s0R = _s1L = _s1R = 0;
            _underrun = 0;
            _sectorsDecoded = 0;
        }
    }

    public static void BeginDiagnosticCapture(string sourceName)
    {
        string? configured =
            Environment.GetEnvironmentVariable("RECOMPONE_XA_CAPTURE");
        if (string.IsNullOrWhiteSpace(configured))
            return;
        lock (_gate)
        {
            EndDiagnosticCaptureLocked();
            string path = Path.GetFullPath(configured);
            if (!Path.HasExtension(path))
            {
                Directory.CreateDirectory(path);
                string safe = string.Concat(
                    sourceName.Select(character =>
                        char.IsLetterOrDigit(character) ? character : '_'));
                path = Path.Combine(path, $"{safe}.wav");
            }
            else
            {
                string? directory = Path.GetDirectoryName(path);
                if (!string.IsNullOrEmpty(directory))
                    Directory.CreateDirectory(directory);
            }
            _diagnosticCapture =
                new FileStream(
                    path, FileMode.Create, FileAccess.ReadWrite,
                    FileShare.Read);
            _diagnosticCapture.Write(new byte[44]);
            _diagnosticCaptureBytes = 0;
            _diagnosticCaptureSource = sourceName;
            WriteDiagnosticCaptureHeaderLocked();
            Console.Error.WriteLine(
                $"[XA_CAPTURE] begin source='{sourceName}' path='{path}'");
        }
    }

    public static void EndDiagnosticCapture()
    {
        lock (_gate)
            EndDiagnosticCaptureLocked();
    }

    static void EndDiagnosticCaptureLocked()
    {
        if (_diagnosticCapture == null)
            return;
        WriteDiagnosticCaptureHeaderLocked();
        Console.Error.WriteLine(
            $"[XA_CAPTURE] end source='{_diagnosticCaptureSource}' " +
            $"rate={_srcRate} bytes={_diagnosticCaptureBytes}");
        _diagnosticCapture.Dispose();
        _diagnosticCapture = null;
        _diagnosticCaptureBytes = 0;
        _diagnosticCaptureSource = "";
    }

    static void WriteDiagnosticCaptureHeaderLocked()
    {
        if (_diagnosticCapture == null)
            return;
        Span<byte> header = stackalloc byte[44];
        "RIFF"u8.CopyTo(header);
        BinaryPrimitives.WriteUInt32LittleEndian(
            header[4..], checked((uint)(36 + _diagnosticCaptureBytes)));
        "WAVE"u8.CopyTo(header[8..]);
        "fmt "u8.CopyTo(header[12..]);
        BinaryPrimitives.WriteUInt32LittleEndian(header[16..], 16);
        BinaryPrimitives.WriteUInt16LittleEndian(header[20..], 1);
        BinaryPrimitives.WriteUInt16LittleEndian(header[22..], 2);
        BinaryPrimitives.WriteUInt32LittleEndian(
            header[24..], checked((uint)_srcRate));
        BinaryPrimitives.WriteUInt32LittleEndian(
            header[28..], checked((uint)(_srcRate * 4)));
        BinaryPrimitives.WriteUInt16LittleEndian(header[32..], 4);
        BinaryPrimitives.WriteUInt16LittleEndian(header[34..], 16);
        "data"u8.CopyTo(header[36..]);
        BinaryPrimitives.WriteUInt32LittleEndian(
            header[40..], checked((uint)_diagnosticCaptureBytes));
        long position = _diagnosticCapture.Position;
        _diagnosticCapture.Position = 0;
        _diagnosticCapture.Write(header);
        _diagnosticCapture.Position = position;
        _diagnosticCapture.Flush();
    }

    static int Clamp(int v) => v < -32768 ? -32768 : v > 32767 ? 32767 : v;

    static void DecodeBlock(byte[] sec, int b, int blk, ref int old, ref int older, int[] dst)
    {
        byte hdr = sec[b + 4 + blk];
        int sv = hdr & 0xF; if (sv > 12) sv = 9;
        int filter = (hdr >> 4) & 0x3;
        int f0 = Pos[filter], f1 = Neg[filter];
        int col = blk >> 1, nshift = (blk & 1) * 4;
        for (int j = 0; j < 28; j++)
        {
            int nib = (sec[b + 16 + 4 * j + col] >> nshift) & 0xF;
            int t = nib >= 8 ? nib - 16 : nib;
            int s = Clamp(((t << 12) >> sv) + ((old * f0 + older * f1 + 32) >> 6));
            older = old;
            old = s;
            dst[j] = s;
        }
    }

    public static void DecodeSector(
        byte[] sec,
        int off,
        byte coding,
        int lba = -1,
        byte file = 0,
        byte channel = 0)
    {
        bool stereo = (coding & 0x01) != 0;
        int rate = (coding & 0x04) != 0 ? 18900 : 37800;
        int[] l = new int[28], r = new int[28];
        int[] frames = new int[stereo ? 2016 : 4032];
        int n = 0;

        for (int p = 0; p < 18; p++)
        {
            int b = off + p * 128;
            if (stereo)
            {
                for (int tb = 0; tb < 4; tb++)
                {
                    DecodeBlock(sec, b, tb * 2, ref _oldL, ref _olderL, l);
                    DecodeBlock(sec, b, tb * 2 + 1, ref _oldR, ref _olderR, r);
                    for (int j = 0; j < 28; j++) frames[n++] = (ushort)l[j] | (r[j] << 16);
                }
            }
            else
            {
                for (int blk = 0; blk < 8; blk++)
                {
                    DecodeBlock(sec, b, blk, ref _oldL, ref _olderL, l);
                    for (int j = 0; j < 28; j++) frames[n++] = (ushort)l[j] | (l[j] << 16);
                }
            }
        }

        lock (_gate)
        {
            _srcRate = rate;
            int peak = 0;
            if (_diagnosticCapture != null)
            {
                byte[] capture = new byte[n * 4];
                for (int index = 0; index < n; index++)
                    BinaryPrimitives.WriteInt32LittleEndian(
                        capture.AsSpan(index * 4, 4), frames[index]);
                _diagnosticCapture.Write(capture);
                _diagnosticCaptureBytes += capture.Length;
            }
            for (int i = 0; i < n; i++)
            {
                int packed = frames[i];
                peak = Math.Max(peak, Math.Abs((int)(short)(packed & 0xFFFF)));
                peak = Math.Max(peak, Math.Abs((int)(short)(packed >> 16)));
                _ring[_writeIdx] = packed;
                _writeIdx = (_writeIdx + 1) & Mask;
                if (_count < Capacity) _count++;
                else _readIdx = (_readIdx + 1) & Mask;
            }
            if (!_playing && _count >= PrimeFrames) _playing = true;
            long decoded = ++_sectorsDecoded;
            if (_trace && (decoded == 1 || decoded % 19 == 0))
            {
                Console.Error.WriteLine(
                    $"[XA] file={file} channel={channel} lba={lba} sectors={decoded} " +
                    $"rate={rate} frames={n} buffered={_count} peak={peak}");
            }
        }
    }

    public static int BufferedSamples { get { lock (_gate) return _count; } }

    public static bool Playing { get { lock (_gate) return _playing; } }

    public static int SourceRate { get { lock (_gate) return _srcRate; } }

    public static bool Next(out short left, out short right)
    {
        lock (_gate)
        {
            if (!_playing) { left = right = 0; return false; }

            while (_pos >= 1.0)
            {
                _s0L = _s1L; _s0R = _s1R;
                if (_count > 0)
                {
                    int packed = _ring[_readIdx];
                    _readIdx = (_readIdx + 1) & Mask;
                    _count--;
                    _s1L = (short)(packed & 0xFFFF);
                    _s1R = (short)(packed >> 16);
                    _underrun = 0;
                }
                else
                {
                    if (++_underrun > MaxHold) { _playing = false; left = right = 0; return false; }
                    _s1L = (short)(_s1L * 31 / 32);
                    _s1R = (short)(_s1R * 31 / 32);
                }
                _pos -= 1.0;
            }

            double f = _pos;
            left = (short)(_s0L + (_s1L - _s0L) * f);
            right = (short)(_s0R + (_s1R - _s0R) * f);
            _pos += (double)_srcRate / 44100.0;
            return true;
        }
    }
}
