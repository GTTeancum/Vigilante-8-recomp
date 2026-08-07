using System.Buffers.Binary;
using System.Runtime.InteropServices;
using Silk.NET.SDL;
using Thread = System.Threading.Thread;

namespace RecompOne.Runtime.Host;

internal static unsafe class Audio
{
    const int SampleRate = 44100;
    const int Channels = 2;
    const int NumBuffers = 8;
    const int FramesPerBuffer = 256;
    const uint BytesPerBuffer = FramesPerBuffer * Channels * sizeof(short);
    const uint TargetQueuedBytes = NumBuffers * BytesPerBuffer;

    static Sdl? _sdl;
    static uint _device;
    static readonly short[] _sampleBuf = new short[FramesPerBuffer * Channels];

    static Thread? _mixerThread;
    static Spu? _spu;
    static volatile bool _running;
    static float _masterVolume = 1.0f;
    static long _mixedFrames;
    static bool _firstAudibleBufferReported;
    static readonly bool _traceAudio =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_AUDIO") == "1";
    static long _traceSamples;
    static double _traceSquareSum;
    static int _tracePeak;

    static FileStream? _capture;
    static long _capturedBytes;
    static int _captureBuffersSinceHeader;

    internal static long MixedFrames =>
        Interlocked.Read(ref _mixedFrames);

    public static void Initialize()
    {
        try
        {
            _sdl = Sdl.GetApi();
            if (_sdl.InitSubSystem(Sdl.InitAudio) != 0)
                throw new InvalidOperationException($"SDL audio init failed: {GetError()}");

            AudioSpec wanted = new()
            {
                Freq = SampleRate,
                Format = (ushort)Sdl.AudioS16Sys,
                Channels = Channels,
                Samples = FramesPerBuffer,
                Callback = default,
                Userdata = null,
            };
            AudioSpec obtained = default;
            _device = _sdl.OpenAudioDevice((byte*)null, 0, &wanted, &obtained, 0);
            if (_device == 0)
                throw new InvalidOperationException($"SDL audio device open failed: {GetError()}");

            if (obtained.Freq != SampleRate || obtained.Format != (ushort)Sdl.AudioS16Sys ||
                obtained.Channels != Channels)
            {
                throw new InvalidOperationException(
                    $"SDL returned unsupported audio format {obtained.Freq} Hz/0x{obtained.Format:X4}/{obtained.Channels}ch");
            }

            Array.Clear(_sampleBuf);
            OpenCapture();
            for (int i = 0; i < NumBuffers; i++)
                QueueCurrentBuffer();

            _running = true;
            _mixerThread = new Thread(MixerLoop) { IsBackground = true, Name = "spu-mixer" };
            _mixerThread.Start();
            _sdl.PauseAudioDevice(_device, 0);
            Console.Error.WriteLine(
                $"[Host] SDL audio ready: device={_device} {obtained.Freq} Hz stereo S16 queue={TargetQueuedBytes} bytes");
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"[Host] audio init failed: {e.Message}");
            Shutdown();
        }
    }

    public static void Attach(Spu? spu)
    {
        if (spu != null) _spu = spu;
    }

    public static void SetMasterVolume(float volume)
    {
        _masterVolume = Math.Clamp(volume, 0f, 1f);
    }

    static void MixerLoop()
    {
        while (_running)
        {
            var sdl = _sdl;
            var spu = _spu;
            if (sdl == null || _device == 0 || spu == null)
            {
                Thread.Sleep(3);
                continue;
            }

            while (_running && sdl.GetQueuedAudioSize(_device) < TargetQueuedBytes)
            {
                spu.Mix(_sampleBuf, FramesPerBuffer);
                ApplyMasterVolume();
                ReportFirstAudibleBuffer();
                ReportAudioSummary();
                CaptureCurrentBuffer();
                QueueCurrentBuffer();
                _mixedFrames += FramesPerBuffer;
            }

            Thread.Sleep(3);
        }
    }

    static void ApplyMasterVolume()
    {
        float volume = _masterVolume;
        if (volume >= 0.999f) return;
        if (volume <= 0.001f)
        {
            Array.Clear(_sampleBuf);
            return;
        }

        for (int i = 0; i < _sampleBuf.Length; i++)
            _sampleBuf[i] = (short)Math.Clamp((int)MathF.Round(_sampleBuf[i] * volume), short.MinValue, short.MaxValue);
    }

    static void ReportFirstAudibleBuffer()
    {
        if (_firstAudibleBufferReported) return;
        int peak = 0;
        foreach (short sample in _sampleBuf)
            peak = Math.Max(peak, Math.Abs((int)sample));
        if (peak == 0) return;

        _firstAudibleBufferReported = true;
        Console.Error.WriteLine($"[Host] first nonzero SPU output at mixed frame {_mixedFrames}: peak={peak}");
    }

    static void ReportAudioSummary()
    {
        if (!_traceAudio) return;
        foreach (short sample in _sampleBuf)
        {
            int value = sample;
            _tracePeak = Math.Max(_tracePeak, Math.Abs(value));
            _traceSquareSum += (double)value * value;
            _traceSamples++;
        }
        if (_traceSamples < SampleRate * Channels) return;

        double rms = Math.Sqrt(_traceSquareSum / _traceSamples);
        double rmsDb = rms > 0 ? 20.0 * Math.Log10(rms / 32768.0) : double.NegativeInfinity;
        string rmsText = double.IsNegativeInfinity(rmsDb) ? "-inf" : rmsDb.ToString("F2");
        Console.Error.WriteLine(
            $"[AUDIO] frame={_mixedFrames} peak={_tracePeak} rms_dbfs={rmsText} " +
            $"cdda={CddaAudio.Playing}/{CddaAudio.BufferedFrames} " +
            $"xa={XaAudio.Playing}/{XaAudio.BufferedSamples}");
        _traceSamples = 0;
        _traceSquareSum = 0;
        _tracePeak = 0;
    }

    static void OpenCapture()
    {
        string? path = Environment.GetEnvironmentVariable("RECOMPONE_AUDIO_CAPTURE");
        if (string.IsNullOrWhiteSpace(path)) return;
        path = Path.GetFullPath(path);
        string? directory = Path.GetDirectoryName(path);
        if (!string.IsNullOrEmpty(directory)) Directory.CreateDirectory(directory);
        _capture = new FileStream(path, FileMode.Create, FileAccess.ReadWrite, FileShare.Read);
        _capture.Write(new byte[44]);
        WriteCaptureHeader(flush: true);
        Console.Error.WriteLine($"[Host] audio capture: {path}");
    }

    static void CaptureCurrentBuffer()
    {
        if (_capture == null) return;
        ReadOnlySpan<byte> bytes = MemoryMarshal.AsBytes(_sampleBuf.AsSpan());
        _capture.Seek(0, SeekOrigin.End);
        _capture.Write(bytes);
        _capturedBytes += bytes.Length;
        if (++_captureBuffersSinceHeader >= 64)
        {
            _captureBuffersSinceHeader = 0;
            WriteCaptureHeader(flush: true);
        }
    }

    static void WriteCaptureHeader(bool flush)
    {
        if (_capture == null) return;
        Span<byte> header = stackalloc byte[44];
        "RIFF"u8.CopyTo(header);
        BinaryPrimitives.WriteUInt32LittleEndian(header[4..], checked((uint)(36 + _capturedBytes)));
        "WAVE"u8.CopyTo(header[8..]);
        "fmt "u8.CopyTo(header[12..]);
        BinaryPrimitives.WriteUInt32LittleEndian(header[16..], 16);
        BinaryPrimitives.WriteUInt16LittleEndian(header[20..], 1);
        BinaryPrimitives.WriteUInt16LittleEndian(header[22..], Channels);
        BinaryPrimitives.WriteUInt32LittleEndian(header[24..], SampleRate);
        BinaryPrimitives.WriteUInt32LittleEndian(header[28..], SampleRate * Channels * sizeof(short));
        BinaryPrimitives.WriteUInt16LittleEndian(header[32..], Channels * sizeof(short));
        BinaryPrimitives.WriteUInt16LittleEndian(header[34..], sizeof(short) * 8);
        "data"u8.CopyTo(header[36..]);
        BinaryPrimitives.WriteUInt32LittleEndian(header[40..], checked((uint)_capturedBytes));
        long position = _capture.Position;
        _capture.Position = 0;
        _capture.Write(header);
        _capture.Position = position;
        if (flush) _capture.Flush();
    }

    static void QueueCurrentBuffer()
    {
        var sdl = _sdl ?? throw new InvalidOperationException("SDL audio is not initialized");
        fixed (short* samples = _sampleBuf)
        {
            if (sdl.QueueAudio(_device, samples, BytesPerBuffer) != 0)
                throw new InvalidOperationException($"SDL audio queue failed: {GetError()}");
        }
    }

    static string GetError()
    {
        if (_sdl == null) return "SDL unavailable";
        return Marshal.PtrToStringUTF8((nint)_sdl.GetError()) ?? "unknown SDL error";
    }

    public static void Shutdown()
    {
        _running = false;
        _mixerThread?.Join();
        _mixerThread = null;
        _spu = null;
        _mixedFrames = 0;
        _firstAudibleBufferReported = false;
        _traceSamples = 0;
        _traceSquareSum = 0;
        _tracePeak = 0;

        if (_capture != null)
        {
            WriteCaptureHeader(flush: true);
            _capture.Dispose();
            _capture = null;
            _capturedBytes = 0;
            _captureBuffersSinceHeader = 0;
        }

        if (_sdl != null)
        {
            if (_device != 0)
            {
                _sdl.PauseAudioDevice(_device, 1);
                _sdl.ClearQueuedAudio(_device);
                _sdl.CloseAudioDevice(_device);
                _device = 0;
            }
            _sdl.QuitSubSystem(Sdl.InitAudio);
            _sdl.Dispose();
            _sdl = null;
        }
    }
}
