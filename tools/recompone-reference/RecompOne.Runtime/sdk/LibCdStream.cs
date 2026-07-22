using System.Diagnostics;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class LibCdStream
{
    const int HeaderSize = 32;
    const int SlotData = 2016;
    const ushort VideoMagic = 0x0160;

    public static bool InUse { get; private set; }
    static uint _statusBase;
    static int _slots;
    static uint _dataBase;

    static volatile bool _active;
    static volatile bool _reading;
    static int _pendingLba = -1;
    static int _streamLba = -1;
    static int _streamStartLba;
    static int _streamEndLba = int.MaxValue;
    static string _streamName = "unknown";
    static int _framesQueued;
    static int _lastFrameNumber = -1;
    static bool _streamSummaryPending;
    static readonly Stopwatch _clock = new();

    static int _writeIdx;
    static bool[] _busy = System.Array.Empty<bool>();
    static readonly Queue<(int start, int n)> _ready = new();
    static int _prevStart = -1, _prevN;

    static Thread? _thread;
    static volatile bool _run;
    static readonly object _lock = new();
    static bool _loggedScan;

    public static void StSetRing(CpuContext c, IMemory m)
    {
        InUse = true;
        lock (_lock)
        {
            _statusBase = c.A0;
            _slots = (int)c.A1;
            _dataBase = _statusBase + (uint)(_slots * HeaderSize);
            ResetRing(m);
        }
        EnsureThread();
        Console.WriteLine($"[CdStream] ring base=0x{_statusBase:X8} slots={_slots}");
        Log.Sdk($"StSetRing base=0x{_statusBase:X8} slots={_slots} data=0x{_dataBase:X8}");
    }

    public static void StClearRing(CpuContext c, IMemory m)
    {
        lock (_lock) ResetRing(m);
        c.V0 = 0;
        Log.Sdk("StClearRing");
    }

    public static void StUnSetRing(CpuContext c, IMemory m)
    {
        lock (_lock)
        {
            _active = false;
            _reading = false;
            LogStreamSummary("ring-unset");
            Monitor.PulseAll(_lock);
        }
        Log.Sdk("StUnSetRing");
    }

    public static void StSetStream(CpuContext c, IMemory m)
    {
        lock (_lock)
        {
            _streamLba = -1;
            ResetRing(m);
            XaAudio.Reset();
        }
        _active = true;
        EnsureThread();
        Log.Sdk("StSetStream");
    }

    public static void StSetMask(CpuContext c, IMemory m) { c.V0 = 0; Log.Sdk("StSetMask"); }

    public static void StGetNext(CpuContext c, IMemory m)
    {
        if (!_active) { c.V0 = 1; return; }

        lock (_lock)
        {
            if (_prevStart >= 0)
            {
                for (int i = 0; i < _prevN; i++) _busy[_prevStart + i] = false;
                _prevStart = -1;
            }

            // The original R3000 loop polls while real CD interrupts advance in
            // parallel. Yield briefly so a host-paced frame can arrive before
            // that statically recompiled polling loop exhausts its iteration cap.
            while (_ready.Count == 0 && _reading) Monitor.Wait(_lock, 4);
            if (_ready.Count == 0) { c.V0 = 1; return; }

            var (start, n) = _ready.Dequeue();
            uint dataPtr = _dataBase + (uint)(start * SlotData);
            uint hdrPtr = _statusBase + (uint)(start * HeaderSize);
            m.WriteU32(c.A0, dataPtr);
            m.WriteU32(c.A1, hdrPtr);
            _prevStart = start;
            _prevN = n;
            c.V0 = 0;
        }
    }

    public static void StFreeRing(CpuContext c, IMemory m) { c.V0 = 0; Log.Sdk("StFreeRing"); }

    public static void StGetBackloc(CpuContext c, IMemory m) { c.V0 = 0xFFFFFFFFu; Log.Sdk("StGetBackloc"); }

    internal static void OnReadStream(int lba)
    {
        if (!InUse) return;
        LibCd.TryDescribeLocatedFile(lba, out string name, out int endLba);
        lock (_lock)
        {
            _pendingLba = lba;
            _streamLba = -1;
            _streamStartLba = lba;
            _streamEndLba = endLba;
            _streamName = name;
            _framesQueued = 0;
            _lastFrameNumber = -1;
            _streamSummaryPending = true;
            _loggedScan = false;
            _reading = true;
            Monitor.PulseAll(_lock);
        }
        Console.WriteLine($"[CdStream] start '{name}' LBA={lba} end={endLba}");
        EnsureThread();
    }

    internal static void OnStopStream()
    {
        lock (_lock)
        {
            _reading = false;
            LogStreamSummary("stop-command");
            Monitor.PulseAll(_lock);
        }
    }

    static void ResetRing(IMemory m)
    {
        _writeIdx = 0;
        _prevStart = -1;
        _prevN = 0;
        _ready.Clear();
        _busy = _slots > 0 ? new bool[_slots] : System.Array.Empty<bool>();
        for (int i = 0; i < _slots; i++)
            m.WriteU16(_statusBase + (uint)(i * HeaderSize), 0);
    }

    static void EnsureThread()
    {
        if (_thread is { IsAlive: true }) return;
        _run = true;
        _thread = new Thread(StreamLoop) { IsBackground = true, Name = "CdStream" };
        _thread.Start();
    }

    static void StreamLoop()
    {
        while (_run)
        {
            var cd = Runtime.Cd;
            var m = Runtime.Mem;
            if (cd == null || m == null || !_active || !_reading || _slots <= 0)
            {
                Thread.Sleep(2);
                continue;
            }

            if (_streamLba < 0)
            {
                _streamLba = _pendingLba >= 0 ? _pendingLba : LibCd.CurrentLba;
                _streamStartLba = _streamLba;
                _clock.Restart();
            }

            if (_streamLba >= _streamEndLba)
            {
                lock (_lock)
                {
                    _reading = false;
                    LogStreamSummary("file-end");
                    Monitor.PulseAll(_lock);
                }
                continue;
            }

            byte[] sec;
            try { lock (LibCd.DiscLock) sec = cd.ReadSectorData(_streamLba, 2336); }
            catch { Thread.Sleep(2); continue; }
            int payload = StrPayloadOffset(sec);

            if (!_loggedScan)
            {
                _loggedScan = true;
                int header = payload >= 0 ? payload : 8;
                Console.WriteLine($"[CdStream] scanning LBA={_streamLba} submode=0x{sec[2]:X2} magic=0x{Read16(sec, header):X4} chunk={Read16(sec, header + 4)} count={Read16(sec, header + 6)} payload={payload}");
            }

            if (payload < 0)
            {
                double audioDelivered = _clock.Elapsed.TotalSeconds * LibCd.SectorsPerSecond;
                if ((_streamLba - _streamStartLba) + 1 > audioDelivered)
                {
                    Thread.Sleep(1);
                    continue;
                }
                if ((sec[2] & 0x04) != 0 && LibCd.AcceptXaSector(sec[0], sec[1]))
                    XaAudio.DecodeSector(sec, 8, sec[3], _streamLba, sec[0], sec[1]);
                LibCd.ReportXaSector(_streamLba);
                _streamLba++;
                continue;
            }
            if (Read16(sec, payload + 4) != 0) { _streamLba++; continue; }

            int n = Read16(sec, payload + 6);
            if (n <= 0 || n > _slots) { _streamLba++; continue; }

            double delivered = _clock.Elapsed.TotalSeconds * LibCd.SectorsPerSecond;
            if ((_streamLba - _streamStartLba) + n > delivered) { Thread.Sleep(1); continue; }

            int start;
            lock (_lock)
            {
                if (_writeIdx + n > _slots) _writeIdx = 0;
                start = _writeIdx;
                bool free = true;
                for (int i = 0; i < n; i++) if (_busy[start + i]) { free = false; break; }
                if (!free) { Thread.Sleep(1); continue; }
            }

            int frameLba = _streamLba;
            int frameNumber = (int)Read32(sec, payload + 8);
            int width = Read16(sec, payload + 16);
            int height = Read16(sec, payload + 18);
            if (!CollectFrame(cd, m, start, n)) continue;

            int queued = ++_framesQueued;
            bool reset = _lastFrameNumber >= 0 && frameNumber <= _lastFrameNumber;
            if (queued == 1 || reset || frameNumber % 120 == 0)
            {
                string marker = reset ? " frame-reset" : "";
                Console.WriteLine($"[CdStream] '{_streamName}' frame={frameNumber} queued={queued} LBA={frameLba} chunks={n} {width}x{height}{marker}");
            }
            _lastFrameNumber = frameNumber;

            lock (_lock)
            {
                for (int i = 0; i < n; i++) _busy[start + i] = true;
                _ready.Enqueue((start, n));
                _writeIdx = start + n;
                Monitor.PulseAll(_lock);
            }
        }
    }

    static bool CollectFrame(Cdrom.CdController cd, IMemory m, int start, int n)
    {
        int collected = 0;
        int lba = _streamLba;
        while (collected < n)
        {
            byte[] sec;
            try { lock (LibCd.DiscLock) sec = cd.ReadSectorData(lba, 2336); }
            catch { return false; }
            lba++;

            int payload = StrPayloadOffset(sec);
            if (payload < 0)
            {
                if ((sec[2] & 0x04) != 0 && LibCd.AcceptXaSector(sec[0], sec[1]))
                    XaAudio.DecodeSector(sec, 8, sec[3], lba - 1, sec[0], sec[1]);
                LibCd.ReportXaSector(lba - 1);
                continue;
            }

            uint hdr = _statusBase + (uint)((start + collected) * HeaderSize);
            uint dat = _dataBase + (uint)((start + collected) * SlotData);
            for (int j = 0; j < HeaderSize; j++) m.WriteU8(hdr + (uint)j, sec[payload + j]);
            for (int j = 0; j < SlotData; j++) m.WriteU8(dat + (uint)j, sec[payload + HeaderSize + j]);
            collected++;
        }
        _streamLba = lba;
        Thread.MemoryBarrier();
        return true;
    }

    static int StrPayloadOffset(byte[] sector)
    {
        if (sector.Length >= 8 + HeaderSize + SlotData && Read16(sector, 8) == VideoMagic) return 8;
        if (sector.Length >= HeaderSize + SlotData && Read16(sector, 0) == VideoMagic) return 0;
        return -1;
    }

    // Caller holds _lock.
    static void LogStreamSummary(string reason)
    {
        if (!_streamSummaryPending) return;
        _streamSummaryPending = false;
        Console.WriteLine($"[CdStream] end '{_streamName}' reason={reason} frames={_framesQueued} last={_lastFrameNumber} elapsed={_clock.Elapsed.TotalSeconds:F2}s");
    }

    static ushort Read16(byte[] b, int o) => (ushort)(b[o] | (b[o + 1] << 8));
    static uint Read32(byte[] b, int o) => (uint)(b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24));
}
