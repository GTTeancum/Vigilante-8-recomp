using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Enhanced;

namespace RecompOne.Runtime.Sdk;

public static class LibCd
{
    const byte Nop = 0x01, 
        Setloc = 0x02,
        Play = 0x03,
        ReadN = 0x06, 
        Stop = 0x08,
        Pause = 0x09,
        Init = 0x0A,
        Mute = 0x0B, 
        Demute = 0x0C, 
        Setfilter = 0x0D,
        Setmode = 0x0E,
        GetlocL = 0x10,
        GetlocP = 0x11,
        GetTN = 0x13,
        GetTD = 0x14,
        SeekL = 0x15,
        SeekP = 0x16,
        ReadS = 0x1B;

    const int Complete = 0x02;
    const int DataReady = 0x01;
    const byte ModeSize1 = 0x20, ModeSize0 = 0x10;

    const byte StatMotor = 0x02;
    static byte _status;
    static byte _mode;
    static byte _com;
    static readonly byte[] _pos = new byte[4];
    static readonly byte[] _lastResult = new byte[8];
    static readonly object _locatedFileGate = new();
    static readonly Dictionary<int, (int EndLba, string Name)> _locatedFiles = new();
    static int _lastIntr = Complete;

    static uint _cbSync;
    static uint _cbReady;
    static uint _cbData;

    static bool _readActive;
    static bool _xaActive;
    static int _xaReportLba = -1;
    static int _xaPendingReportLba = -1;
    static int _xaLastTraceSecond = -1;
    static volatile bool _cddaActive;
    static int _cddaLba;
    static int _cddaPendingReportLba = -1;
    static volatile int _cddaTrackNumber;
    static int _cddaLastReportSecond = -1;
    static bool _menuMusicWanted;
    static int _menuMusicGraceFrames;
    static byte _filterFile;
    static byte _filterChannel;
    static byte _cdMixLl = 0x80;
    static byte _cdMixLr;
    static byte _cdMixRr = 0x80;
    static byte _cdMixRl;
    static bool _cdMuted;
    static int _v8FileStartLba = -1;
    static int _v82FileStartLba = -1;
    static readonly bool TraceAudio =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_AUDIO") == "1";
    static readonly bool TraceCd =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_CD") == "1";

    internal static readonly object DiscLock = new();
    static readonly object _posGate = new();

    static Thread? _xaThread;
    static volatile bool _xaRun;

    static readonly bool[] NeedsLoc = BuildNeedsLoc();

    static bool[] BuildNeedsLoc()
    {
        var t = new bool[32];
        t[Play] = t[ReadN] = t[SeekL] = t[SeekP] = t[ReadS] = true;
        return t;
    }

    public static void CdInit(CpuContext c, IMemory m)
    {
        CdResetState();
        c.V0 = CdInitInternal() ? 0u : 1u;
    }

    public static void CdReset(CpuContext c, IMemory m)
    {
        CdResetState();
        c.V0 = CdInitInternal() ? 1u : 0u;
    }

    public static void CdControl(CpuContext c, IMemory m) => c.V0 = (uint)(CommandWait(m, (byte)c.A0, c.A1, c.A2, 0) == 0 ? 1 : 0);
    public static void CdControlF(CpuContext c, IMemory m) => c.V0 = (uint)(CommandWait(m, (byte)c.A0, c.A1, 0, 1) == 0 ? 1 : 0);

    public static void CdControlB(CpuContext c, IMemory m)
    {
        if (CommandWait(m, (byte)c.A0, c.A1, c.A2, 0) != 0) { c.V0 = 0; return; }
        c.V0 = (uint)(SyncResult(m, c.A2) == Complete ? 1 : 0);
    }

    public static void CdSync(CpuContext c, IMemory m)
 => c.V0 = (uint)SyncResult(m, c.A1);

    public static void CdReady(CpuContext c, IMemory m)
    {
        if (c.A1 != 0) WriteResult(m, c.A1);
        c.V0 = (uint)_lastIntr;
    }

    public static void CdRead(CpuContext c, IMemory m)
    {
        int sectors = (int)c.A0;
        uint buf = c.A1;
        _mode = (byte)c.A2;
        int lba = CurrentLba;
        int size = SectorSize(_mode);
        Dispatcher.LoadByLba(lba);
        Log.Sdk($"CdRead sectors={sectors} buf=0x{buf:X8} mode=0x{_mode:X2} lba={lba} size={size}");

        for (int i = 0; i < sectors; i++)
        {
            Dispatcher.LoadByLba(lba + i);
            byte[] data;
            lock (DiscLock) data = Runtime.Cd!.ReadSectorData(lba + i, size);
            for (int j = 0; j < data.Length; j++)
                m.WriteU8(buf + (uint)(i * size + j), data[j]);
            if (TryDescribeLocatedFile(
                    lba + i, out string fileName,
                    out int fileStartLba, out _))
                FontFileProvenance.TrackFileRead(
                    fileName,
                    buf + (uint)(i * size),
                    (lba + i - fileStartLba) * size,
                    data.Length);
        }
        _lastIntr = Complete;
        c.V0 = 1;
    }

    internal static int CurrentLba { get { lock (_posGate) return PosToInt(_pos); } }
    internal static double SectorsPerSecond => (_mode & 0x80) != 0 ? 150.0 : 75.0; //cd pacer

    internal static bool AcceptXaSector(byte file, byte channel) =>
        (_mode & 0x08) == 0 || (file == _filterFile && channel == _filterChannel);

    internal static void ReportXaSector(int lba) =>
        Interlocked.Exchange(ref _xaPendingReportLba, lba);

    internal static void MixCdInput(short left, short right, out int mixedLeft, out int mixedRight)
    {
        if (_cdMuted)
        {
            mixedLeft = mixedRight = 0;
            return;
        }
        mixedLeft = Math.Clamp((left * _cdMixLl + right * _cdMixRl) >> 7, short.MinValue, short.MaxValue);
        mixedRight = Math.Clamp((left * _cdMixLr + right * _cdMixRr) >> 7, short.MinValue, short.MaxValue);
    }

    internal static bool TryDescribeLocatedFile(int lba, out string name, out int endLba)
    {
        bool found = TryDescribeLocatedFile(
            lba, out name, out _, out endLba);
        return found;
    }

    internal static bool TryDescribeLocatedFile(
        int lba, out string name, out int startLba, out int endLba)
    {
        lock (_locatedFileGate)
        {
            foreach (var (start, file) in _locatedFiles)
            {
                if (lba < start || lba >= file.EndLba) continue;
                name = file.Name;
                startLba = start;
                endLba = file.EndLba;
                return true;
            }
        }

        if (Runtime.Cd?.Fs.TryDescribeLba(lba, out name, out endLba) == true)
        {
            if (!Runtime.Cd.Fs.Locate(name, out startLba, out _))
                startLba = lba;
            return true;
        }

        name = $"LBA {lba}";
        startLba = lba;
        endLba = int.MaxValue;
        return false;
    }

    internal static void Tick()
    {
        DispatchXaReport();
        DispatchCddaReport();
        MaintainMenuMusic();
        bool xaMode = (_mode & 0x40) != 0;

        if (_readActive && xaMode) return;

        if (!_readActive || _cbData == 0) return;
        while (_cbData != 0)
            ServiceReadOnce();
    }

    public static void NotifyOverlayLoaded(string name)
    {
        if (!Runtime.GameTitle.Contains("2nd Offense", StringComparison.Ordinal))
            return;

        if (name.Equals("SHELL_SHELL", StringComparison.OrdinalIgnoreCase))
        {
            // Stop presenting the expanded gameplay target immediately.
            // Returning to the authored 4:3 shell exposes host-side regions
            // that did not exist in the 16:9 image. The render thread owns
            // those regions and must black them without erasing the shell's
            // native VRAM assets or framebuffer pages.
            Hle.GpuHle.GameplayActive = false;
            Hle.GpuHle.WidescreenMenuReturnPending = true;
            Host.InputManager.SignalScriptStage("shell_transition");
            _menuMusicWanted = true;
            _menuMusicGraceFrames = 180;
            return;
        }

        if (name.Equals("SHELL_LOAD", StringComparison.OrdinalIgnoreCase) ||
            name.StartsWith("LEVELS_", StringComparison.OrdinalIgnoreCase))
        {
            _menuMusicWanted = false;
            _menuMusicGraceFrames = 0;
        }
    }

    static void MaintainMenuMusic()
    {
        if (!_menuMusicWanted || Runtime.Cd == null || _cddaActive) return;
        // Never compete with the game's data or movie/voice streams. The
        // native shell issues Play after its reads are complete; recovery is
        // reserved for a genuinely idle shell (for example after an explicit
        // Stop or after a menu track reaches its end).
        if (_xaActive || _readActive)
        {
            _menuMusicGraceFrames = Math.Max(_menuMusicGraceFrames, 180);
            return;
        }
        if (_menuMusicGraceFrames-- > 0) return;
        if (!Runtime.Cd.TryGetTrackStartLba(2, out int lba)) return;
        StartCdda(lba, "menu recovery");
    }

    static void StartCdda(int lba, string source)
    {
        _readActive = false;
        _xaActive = false;
        _xaReportLba = -1;
        _xaPendingReportLba = -1;
        _xaLastTraceSecond = -1;
        _cddaLba = lba;
        _cddaActive = true;
        _cddaPendingReportLba = -1;
        _cddaLastReportSecond = -1;
        XaAudio.Reset();
        CddaAudio.Reset();
        EnsureXaThread();
        Console.Error.WriteLine($"[CDDA] play LBA={_cddaLba} mode=0x{_mode:X2} source={source}");
    }

    public static void ServiceReadOnce()
    {
        bool xaMode = (_mode & 0x40) != 0;
        if (!_readActive || xaMode || (_cbReady == 0 && _cbData == 0))
            return;

        var c = Runtime.Cpu;
        var m = Runtime.Mem;
        if (c == null || m == null) return;

        var snap = c.Snapshot();
        _lastIntr = DataReady;
        if (_cbReady != 0) { c.A0 = DataReady; c.A1 = 0; Dispatcher.Call(c, m, _cbReady); }
        AdvancePos(1);
        Dispatcher.LoadByLba(CurrentLba);
        if (_cbData != 0) { c.A0 = DataReady; c.A1 = 0; Dispatcher.Call(c, m, _cbData); }
        c.Restore(snap);
    }

    static void EnsureXaThread()
    {
        if (_xaThread is { IsAlive: true }) return;
        _xaRun = true;
        _xaThread = new Thread(XaLoop) { IsBackground = true, Name = "CdXa" };
        _xaThread.Start();
    }

    static void XaLoop()
    {
        while (_xaRun)
        {
            if (_cddaActive && Runtime.Cd != null)
                PumpCdda();
            else if (_readActive && (_mode & 0x40) != 0 && Runtime.Cd != null)
                PumpXa();
            Thread.Sleep(2);
        }
    }

    static void PumpCdda()
    {
        var cd = Runtime.Cd;
        if (cd == null) return;
        const int MinBufferFrames = 4096;
        const int MaxScan = 16;
        int scanned = 0;

        while (_cddaActive && CddaAudio.BufferedFrames < MinBufferFrames && scanned++ < MaxScan)
        {
            int lba = _cddaLba;
            byte[] sector;
            int trackNumber;
            int trackEndLba;
            lock (DiscLock)
            {
                if (!cd.TryReadAudioSector(lba, out sector, out trackNumber, out trackEndLba))
                {
                    _cddaActive = false;
                    Console.Error.WriteLine($"[CDDA] stopped at unmapped audio LBA {lba}");
                    return;
                }
            }

            _cddaTrackNumber = trackNumber;
            CddaAudio.QueueSector(sector, trackNumber, lba);
            _cddaLba = lba + 1;
            int reportSecond = lba / 75;
            if (reportSecond != _cddaLastReportSecond)
            {
                _cddaLastReportSecond = reportSecond;
                Interlocked.Exchange(ref _cddaPendingReportLba, lba);
            }
            if (lba + 1 >= trackEndLba) return;
        }
    }

    static void DispatchCddaReport()
    {
        int lba = Interlocked.Exchange(ref _cddaPendingReportLba, -1);
        if (lba < 0 || _cbReady == 0 || Runtime.Cpu == null || Runtime.Mem == null) return;

        IntToPos(lba, out byte mm, out byte ss, out byte ff);
        _lastResult[0] = _status;
        _lastResult[1] = ToBcd(_cddaTrackNumber);
        _lastResult[2] = 1;
        _lastResult[3] = mm;
        _lastResult[4] = ss;
        _lastResult[5] = ff;
        _lastResult[6] = mm;
        _lastResult[7] = ss;

        var c = Runtime.Cpu;
        var m = Runtime.Mem;
        const uint resultAddress = 0x8000FF00u;
        WriteResult(m, resultAddress);
        var snap = c.Snapshot();
        c.A0 = DataReady;
        c.A1 = resultAddress;
        Dispatcher.Call(c, m, _cbReady);
        c.Restore(snap);
    }

    static void DispatchXaReport()
    {
        int lba = Interlocked.Exchange(ref _xaPendingReportLba, -1);
        if (lba < 0 || !_xaActive || _cbReady == 0 || Runtime.Cpu == null || Runtime.Mem == null)
            return;

        _xaReportLba = lba;
        int traceSecond = lba / 75;
        if (TraceAudio && traceSecond != _xaLastTraceSecond)
        {
            _xaLastTraceSecond = traceSecond;
            Console.Error.WriteLine($"[XA-REPORT] lba={lba} ready=0x{_cbReady:X8}");
        }
        var c = Runtime.Cpu;
        var m = Runtime.Mem;
        var snap = c.Snapshot();
        _lastIntr = DataReady;
        c.A0 = DataReady;
        c.A1 = 0;
        Dispatcher.Call(c, m, _cbReady);
        c.Restore(snap);
    }

    static void PumpXa()
    {
        if (Runtime.Cd == null) return;
        const int MinBuffer = 4096;
        const int MaxScan = 32;
        bool useFilter = (_mode & 0x08) != 0;
        int scanned = 0;

        while (_readActive && XaAudio.BufferedSamples < MinBuffer && scanned < MaxScan)
        {
            int lba = CurrentLba;
            if (lba < 0) break;
            byte[] sec;
            lock (DiscLock) sec = Runtime.Cd.ReadSectorData(lba, 2336);
            AdvancePos(1);
            scanned++;
            if ((sec[2] & 0x04) == 0) continue;
            if (useFilter && (sec[0] != _filterFile || sec[1] != _filterChannel)) continue;
            XaAudio.DecodeSector(sec, 8, sec[3]);
        }
    }

    static void AdvancePos(int n)
    {
        lock (_posGate)
            IntToPos(PosToInt(_pos) + n, out _pos[0], out _pos[1], out _pos[2]);
    }

    public static void CdReadSync(CpuContext c, IMemory m)
    {
        if (c.A1 != 0) WriteResult(m, c.A1);
        c.V0 = 0;
    }

    public static void CdGetSector(CpuContext c, IMemory m)
    {
        uint madr = c.A0;
        int words = (int)c.A1;
        // The retail XA-ready callback asks for exactly one word and treats it
        // as the current absolute MSF. Preserve normal sector payload reads.
        if (_xaActive && words == 1 && _xaReportLba >= 0)
        {
            IntToPos(_xaReportLba, out byte mm, out byte ss, out byte ff);
            m.WriteU8(madr, mm);
            m.WriteU8(madr + 1, ss);
            m.WriteU8(madr + 2, ff);
            m.WriteU8(madr + 3, 0);
            c.V0 = 1;
            return;
        }
        int lba = CurrentLba;
        byte[] data;
        lock (DiscLock) data = Runtime.Cd!.ReadSectorData(lba);
        int bytes = Math.Min(data.Length, words * 4);
        for (int j = 0; j < bytes; j++)
            m.WriteU8(madr + (uint)j, data[j]);
        if (TryDescribeLocatedFile(
                lba, out string fileName, out int fileStartLba, out _))
            FontFileProvenance.TrackFileRead(
                fileName, madr,
                (lba - fileStartLba) * data.Length,
                bytes);
        if (TraceCd)
        {
            string prefix = Convert.ToHexString(data.AsSpan(0, Math.Min(32, data.Length)));
            Console.Error.WriteLine(
                $"[LibCd] CdGetSector LBA={lba} dest=0x{madr:X8} words={words} data={prefix}");
        }
        c.V0 = 1;
    }

    public static void CdDataSync(CpuContext c, IMemory m) => c.V0 = 0;

    public static void CdSearchFile(CpuContext c, IMemory m)
    {
        uint fp = c.A0;
        string name = ReadCString(m, c.A1);

        if (Runtime.Cd == null || !Runtime.Cd.Fs.Locate(name, out int lba, out uint size))
        {
            Log.Sdk($"CdSearchFile '{name}'wasnt found");
            if (name.Contains("V8VOICE", StringComparison.OrdinalIgnoreCase))
                Console.Error.WriteLine(
                    $"[V82ResultVoice] CdSearchFile failed '{name}'");
            c.V0 = 0;
            return;
        }
        Log.Sdk($"CdSearchFile '{name}' lba={lba} size={size}");
        if (name.Contains("V8VOICE", StringComparison.OrdinalIgnoreCase))
            Console.Error.WriteLine(
                $"[V82ResultVoice] CdSearchFile '{name}' " +
                $"lba={lba} logicalSize={size}");
        lock (_locatedFileGate)
            _locatedFiles[lba] = (lba + (int)((size + 2047u) >> 11), name);

        IntToPos(lba, out byte mm, out byte ss, out byte ff);
        m.WriteU8(fp + 0, mm);
        m.WriteU8(fp + 1, ss);
        m.WriteU8(fp + 2, ff);
        m.WriteU8(fp + 3, 0);
        m.WriteU32(fp + 4, size);

        int slash = name.LastIndexOfAny(['/', '\\']);
        string basename = slash >= 0 ? name[(slash + 1)..] : name;
        
        for (int i = 0; i < 16; i++)
        {
            m.WriteU8(fp + 8 + (uint)i, i < basename.Length ? (byte)basename[i] : (byte)0);
        }

        c.V0 = fp;
    }

    public static void CdSyncCallback(CpuContext c, IMemory m) { c.V0 = _cbSync; _cbSync = c.A0; }
    public static void CdReadyCallback(CpuContext c, IMemory m) { c.V0 = _cbReady; _cbReady = c.A0; }
    public static void CdReadCallback(CpuContext c, IMemory m) { c.V0 = _cbData; _cbData = c.A0; }
    public static void CdDataCallback(CpuContext c, IMemory m) { c.V0 = _cbData; _cbData = c.A0; }

    public static void CdStatus(CpuContext c, IMemory m) => c.V0 = _status;
    public static void CdMode(CpuContext c, IMemory m) => c.V0 = _mode;
    public static void CdLastCom(CpuContext c, IMemory m) => c.V0 = _com;
    public static void CdMix(CpuContext c, IMemory m)
    {
        if (c.A0 != 0)
        {
            _cdMixLl = m.ReadU8(c.A0);
            _cdMixLr = m.ReadU8(c.A0 + 1);
            _cdMixRr = m.ReadU8(c.A0 + 2);
            _cdMixRl = m.ReadU8(c.A0 + 3);
            if (TraceAudio)
                Console.Error.WriteLine(
                    $"[CDMIX] ll={_cdMixLl} lr={_cdMixLr} rr={_cdMixRr} rl={_cdMixRl}");
        }
        c.V0 = 1;
    }

    static void CdResetState()
    {
        LibCdStream.OnStopStream();
        _status = StatMotor; //drive aways spin
        _mode = 0;
        _com = 0;
        _lastIntr = Complete;
        _cbSync = _cbReady = _cbData = 0;
        _readActive = false;
        _xaActive = false;
        _xaReportLba = -1;
        _xaPendingReportLba = -1;
        _xaLastTraceSecond = -1;
        _cddaActive = false;
        _cddaLba = 0;
        _cddaPendingReportLba = -1;
        _cddaTrackNumber = 0;
        _cddaLastReportSecond = -1;
        _menuMusicWanted = false;
        _menuMusicGraceFrames = 0;
        CddaAudio.Reset();
        XaAudio.Reset();
        _filterFile = _filterChannel = 0;
        _cdMixLl = _cdMixRr = 0x80;
        _cdMixLr = _cdMixRl = 0;
        _cdMuted = false;
        _v8FileStartLba = _v82FileStartLba = -1;
        Array.Clear(_pos);
        Array.Clear(_lastResult);
        Dispatcher.ClearPending();
    }

    static bool CdInitInternal()
    {
        _lastIntr = Complete;
        _lastResult[0] = _status;
        return true;
    }

    static int CommandWait(IMemory m, byte com, uint param, uint result, uint arg)
    {
        if (param != 0 && com < NeedsLoc.Length && NeedsLoc[com])
            ExecCommand(m, Setloc, param, 0);
        return ExecCommand(m, com, param, result);
    }

    static int ExecCommand(IMemory m, byte com, uint param, uint result)
    {
        _com = com;
        _lastIntr = Complete;
        Log.Sdk($"Cd cmd 0x{com:X2} param=0x{param:X8} pos={_pos[0]:X2}:{_pos[1]:X2}:{_pos[2]:X2}");

        switch (com)
        {
            case Setloc:
                if (param != 0)
                    lock (_posGate)
                        for (int i = 0; i < 4; i++) _pos[i] = m.ReadU8(param + (uint)i);
                break;
            case Setmode:
                if (param != 0) _mode = m.ReadU8(param);
                break;
            case Setfilter:
                if (param != 0) { _filterFile = m.ReadU8(param); _filterChannel = m.ReadU8(param + 1); }
                break;
            case ReadN:
                _cddaActive = false;
                CddaAudio.Reset();
                XaAudio.Reset();
                _xaActive = false;
                _xaReportLba = -1;
                _xaPendingReportLba = -1;
                _xaLastTraceSecond = -1;
                _readActive = true;
                Dispatcher.LoadByLba(CurrentLba);
                Console.Error.WriteLine(
                    $"[LibCd] ReadN start LBA={CurrentLba} msf={_pos[0]:X2}:{_pos[1]:X2}:{_pos[2]:X2} " +
                    $"ready=0x{_cbReady:X8} data=0x{_cbData:X8}");
                EnsureXaThread();
                break;
            case ReadS:
                _cddaActive = false;
                CddaAudio.Reset();
                XaAudio.Reset();
                _xaActive = true;
                _xaReportLba = CurrentLba;
                _xaPendingReportLba = -1;
                _xaLastTraceSecond = -1;
                _readActive = false;
                LibCdStream.OnReadStream(CurrentLba);
                break;
            case GetlocL:
            case GetlocP:
                lock (_posGate)
                {
                    _lastResult[0] = _pos[0];
                    _lastResult[1] = _pos[1];
                    _lastResult[2] = _pos[2];
                }
                _lastResult[3] = _mode;
                _lastResult[4] = _filterFile;
                _lastResult[5] = _filterChannel;
                _lastResult[6] = 0;
                _lastResult[7] = 0;
                if (result != 0) WriteResult(m, result);
                return 0;
            case GetTN:
                if (Runtime.Cd == null) return -1;
                _lastResult[0] = _status;
                _lastResult[1] = ToBcd(Runtime.Cd.FirstTrackNumber);
                _lastResult[2] = ToBcd(Runtime.Cd.LastTrackNumber);
                for (int i = 3; i < _lastResult.Length; i++) _lastResult[i] = 0;
                if (result != 0) WriteResult(m, result);
                Console.Error.WriteLine(
                    $"[CDDA] TOC tracks={Runtime.Cd.FirstTrackNumber}-{Runtime.Cd.LastTrackNumber}");
                return 0;
            case GetTD:
                if (Runtime.Cd == null) return -1;
                int trackNumber = param == 0 ? 0 : Bcd(m.ReadU8(param));
                int trackLba;
                if (trackNumber == 0) trackLba = Runtime.Cd.LeadOutLba;
                else if (!Runtime.Cd.TryGetTrackStartLba(trackNumber, out trackLba)) return -1;
                IntToPos(trackLba, out byte tdMm, out byte tdSs, out byte tdFf);
                _lastResult[0] = _status;
                _lastResult[1] = tdMm;
                _lastResult[2] = tdSs;
                _lastResult[3] = tdFf;
                for (int i = 4; i < _lastResult.Length; i++) _lastResult[i] = 0;
                if (result != 0) WriteResult(m, result);
                return 0;
            case Pause: case Stop: case Init:
                LibCdStream.OnStopStream();
                _readActive = false;
                _xaActive = false;
                _xaReportLba = -1;
                _xaPendingReportLba = -1;
                _xaLastTraceSecond = -1;
                _cddaActive = false;
                CddaAudio.Reset();
                XaAudio.Reset();
                Dispatcher.ClearPending();
                break;
            case Play:
                StartCdda(CurrentLba, "native command");
                break;
            case Mute:
                _cdMuted = true;
                if (TraceAudio) Console.Error.WriteLine("[CDMIX] muted");
                break;
            case Demute:
                _cdMuted = false;
                if (TraceAudio) Console.Error.WriteLine("[CDMIX] demuted");
                break;
            case Nop: case SeekL: case SeekP:
                break;
            default:
                break;
        }

        _lastResult[0] = _status;
        for (int i = 1; i < _lastResult.Length; i++) _lastResult[i] = 0;
        if (result != 0) WriteResult(m, result);
        return 0;
    }

    static void DeliverInitialReadyCallback(IMemory m)
    {
        if (_cbReady == 0 || Runtime.Cpu == null) return;

        var c = Runtime.Cpu;
        var snap = c.Snapshot();
        if (TraceCd)
            Console.Error.WriteLine($"[LibCd] DeliverInitial before callback LBA={CurrentLba}");
        _lastIntr = DataReady;
        c.A0 = DataReady;
        c.A1 = 0;
        Dispatcher.Call(c, m, _cbReady);
        if (TraceCd)
            Console.Error.WriteLine($"[LibCd] DeliverInitial after callback LBA={CurrentLba}");
        AdvancePos(1);
        Dispatcher.LoadByLba(CurrentLba);
        if (_cbData != 0)
        {
            c.A0 = DataReady;
            c.A1 = 0;
            Dispatcher.Call(c, m, _cbData);
        }
        c.Restore(snap);
    }

    public static void WaitForV8Sector(CpuContext c, IMemory m)
    {
        uint consumedPtr = m.ReadU32(c.GP + 0x6A4u);
        uint producedPtr = m.ReadU32(c.GP + 0x6A8u);
        if (producedPtr == consumedPtr)
        {
            DeliverInitialReadyCallback(m);
            producedPtr = m.ReadU32(c.GP + 0x6A8u);
        }
        m.WriteU32(c.GP + 0x6A4u, producedPtr);
        c.V0 = consumedPtr;
    }

    public static void BeginV8FileRead(CpuContext c, IMemory m)
    {
        _v8FileStartLba = unchecked((int)c.A0);
    }

    public static void BeginV82FileRead(CpuContext c, IMemory m)
    {
        _v82FileStartLba = unchecked((int)c.A0);
    }

    // Vigilante 8's file reader consumes a two-sector callback ring. On the
    // original console the CD interrupt can refill that ring during a long
    // memcpy; the recompiled single-threaded path cannot reproduce that timing
    // safely. Read the same 2048-byte sectors directly while retaining the
    // game's byte-offset state and public reader semantics.
    public static void ReadV8FileBytes(CpuContext c, IMemory m)
    {
        ReadFileBytes(c, m, _v8FileStartLba, c.GP + 0x6ACu, "Vigilante 8");
    }

    public static void ReadV82FileBytes(CpuContext c, IMemory m)
    {
        ReadFileBytes(c, m, _v82FileStartLba, c.GP + 0xD64u, "Vigilante 8: 2nd Offense");
    }

    static void ReadFileBytes(
        CpuContext c, IMemory m, int fileStartLba, uint offsetAddress, string game)
    {
        if (fileStartLba < 0 || Runtime.Cd == null)
            throw new InvalidOperationException($"{game} file read started without a disc/LBA");

        m = Dispatcher.UnwrapMemory(m);
        uint destination = c.A0;
        uint length = c.A1;
        uint offset = m.ReadU32(offsetAddress);

        if (Runtime.Cd.Fs.TryReadLooseFileRange(
                fileStartLba, offset, checked((int)length), out byte[] looseData))
        {
            for (int i = 0; i < looseData.Length; i++)
                m.WriteU8(destination + (uint)i, looseData[i]);
            m.WriteU32(offsetAddress, offset + length);
            c.V0 = 1u;
            return;
        }

        uint copied = 0u;
        while (copied < length)
        {
            int lba = fileStartLba + (int)(offset >> 11);
            int inSector = (int)(offset & 0x7FFu);
            byte[] sector;
            lock (DiscLock) sector = Runtime.Cd.ReadSectorData(lba, 2048);
            int take = Math.Min((int)(length - copied), 2048 - inSector);
            for (int i = 0; i < take; i++)
            {
                uint writeAddress = destination + copied + (uint)i;
                m.WriteU8(writeAddress, sector[inSector + i]);
            }
            copied += (uint)take;
            offset += (uint)take;
        }

        m.WriteU32(offsetAddress, offset);
        c.V0 = 1u;
    }

    public static void SeekV8File(CpuContext c, IMemory m)
    {
        SeekFile(c, m, c.GP + 0x6ACu);
    }

    public static void SeekV82File(CpuContext c, IMemory m)
    {
        SeekFile(c, m, c.GP + 0xD64u);
    }

    static void SeekFile(CpuContext c, IMemory m, uint offsetAddress)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint current = m.ReadU32(offsetAddress);
        uint target = c.A1 == 0u ? c.A0 : current + c.A0;
        m.WriteU32(offsetAddress, target);
    }

    static int SyncResult(IMemory m, uint result)
    {
        if (result != 0) WriteResult(m, result);
        return _lastIntr;
    }

    static void WriteResult(IMemory m, uint addr)
    {
        for (int i = 0; i < _lastResult.Length; i++)
            m.WriteU8(addr + (uint)i, _lastResult[i]);
    }

    static int SectorSize(byte mode)
    {
        if ((mode & ModeSize1) != 0) return 2340;
        if ((mode & ModeSize0) != 0) return 2328;
        return 2048;
    }
    
    static string ReadCString(IMemory m, uint addr)
    {
        var sb = new System.Text.StringBuilder();
        for (uint i = 0; i < 128; i++)
        {
            byte b = m.ReadU8(addr + i);
            if (b == 0) break;
            sb.Append((char)b);
        }
        return sb.ToString();
    }

    static int Bcd(byte b) => (b >> 4) * 10 + (b & 0xF);
    static byte ToBcd(int n) => (byte)(((n / 10) << 4) + (n % 10));

    static int PosToInt(byte[] p) => (Bcd(p[0]) * 60 + Bcd(p[1])) * 75 + Bcd(p[2]) - 150;

    static void IntToPos(int i, out byte mm, out byte ss, out byte ff)
    {
        i += 150;
        ff = ToBcd(i % 75);
        ss = ToBcd(i / 75 % 60);
        mm = ToBcd(i / 75 / 60);
    }
}
