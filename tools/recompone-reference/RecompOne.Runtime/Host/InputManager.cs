using Silk.NET.Input;
using Silk.NET.SDL;
using System.Linq;
using System.Runtime.InteropServices;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Hle;

namespace RecompOne.Runtime.Host;

internal static unsafe class InputManager
{
    static IKeyboard?_keyboard;
    static readonly ushort[] _joinStates = new ushort[4];
    static readonly bool[] _joinConnected = new bool[4];
    internal static ushort JoinDeviceState(int device) => device == LocalInputSession.KeyboardDevice
        ? LocalInputSession.KeyboardState : _joinStates[device];
    internal static bool JoinDeviceConnected(int device) => device == LocalInputSession.KeyboardDevice
        ? LocalInputSession.KeyboardConnected : _joinConnected[device];
    readonly record struct KeyPulse(string? Stage, int Start, int End, Key Key);
    static readonly List<KeyPulse> _scriptedKeys = new();
    static readonly HashSet<Key> _fakeKeys = new();
    static Sdl?_sdl;
    static readonly nint[] _pads = new nint[4];
    static readonly int[] _padIds = [-1, -1, -1, -1];
    static GameController* PadHandle(int pad) => (uint)pad < 4 ? (GameController*)_pads[pad] : null;
    static GameController* _pad0 => PadHandle(0);
    static GameController* _pad1 => PadHandle(1);

    const int AxisThreshold = 8000;
    const int StickThreshold = 16000;
    const int LeftTrigger = 100;
    const int RightTrigger = 101;
    const int LeftStickLeft = 102;
    const int LeftStickRight = 103;
    const int LeftStickUp = 104;
    const int LeftStickDown = 105;
    const int RightStickLeft = 106;
    const int RightStickRight = 107;
    const int RightStickUp = 108;
    const int RightStickDown = 109;
    static bool _topBarToggle;
    static bool _fullscreenToggle;
    readonly record struct ScriptedPulse(
        string? Stage, int Start, int End, ushort Pad1Mask, ushort Pad2Mask, ushort Pad3Mask, ushort Pad4Mask);

    static readonly List<ScriptedPulse> _scriptedInput = new();

    // Physical-input pulses. An ordinary scripted pulse writes Controller.State
    // directly and so bypasses profiles, ResolvePad and Pressed entirely --
    // which means no headless run has ever covered the binding layer, the very
    // place a "this button does not work" report lives. A physical pulse
    // instead marks a controller input as held and lets the real resolution
    // path decide which native bit that produces.
    readonly record struct PhysicalPulse(
        string? Stage, int Start, int End, int[] Codes, int Pad);

    static readonly List<PhysicalPulse> _scriptedPhysical = new();
    static readonly HashSet<int>[] _fakePressed = [new(), new(), new(), new()];
    static readonly bool[] _fakeConnected = new bool[4];
    static readonly bool[] _fakeDisconnected = new bool[4];
    static bool _fakePadActive;

    static readonly Dictionary<string, int> PhysicalNames = new(
        StringComparer.OrdinalIgnoreCase)
    {
        ["DISCONNECTED"] = -1,
        ["A"] = 0, ["B"] = 1, ["X"] = 2, ["Y"] = 3,
        ["BACK"] = 4, ["START"] = 6, ["LS"] = 7, ["RS"] = 8,
        ["LB"] = 9, ["RB"] = 10,
        ["DPADUP"] = 11, ["DPADDOWN"] = 12,
        ["DPADLEFT"] = 13, ["DPADRIGHT"] = 14,
        ["LT"] = LeftTrigger, ["RT"] = RightTrigger,
        ["LSLEFT"] = LeftStickLeft, ["LSRIGHT"] = LeftStickRight,
        ["LSUP"] = LeftStickUp, ["LSDOWN"] = LeftStickDown,
        ["RSLEFT"] = RightStickLeft, ["RSRIGHT"] = RightStickRight,
        ["RSUP"] = RightStickUp, ["RSDOWN"] = RightStickDown,
    };
    static int _inputPoll;
    static string? _scriptStage;
    static int _stagePoll;
    static int _stageCapturePoll = -1;
    static string? _stageCaptureLabel;
    static bool _disableLiveInput;
    static bool _forcePad2Connected;
    static bool _traceInput;
    static string? _captureScriptedStage;
    static bool _disableScriptStageCaptures;
    static HashSet<string>? _scriptStageCaptureFilter;
    static bool _suppressRumble;
    static int _scriptExitAfterPoll = -1;
    static string? _scriptExitAfterStage;
    static int _scriptExitAfterStageVisits = -1;
    static readonly Dictionary<string, int> _scriptStageVisits =
        new(StringComparer.OrdinalIgnoreCase);
    static readonly Dictionary<string, int> _scriptStageStartPolls =
        new(StringComparer.OrdinalIgnoreCase);
    static int _nativeGameplayMenuPolls;
    static bool _readyPromptActive;
    static readonly bool[] _promptKeyboard = new bool[4];
    internal static bool NativeMenuActive => _nativeGameplayMenuPolls > 0 && !_readyPromptActive;
    internal static void SignalReadyPrompt() => _readyPromptActive = true;
    internal static void EndReadyPrompt() => _readyPromptActive = false;
    internal static bool PromptUsesGamepad(int player)
    {
        int device = LocalInputSession.DeviceForPlayer(player);
        return (uint)device < 4 && (_fakeConnected[device] ||
            (IsPadConnected(device) && (LocalInputSession.PlayerCount > 0 || !_promptKeyboard[device])));
    }
    static readonly (byte Large, byte Small)[] _lastRumble =
        Enumerable.Repeat((byte.MaxValue, byte.MaxValue), 4).ToArray();

    
    public static bool ConsumeTopBarToggle() { var v = _topBarToggle; _topBarToggle = false; return v; }
    public static bool ConsumeFullscreenToggle(){ var v = _fullscreenToggle; _fullscreenToggle = false; return v; }

    public static void Initialize(IInputContext input)
    {
        InitializeConfiguration();
        if (input.Keyboards.Count > 0)
        {
            _keyboard = input.Keyboards[0];
            _keyboard.KeyDown += OnKeyDown;
        }
        
        
        try
        {
            _sdl = Sdl.GetApi();
            _sdl.SetHint("SDL_JOYSTICK_RAWINPUT", "0");
            _sdl.InitSubSystem(Sdl.InitGamecontroller);
            Rescan();
            if (Environment.GetEnvironmentVariable("RECOMPONE_TEST_CONTROLLER_RESCAN") == "1")
            {
                string before = $"p1={ControllerName(_pad0)} p2={ControllerName(_pad1)}";
                Rescan();
                string after = $"p1={ControllerName(_pad0)} p2={ControllerName(_pad1)}";
                Console.WriteLine(
                    $"[Input] controller rescan self-test before=({before}) after=({after})");
            }
            if (Environment.GetEnvironmentVariable("RECOMPONE_TEST_RUMBLE") == "1")
            {
                SetRumble(0, 128, 1);
                SetRumble(1, 96, 1);
                SetRumble(0, 0, 0);
                SetRumble(1, 0, 0);
            }
        }
        catch { _sdl = null; }
    }

    public static void InitializeHeadless()
    {
        InitializeConfiguration();
        _disableLiveInput = true;
        Console.Error.WriteLine(
            "[Input] headless deterministic input initialized");
    }

    static void InitializeConfiguration()
    {
        _disableLiveInput =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_DISABLE_LIVE_INPUT") == "1";
        _forcePad2Connected =
            Environment.GetEnvironmentVariable("RECOMPONE_FORCE_PAD2_CONNECTED") == "1";
        _traceInput = Environment.GetEnvironmentVariable("RECOMPONE_TRACE_INPUT") == "1";
        string? captureScriptedStage =
            Environment.GetEnvironmentVariable("RECOMPONE_CAPTURE_SCRIPTED_STAGE");
        _captureScriptedStage = string.IsNullOrWhiteSpace(captureScriptedStage)
            ? null
            : NormalizeStage(captureScriptedStage);
        _disableScriptStageCaptures =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES") == "1";
        string? scriptStageCaptureFilter =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_SCRIPT_STAGE_CAPTURE_FILTER");
        _scriptStageCaptureFilter = string.IsNullOrWhiteSpace(
                scriptStageCaptureFilter)
            ? null
            : scriptStageCaptureFilter
                .Split(',', StringSplitOptions.RemoveEmptyEntries |
                            StringSplitOptions.TrimEntries)
                .Select(NormalizeStage)
                .ToHashSet(StringComparer.OrdinalIgnoreCase);
        _suppressRumble =
            Environment.GetEnvironmentVariable("RECOMPONE_SUPPRESS_RUMBLE") == "1";
        string? exitAfterPoll =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS");
        _scriptExitAfterPoll =
            int.TryParse(exitAfterPoll, out int parsedExitPoll) &&
            parsedExitPoll > 0
                ? parsedExitPoll
                : -1;
        string? exitAfterStage =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE");
        _scriptExitAfterStage = string.IsNullOrWhiteSpace(exitAfterStage)
            ? null
            : NormalizeStage(exitAfterStage);
        string? exitAfterStageVisits =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE_VISITS");
        _scriptExitAfterStageVisits =
            int.TryParse(exitAfterStageVisits, out int parsedStageVisits) &&
            parsedStageVisits > 0
                ? parsedStageVisits
                : -1;
        ParseScriptedInput();
        if (_disableLiveInput)
            Console.Error.WriteLine("[Input] live keyboard/gamepad input disabled for deterministic replay");
        if (_forcePad2Connected)
            Console.Error.WriteLine("[Input] controller 2 connection forced for deterministic replay");
    }

    public static bool IsConnected => _pad0 != null;

    public static bool IsPadConnected(int pad) => PadHandle(pad) != null;

    public static bool IsKeyDown(Key k) => _fakeKeys.Contains(k) || (!_disableLiveInput && (_keyboard?.IsKeyPressed(k) ?? false));

    // Refreshed by the retail PAUSED / QUEST OBJECTIVES text path. Keeping the
    // context pulse at the native UI seam lets Trigger Drive retain ordinary
    // A/B/X/Y menu navigation without inventing a second pause-state machine.
    internal static void SignalNativeGameplayMenu() =>
        _nativeGameplayMenuPolls = 8;

    public static void Poll()
    {
        for (int pad = 0; pad < 4; pad++)
        {
            Controller.SetState(pad, 0xFFFF);
            Controller.SetAxes(pad, 128, 128, 128, 128);
        }
        if (!_disableLiveInput)
        {
            PollGamepadEvents();
            PollGamepads();
        }
        ApplyScriptedInput();
        Controller.State &= (ushort)~V8Compat.GetAutomationInputMask();
        Controller.State &= (ushort)~V82Compat.GetAutomationInputMask();
        for (int pad = 0; pad < 4; pad++)
        {
            Controller.LocalConnected[pad] = !_fakeDisconnected[pad] && (_fakeConnected[pad] ||
                (!_disableLiveInput && PadHandle(pad) != null));
            if (_fakeDisconnected[pad])
            {
                Controller.SetState(pad, 0xFFFF);
                Controller.SetAxes(pad, 128, 128, 128, 128);
            }
        }
        for (int device = 0; device < 4; device++)
        {
            _joinStates[device] = Controller.GetState(device);
            _joinConnected[device] = Controller.LocalConnected[device];
        }
        PollKeyboard();
        Controller.Connected2 = _forcePad2Connected || Controller.LocalConnected[1];
        LocalInputSession.RoutePolledDevices();
        if (_nativeGameplayMenuPolls > 0)
            _nativeGameplayMenuPolls--;
    }

    static void ParseScriptedInput()
    {
        _scriptedKeys.Clear();
        _fakeKeys.Clear();
        _scriptedInput.Clear();
        _scriptedPhysical.Clear();
        _fakePadActive = false;
        Array.Clear(_fakeConnected);
        _inputPoll = 0;
        _scriptStage = null;
        _stagePoll = 0;
        _stageCapturePoll = -1;
        _stageCaptureLabel = null;
        _scriptStageVisits.Clear();
        _scriptStageStartPolls.Clear();

        string? script = Environment.GetEnvironmentVariable("RECOMPONE_INPUT_SCRIPT");
        string? scriptFile = Environment.GetEnvironmentVariable("RECOMPONE_INPUT_FILE");
        if (!string.IsNullOrWhiteSpace(script) && !string.IsNullOrWhiteSpace(scriptFile))
            throw new InvalidOperationException("Set only one of RECOMPONE_INPUT_SCRIPT or RECOMPONE_INPUT_FILE");

        if (!string.IsNullOrWhiteSpace(scriptFile))
        {
            string fullPath = Path.GetFullPath(scriptFile);
            if (!File.Exists(fullPath))
                throw new FileNotFoundException("Scripted input fixture was not found", fullPath);

            script = string.Join('\n', File.ReadLines(fullPath).Select(line => line.Split('#', 2)[0]));
            Console.Error.WriteLine($"[Input] fixture: {fullPath}");
        }
        if (string.IsNullOrWhiteSpace(script)) return;

        string? stage = null;
        foreach (string raw in script.Split([';', '\r', '\n'], StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            if (raw.StartsWith('[') && raw.EndsWith(']'))
            {
                string header = raw[1..^1].Trim();
                bool afterStage = header.StartsWith(
                    "after:", StringComparison.OrdinalIgnoreCase);
                string normalized = NormalizeStage(
                    afterStage ? header[6..] : header);
                if (normalized.Length == 0)
                    throw new InvalidOperationException(
                        "Scripted input stage name cannot be empty");
                stage = afterStage ? $"after:{normalized}" : normalized;
                continue;
            }

            string[] sides = raw.Split('=', 2, StringSplitOptions.TrimEntries);
            string[] range = sides[0].Split('+', 2, StringSplitOptions.TrimEntries);
            if (sides.Length != 2 || range.Length != 2 ||
                !int.TryParse(range[0], out int start) || !int.TryParse(range[1], out int duration) ||
                start < 0 || duration <= 0)
                throw new InvalidOperationException($"Invalid RECOMPONE_INPUT_SCRIPT entry: {raw}");

            ushort pad1Mask = 0;
            ushort pad2Mask = 0;
            ushort pad3Mask = 0;
            ushort pad4Mask = 0;
            var physical = new List<(int Code, int Pad)>();
            foreach (string token in sides[1].Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
            {
                if (token.StartsWith("KEY:", StringComparison.OrdinalIgnoreCase))
                {
                    if (!Enum.TryParse<Key>(token[4..], true, out var key))
                        throw new InvalidOperationException($"Unknown scripted keyboard key: {token}");
                    _scriptedKeys.Add(new KeyPulse(stage, start, start + duration, key));
                    continue;
                }
                string button = token;
                int pad = 1;
                int colon = token.IndexOf(':');
                // Only a pad selector is consumed here; "PHYS:" is part of the
                // input name and must survive for the physical branch below.
                if (colon >= 0)
                {
                    string prefix = token[..colon].Trim();
                    int? selected = prefix.ToUpperInvariant() switch
                    {
                        "P1" or "PAD1" => 1,
                        "P2" or "PAD2" => 2,
                        "P3" or "PAD3" => 3,
                        "P4" or "PAD4" => 4,
                        "PHYS" => null,
                        _ => throw new InvalidOperationException(
                            $"Unknown scripted controller prefix: {prefix}"),
                    };
                    if (selected is int padIndex)
                    {
                        pad = padIndex;
                        button = token[(colon + 1)..].Trim();
                    }
                }

                if (button.StartsWith("PHYS:", StringComparison.OrdinalIgnoreCase))
                {
                    string name = button[5..].Trim();
                    if (!PhysicalNames.TryGetValue(name, out int code))
                        throw new InvalidOperationException(
                            $"Unknown scripted physical input: {name}");
                    physical.Add((code, pad));
                    _fakeConnected[pad - 1] = true;
                    continue;
                }

                ushort mask = button.ToUpperInvariant() switch
                {
                    "NONE" => 0,
                    "CROSS" => Controller.Cross,
                    "CIRCLE" => Controller.Circle,
                    "SQUARE" => Controller.Square,
                    "TRIANGLE" => Controller.Triangle,
                    "START" => Controller.Start,
                    "SELECT" => Controller.Select,
                    "UP" => Controller.Up,
                    "DOWN" => Controller.Down,
                    "LEFT" => Controller.Left,
                    "RIGHT" => Controller.Right,
                    "L1" => Controller.L1,
                    "R1" => Controller.R1,
                    "L2" => Controller.L2,
                    "R2" => Controller.R2,
                    _ => throw new InvalidOperationException($"Unknown scripted controller button: {button}"),
                };
                if (mask != 0) _fakeConnected[pad - 1] = true;
                if (pad == 1) pad1Mask |= mask;
                else if (pad == 2) pad2Mask |= mask;
                else if (pad == 3) pad3Mask |= mask;
                else pad4Mask |= mask;
            }
            if (physical.Count > 0)
            {
                _fakePadActive = true;
                foreach (var group in physical.GroupBy(entry => entry.Pad))
                    _scriptedPhysical.Add(new PhysicalPulse(
                        stage, start, start + duration,
                        group.Select(entry => entry.Code).ToArray(),
                        group.Key - 1));
            }
            _scriptedInput.Add(new ScriptedPulse(
                stage, start, start + duration, pad1Mask, pad2Mask, pad3Mask, pad4Mask));
        }

        int staged = _scriptedInput.Count(pulse => pulse.Stage != null);
        Console.Error.WriteLine(
            $"[Input] loaded {_scriptedInput.Count} scripted input pulses ({staged} stage-relative)");
    }

    static string NormalizeStage(string stage) =>
        stage.Trim().ToLowerInvariant().Replace(' ', '_').Replace('-', '_');

    static int GetLatchedStagePoll(string stageSpec, int absolutePoll)
    {
        string stage = stageSpec;
        int requiredVisit = -1;
        int visitSeparator = stageSpec.LastIndexOf('@');
        if (visitSeparator >= 0)
        {
            string visitText = stageSpec[(visitSeparator + 1)..];
            if (!int.TryParse(visitText, out requiredVisit) || requiredVisit <= 0)
                throw new InvalidOperationException(
                    $"Invalid latched scripted-input stage visit: {stageSpec}");
            stage = stageSpec[..visitSeparator];
        }
        if (requiredVisit > 0 &&
            (!_scriptStageVisits.TryGetValue(stage, out int visits) ||
             visits != requiredVisit))
            return -1;
        return _scriptStageStartPolls.TryGetValue(stage, out int originPoll)
            ? absolutePoll - originPoll
            : -1;
    }

    internal static void SignalScriptStage(string stage, int captureDelayPolls = 0)
    {
        if (_scriptedInput.Count == 0) return;
        stage = NormalizeStage(stage);
        if (_scriptStage == stage) return;

        _scriptStageStartPolls[stage] = _inputPoll;
        int visits = _scriptStageVisits.TryGetValue(stage, out int count)
            ? count + 1
            : 1;
        _scriptStageVisits[stage] = visits;
        _scriptStage = stage;
        _stagePoll = 0;
        bool captureStage =
            !_disableScriptStageCaptures &&
            (_scriptStageCaptureFilter == null ||
             _scriptStageCaptureFilter.Contains(stage));
        _stageCapturePoll = captureStage ? captureDelayPolls : -1;
        _stageCaptureLabel = captureStage ? stage : null;
        if (captureDelayPolls == 0 && captureStage)
        {
            HostWindow.RequestDisplayCapture(stage);
            _stageCapturePoll = -1;
            _stageCaptureLabel = null;
        }
        Console.Error.WriteLine($"[Input] stage '{stage}' at absolute poll {_inputPoll}");
        if (_scriptExitAfterStageVisits > 0 &&
            stage.Equals(
                _scriptExitAfterStage,
                StringComparison.OrdinalIgnoreCase) &&
            visits >= _scriptExitAfterStageVisits)
        {
            Console.Error.WriteLine(
                $"[Input] deterministic replay completed after " +
                $"stage '{stage}' visit {visits}");
            Runtime.Shutdown();
            Environment.Exit(0);
        }
    }

    static void ApplyScriptedInput()
    {
        Array.Clear(_fakeDisconnected);
        int poll = _inputPoll++;
        int stagePoll = _stagePoll++;
        _fakeKeys.Clear();
        foreach (var pulse in _scriptedKeys)
        {
            int current = pulse.Stage == null ? poll : pulse.Stage.StartsWith("after:", StringComparison.OrdinalIgnoreCase)
                ? GetLatchedStagePoll(pulse.Stage[6..], poll) : pulse.Stage == _scriptStage ? stagePoll : -1;
            if (current >= pulse.Start && current < pulse.End) _fakeKeys.Add(pulse.Key);
        }
        if (!_disableScriptStageCaptures &&
            _stageCapturePoll == stagePoll && _stageCaptureLabel != null)
        {
            HostWindow.RequestDisplayCapture(_stageCaptureLabel);
            Console.Error.WriteLine(
                $"[Input] requested stage '{_stageCaptureLabel}' capture at poll {stagePoll}");
            _stageCapturePoll = -1;
            _stageCaptureLabel = null;
        }
        foreach (var pulse in _scriptedInput)
        {
            int currentPoll;
            if (pulse.Stage == null)
                currentPoll = poll;
            else if (pulse.Stage.StartsWith(
                         "after:", StringComparison.OrdinalIgnoreCase))
            {
                currentPoll = GetLatchedStagePoll(pulse.Stage[6..], poll);
            }
            else if (pulse.Stage == _scriptStage)
                currentPoll = stagePoll;
            else
                continue;

            if (currentPoll < pulse.Start || currentPoll >= pulse.End) continue;
            if (currentPoll == pulse.Start)
            {
                string location = pulse.Stage == null
                    ? $"absolute poll {poll}"
                    : pulse.Stage.StartsWith(
                        "after:", StringComparison.OrdinalIgnoreCase)
                        ? $"after stage '{pulse.Stage[6..]}' poll " +
                          $"{currentPoll} (absolute {poll})"
                    : $"stage '{pulse.Stage}' poll {stagePoll} (absolute {poll})";
                Console.Error.WriteLine(
                    $"[Input] scripted pulse at {location}: " +
                    $"p1=0x{pulse.Pad1Mask:X4} p2=0x{pulse.Pad2Mask:X4}");
                // "*" is a proof-run mode: capture every deterministic input
                // checkpoint, including absolute boot/FMV/menu pulses as well
                // as stage-relative gameplay checkpoints.
                if (_captureScriptedStage != null &&
                    (_captureScriptedStage == "*" ||
                     _captureScriptedStage == pulse.Stage))
                    HostWindow.RequestDisplayCapture($"{pulse.Stage}_{stagePoll:0000}");
            }
            Controller.State &= (ushort)~pulse.Pad1Mask;
            Controller.State2 &= (ushort)~pulse.Pad2Mask;
            Controller.SetState(2, (ushort)(Controller.GetState(2) & ~pulse.Pad3Mask));
            Controller.SetState(3, (ushort)(Controller.GetState(3) & ~pulse.Pad4Mask));
        }
        if (_fakePadActive)
        {
            foreach (var pressed in _fakePressed) pressed.Clear();
            foreach (var pulse in _scriptedPhysical)
            {
                int currentPoll;
                if (pulse.Stage == null)
                    currentPoll = poll;
                else if (pulse.Stage.StartsWith(
                             "after:", StringComparison.OrdinalIgnoreCase))
                {
                    currentPoll = GetLatchedStagePoll(pulse.Stage[6..], poll);
                }
                else
                    currentPoll = pulse.Stage == _scriptStage ? stagePoll : -1;
                if (currentPoll < pulse.Start || currentPoll >= pulse.End) continue;
                int pad = pulse.Pad;
                foreach (int code in pulse.Codes)
                    if (code == -1) _fakeDisconnected[pad] = true;
                    else _fakePressed[pad].Add(code);
            }
            ApplyFakePads();
        }
        if (_scriptExitAfterPoll >= 0 && poll >= _scriptExitAfterPoll)
        {
            Console.Error.WriteLine(
                $"[Input] deterministic replay completed at poll {poll}");
            Runtime.Shutdown();
            Environment.Exit(0);
        }
    }

    public static int? GetFirstPressedPadButton(int pad = 0)
    {
        if (_fakePadActive)
            return _fakePressed[pad].Count == 0 ? null : _fakePressed[pad].Min();
        var ctrl = PadHandle(pad);
        if (_sdl == null || ctrl == null) return null;
        for (int b = 0; b < (int)GameControllerButton.Max; b++)
            if (_sdl.GameControllerGetButton(ctrl, (GameControllerButton)b) != 0)
                return b;
        if (Pressed(ctrl, LeftTrigger, pad, latch: false)) return LeftTrigger;
        if (Pressed(ctrl, RightTrigger, pad, latch: false)) return RightTrigger;
        for (int b = LeftStickLeft; b <= RightStickDown; b++)
            if (Pressed(ctrl, b, pad, latch: false)) return b;
        return null;
    }

    static bool IsStickBinding(int b) => b is >= LeftStickLeft and <= RightStickDown;

    static (GameControllerAxis Axis, bool Positive) AxisBinding(int b) => b switch
    {
        LeftStickLeft   => (GameControllerAxis.Leftx,  false),
        LeftStickRight  => (GameControllerAxis.Leftx,  true),
        LeftStickUp     => (GameControllerAxis.Lefty,  false),
        LeftStickDown   => (GameControllerAxis.Lefty,  true),
        RightStickLeft  => (GameControllerAxis.Rightx, false),
        RightStickRight => (GameControllerAxis.Rightx, true),
        RightStickUp    => (GameControllerAxis.Righty, false),
        _               => (GameControllerAxis.Righty, true),
    };

    public static void Shutdown()
    {
        CloseControllers();
        _sdl?.QuitSubSystem(Sdl.InitGamecontroller);
        _sdl?.Dispose();
        _sdl = null;
    }

    static void PollGamepadEvents()
    {
        if (_sdl == null) return;
        Event ev;
        bool changed = false;
        while (_sdl.PollEvent(&ev) != 0)
        {
            if (ev.Type == (uint)EventType.Controllerdeviceadded) changed = true;
            if (ev.Type == (uint)EventType.Controllerdeviceremoved) changed = true;
        }
        if (changed) Rescan();
    }

    static void CloseController(int pad)
    {
        if (PadHandle(pad) != null) _sdl?.GameControllerClose(PadHandle(pad));
        _pads[pad] = 0;
        _padIds[pad] = -1;
        for (int binding = 0; binding < 128; binding++) _analogLatched[pad, binding] = false;
        Controller.SetState(pad, 0xFFFF);
        Controller.SetAxes(pad, 128, 128, 128, 128);
        Controller.LocalConnected[pad] = false;
        _lastRumble[pad] = (byte.MaxValue, byte.MaxValue);
    }

    static void CloseControllers()
    {
        for (int pad = 0; pad < 4; pad++) CloseController(pad);
    }

    static void Rescan()
    {
        if (_sdl == null) return;
        // Keep surviving devices in their slots when another pad is removed.
        // Reopening every handle would silently transfer player ownership.
        for (int pad = 0; pad < 4; pad++)
            if (PadHandle(pad) != null && _sdl.GameControllerGetAttached(PadHandle(pad)) != SdlBool.True)
                CloseController(pad);
        int n = _sdl.NumJoysticks();
        for (int i = 0; i < n; i++)
        {
            if (_sdl.IsGameController(i) != SdlBool.True) continue;
            int id = _sdl.JoystickGetDeviceInstanceID(i);
            if (_padIds.Contains(id)) continue;
            int slot = Array.IndexOf(_pads, (nint)0);
            if (slot < 0) break;
            var ctrl = _sdl.GameControllerOpen(i);
            if (ctrl == null) continue;
            _pads[slot] = (nint)ctrl;
            _padIds[slot] = id;
        }
        Console.WriteLine($"[Input] SDL controllers: joysticks={n} " +
            string.Join(" ", Enumerable.Range(0, 4).Select(p => $"p{p + 1}={ControllerName(PadHandle(p))}")));
    }

    static string ControllerName(GameController* controller)
    {
        if (_sdl == null || controller == null) return "none";
        string name = Marshal.PtrToStringUTF8((nint)_sdl.GameControllerName(controller)) ?? "unknown";
        return $"'{name}'";
    }

    static void PollKeyboard()
    {
        LocalInputSession.KeyboardConnected = (!_disableLiveInput && _keyboard != null) || _scriptedKeys.Count > 0;
        LocalInputSession.KeyboardState = KeyState(InputBindingResolver.ResolveKeys(ConfigManager.Game.Keys,
            0, GpuHle.GameplayActive || _readyPromptActive, NativeMenuActive));
        if (LocalInputSession.PlayerCount != 0) return;
        // Preserve the existing solo/legacy two-player bindings outside a joined session.
        Controller.State &= LocalInputSession.KeyboardState;
        Controller.State2 &= KeyState(InputBindingResolver.ResolveKeys(ConfigManager.Game.Keys2,
            1, GpuHle.GameplayActive || _readyPromptActive, NativeMenuActive));
        Controller.LocalConnected[0] |= LocalInputSession.KeyboardConnected;
        Controller.LocalConnected[1] |= LocalInputSession.KeyboardConnected && HasAnyKey(ConfigManager.Game.Keys2);
    }

    static ushort KeyState(KeyBindings cfg)
    {
        ushort s = 0xFFFF;
        void B(string keyName, ushort bit)
        {
            if (Enum.TryParse<Key>(keyName, out var k) && IsKeyDown(k))
                s &= (ushort)~bit;
        }

        B(cfg.Cross,    Controller.Cross);
        B(cfg.Circle,   Controller.Circle);
        B(cfg.Square,   Controller.Square);
        B(cfg.Triangle, Controller.Triangle);
        B(cfg.L1,       Controller.L1);
        B(cfg.R1,       Controller.R1);
        B(cfg.L2,       Controller.L2);
        B(cfg.R2,       Controller.R2);
        B(cfg.L3,       Controller.L3);
        B(cfg.R3,       Controller.R3);
        B(cfg.Start,    Controller.Start);
        B(cfg.Select,   Controller.Select);
        B(cfg.Up,       Controller.Up);
        B(cfg.Down,     Controller.Down);
        B(cfg.Left,     Controller.Left);
        B(cfg.Right,    Controller.Right);

        return s;
    }

    static bool HasAnyKey(KeyBindings cfg) =>
        cfg.Cross.Length > 0 || cfg.Circle.Length > 0 || cfg.Square.Length > 0 || cfg.Triangle.Length > 0 ||
        cfg.L1.Length > 0 || cfg.R1.Length > 0 || cfg.L2.Length > 0 || cfg.R2.Length > 0 ||
        cfg.L3.Length > 0 || cfg.R3.Length > 0 || cfg.Start.Length > 0 || cfg.Select.Length > 0 ||
        cfg.Up.Length > 0 || cfg.Down.Length > 0 || cfg.Left.Length > 0 || cfg.Right.Length > 0;

    // Runs the synthetic controller through the same profile resolution and
    // press tests as a real pad, so a scripted physical input exercises
    // ResolvePad, the binding arrays and the hysteresis latch.
    static void ApplyFakePads()
    {
        for (int pad = 0; pad < 4; pad++)
        {
            GamepadBindings bindings = InputBindingResolver.ResolvePad(
                InputProfiles.ForPlayer(ConfigManager.Game, pad),
                InputProfiles.PadForPlayer(ConfigManager.Game, pad),
                GpuHle.GameplayActive || _readyPromptActive,
                NativeMenuActive);
            ushort s = Controller.GetState(pad);
            s = FakeApply(bindings.Cross,    Controller.Cross,    s, pad);
            s = FakeApply(bindings.Circle,   Controller.Circle,   s, pad);
            s = FakeApply(bindings.Square,   Controller.Square,   s, pad);
            s = FakeApply(bindings.Triangle, Controller.Triangle, s, pad);
            s = FakeApply(bindings.L1,       Controller.L1,       s, pad);
            s = FakeApply(bindings.R1,       Controller.R1,       s, pad);
            s = FakeApply(bindings.L2,       Controller.L2,       s, pad);
            s = FakeApply(bindings.R2,       Controller.R2,       s, pad);
            s = FakeApply(bindings.L3,       Controller.L3,       s, pad);
            s = FakeApply(bindings.R3,       Controller.R3,       s, pad);
            s = FakeApply(bindings.Start,    Controller.Start,    s, pad);
            s = FakeApply(bindings.Select,   Controller.Select,   s, pad);
            s = FakeApply(bindings.Up,       Controller.Up,       s, pad);
            s = FakeApply(bindings.Down,     Controller.Down,     s, pad);
            s = FakeApply(bindings.Left,     Controller.Left,     s, pad);
            s = FakeApply(bindings.Right,    Controller.Right,    s, pad);
            Controller.SetState(pad, s);

            // Keep the analog bytes consistent with a stick that is actually
            // deflected, so the pad image the game sees is coherent.
            byte lx = FakeAxisByte(pad, LeftStickLeft, LeftStickRight);
            byte ly = FakeAxisByte(pad, LeftStickUp, LeftStickDown);
            byte rx = FakeAxisByte(pad, RightStickLeft, RightStickRight);
            byte ry = FakeAxisByte(pad, RightStickUp, RightStickDown);
            Controller.SetAxes(pad, lx, ly, rx, ry);
        }
    }

    static byte FakeAxisByte(int pad, int negative, int positive) =>
        _fakePressed[pad].Contains(positive) ? (byte)0xFF
        : _fakePressed[pad].Contains(negative) ? (byte)0x00
        : (byte)0x80;

    static ushort FakeApply(int[] bindings, ushort bit, ushort s, int pad)
    {
        bool pressed = false;
        foreach (int binding in bindings)
            if (FakePressed(binding, pad)) pressed = true;
        return pressed ? (ushort)(s & ~bit) : s;
    }

    static bool FakePressed(int binding, int pad)
    {
        bool held = _fakePressed[pad].Contains(binding);
        int value = held ? short.MaxValue : 0;
        if (binding == LeftTrigger || binding == RightTrigger)
            return AnalogLatch(pad, binding, value, AxisThreshold, latch: true);
        if (IsStickBinding(binding))
            return AnalogLatch(pad, binding, value, StickThreshold, latch: true);
        return held;
    }

    static void PollGamepads()
    {
        if (_sdl == null) return;
        for (int player = 0; player < 4; player++)
        {
            var ctrl = PadHandle(player);
            if (ctrl == null) continue;
            if (GetFirstPressedPadButton(player).HasValue) _promptKeyboard[player] = false;
            var bindings = InputBindingResolver.ResolvePad(
                InputProfiles.ForPlayer(ConfigManager.Game, player),
                InputProfiles.PadForPlayer(ConfigManager.Game, player),
                GpuHle.GameplayActive || _readyPromptActive, NativeMenuActive);
            Controller.SetState(player, PadState(ctrl, bindings, Controller.GetState(player), player));
            Controller.SetAxes(player,
                AxisToByte(_sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Leftx)),
                AxisToByte(_sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Lefty)),
                AxisToByte(_sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Rightx)),
                AxisToByte(_sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Righty)));
        }
    }

    static ushort PadState(
        GameController* ctrl, GamepadBindings pad, ushort s, int padIndex)
    {
        s = Apply(ctrl, pad.Cross,    Controller.Cross,    s, padIndex);
        s = Apply(ctrl, pad.Circle,   Controller.Circle,   s, padIndex);
        s = Apply(ctrl, pad.Square,   Controller.Square,   s, padIndex);
        s = Apply(ctrl, pad.Triangle, Controller.Triangle, s, padIndex);
        s = Apply(ctrl, pad.L1,       Controller.L1,       s, padIndex);
        s = Apply(ctrl, pad.R1,       Controller.R1,       s, padIndex);
        s = Apply(ctrl, pad.L2,       Controller.L2,       s, padIndex);
        s = Apply(ctrl, pad.R2,       Controller.R2,       s, padIndex);
        s = Apply(ctrl, pad.L3,       Controller.L3,       s, padIndex);
        s = Apply(ctrl, pad.R3,       Controller.R3,       s, padIndex);
        s = Apply(ctrl, pad.Start,    Controller.Start,    s, padIndex);
        s = Apply(ctrl, pad.Select,   Controller.Select,   s, padIndex);
        s = Apply(ctrl, pad.Up,       Controller.Up,       s, padIndex);
        s = Apply(ctrl, pad.Down,     Controller.Down,     s, padIndex);
        s = Apply(ctrl, pad.Left,     Controller.Left,     s, padIndex);
        s = Apply(ctrl, pad.Right,    Controller.Right,    s, padIndex);
        return s;
    }

    static ushort Apply(
        GameController* ctrl, int[] bindings, ushort bit, ushort s, int pad)
    {
        // Every binding is evaluated, not just up to the first hit, so the
        // latch state of the others stays current.
        bool pressed = false;
        foreach (var binding in bindings)
            if (Pressed(ctrl, binding, pad)) pressed = true;
        return pressed ? (ushort)(s & ~bit) : s;
    }

    // Analog bindings latch with hysteresis. A bare threshold compare makes a
    // trigger or stick held near the boundary chatter the command on and off
    // frame to frame, and the commands that matter most here are the ones the
    // engine requires to be held unbroken: brake/reverse only engages after
    // the car has been held stationary, so a single dropped frame restarts it.
    // Press at the full threshold, release at three quarters of it.
    const int ReleaseNumerator = 3;
    const int ReleaseDenominator = 4;
    static readonly bool[,] _analogLatched = new bool[4, 128];

    static bool AnalogLatch(int pad, int binding, int value, int threshold, bool latch)
    {
        if (!latch || binding < 0 || binding >= 128) return value > threshold;
        int release = threshold * ReleaseNumerator / ReleaseDenominator;
        bool pressed = value > (_analogLatched[pad, binding] ? release : threshold);
        _analogLatched[pad, binding] = pressed;
        return pressed;
    }

    // latch:false is for the rebinding capture UI, which only asks "is this
    // input active right now" and must not disturb the gameplay latch state.
    static bool Pressed(
        GameController* ctrl, int binding, int pad = 0, bool latch = true)
    {
        if (_sdl == null) return false;
        if (binding == LeftTrigger)
            return AnalogLatch(pad, binding,
                _sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Triggerleft),
                AxisThreshold, latch);
        if (binding == RightTrigger)
            return AnalogLatch(pad, binding,
                _sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Triggerright),
                AxisThreshold, latch);
        if (IsStickBinding(binding))
        {
            var (axis, positive) = AxisBinding(binding);
            short v = _sdl.GameControllerGetAxis(ctrl, axis);
            return AnalogLatch(pad, binding,
                positive ? v : -v, StickThreshold, latch);
        }
        return _sdl.GameControllerGetButton(ctrl, (GameControllerButton)binding) != 0;
    }

    static byte AxisToByte(short axis)
    {
        float f = Math.Clamp(axis * 1.3f / 32768.0f, -1.0f, 1.0f);
        return (byte)Math.Clamp((int)MathF.Round((f + 1.0f) * 127.5f), 0, 255);
    }

    public static void SetRumble(int pad, byte large, byte small)
    {
        pad = LocalInputSession.DeviceForPlayer(pad);
        GameController* controller = PadHandle(pad);
        if ((uint)pad < (uint)_lastRumble.Length &&
            _lastRumble[pad] != (large, small))
        {
            _lastRumble[pad] = (large, small);
            if (_traceInput)
            {
                Console.WriteLine(
                    $"[Input] rumble pad={pad + 1} large={large} small={small} " +
                    $"controller={ControllerName(controller)} suppressed={_suppressRumble}");
            }
        }
        if (_sdl == null || controller == null) return;
        if (_suppressRumble) return;
        ushort lo = (ushort)(large * 257);
        ushort hi = small != 0 ? (ushort)65535 : (ushort)0;
        uint duration = large == 0 && small == 0 ? 0u : 500u;
        _sdl.GameControllerRumble(controller, lo, hi, duration);
    }

    static void OnKeyDown(IKeyboard kb, Key key, int _)
    {
        _promptKeyboard[0] = true;
        if (key == Key.F1)  _topBarToggle = true;
        if (key == Key.F11) _fullscreenToggle = true;
    }

}
