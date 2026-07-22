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
