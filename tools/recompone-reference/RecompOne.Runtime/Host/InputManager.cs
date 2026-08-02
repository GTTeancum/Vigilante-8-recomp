using Silk.NET.Input;
using Silk.NET.SDL;
using System.Runtime.InteropServices;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Sdk;

namespace RecompOne.Runtime.Host;

internal static unsafe class InputManager
{
    static IKeyboard?_keyboard;
    static Sdl?_sdl;
    static GameController* _pad0;
    static GameController* _pad1;

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
        string? Stage, int Start, int End, ushort Pad1Mask, ushort Pad2Mask);

    static readonly List<ScriptedPulse> _scriptedInput = new();
    static int _inputPoll;
    static string? _scriptStage;
    static int _stagePoll;
    static int _stageCapturePoll = -1;
    static string? _stageCaptureLabel;
    static bool _disableLiveInput;
    static bool _forcePad2Connected;
    static bool _traceInput;
    static string? _captureScriptedStage;
    static bool _suppressRumble;
    static int _scriptExitAfterPoll = -1;
    static readonly (byte Large, byte Small)[] _lastRumble =
        [(byte.MaxValue, byte.MaxValue), (byte.MaxValue, byte.MaxValue)];

    
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
        ParseScriptedInput();
        if (_disableLiveInput)
            Console.Error.WriteLine("[Input] live keyboard/gamepad input disabled for deterministic replay");
        if (_forcePad2Connected)
            Console.Error.WriteLine("[Input] controller 2 connection forced for deterministic replay");
    }

    public static bool IsConnected => _pad0 != null;

    public static bool IsPadConnected(int pad) => pad == 0 ? _pad0 != null : _pad1 != null;

    public static bool IsKeyDown(Key k) => _keyboard?.IsKeyPressed(k) ?? false;

    public static void Poll()
    {
        if (_disableLiveInput)
        {
            Controller.State = Controller.State2 = 0xFFFF;
            Controller.LeftX = Controller.LeftY = Controller.RightX = Controller.RightY = 0x80;
            Controller.LeftX2 = Controller.LeftY2 = Controller.RightX2 = Controller.RightY2 = 0x80;
        }
        else
        {
            PollGamepadEvents();
            PollKeyboard();
            PollGamepads();
        }
        ApplyScriptedInput();
        Controller.State &= (ushort)~V8Compat.GetAutomationInputMask();
        Controller.State &= (ushort)~V82Compat.GetAutomationInputMask();
        Controller.Connected2 = _forcePad2Connected ||
            (!_disableLiveInput && (_pad1 != null || HasAnyKey(ConfigManager.Game.Keys2)));
    }

    static void ParseScriptedInput()
    {
        _scriptedInput.Clear();
        _inputPoll = 0;
        _scriptStage = null;
        _stagePoll = 0;
        _stageCapturePoll = -1;
        _stageCaptureLabel = null;

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
                stage = NormalizeStage(raw[1..^1]);
                if (stage.Length == 0)
                    throw new InvalidOperationException("Scripted input stage name cannot be empty");
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
            foreach (string token in sides[1].Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
            {
                string button = token;
                int pad = 1;
                int colon = token.IndexOf(':');
                if (colon >= 0)
                {
                    string prefix = token[..colon].Trim();
                    button = token[(colon + 1)..].Trim();
                    pad = prefix.ToUpperInvariant() switch
                    {
                        "P1" or "PAD1" => 1,
                        "P2" or "PAD2" => 2,
                        _ => throw new InvalidOperationException(
                            $"Unknown scripted controller prefix: {prefix}"),
                    };
                }

                ushort mask = button.ToUpperInvariant() switch
                {
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
                if (pad == 1) pad1Mask |= mask;
                else pad2Mask |= mask;
            }
            _scriptedInput.Add(new ScriptedPulse(
                stage, start, start + duration, pad1Mask, pad2Mask));
        }

        int staged = _scriptedInput.Count(pulse => pulse.Stage != null);
        Console.Error.WriteLine(
            $"[Input] loaded {_scriptedInput.Count} scripted input pulses ({staged} stage-relative)");
    }

    static string NormalizeStage(string stage) =>
        stage.Trim().ToLowerInvariant().Replace(' ', '_').Replace('-', '_');

    internal static void SignalScriptStage(string stage, int captureDelayPolls = 0)
    {
        if (_scriptedInput.Count == 0) return;
        stage = NormalizeStage(stage);
        if (_scriptStage == stage) return;

        _scriptStage = stage;
        _stagePoll = 0;
        _stageCapturePoll = captureDelayPolls;
        _stageCaptureLabel = stage;
        if (captureDelayPolls == 0)
        {
            HostWindow.RequestDisplayCapture(stage);
            _stageCapturePoll = -1;
            _stageCaptureLabel = null;
        }
        Console.Error.WriteLine($"[Input] stage '{stage}' at absolute poll {_inputPoll}");
    }

    static void ApplyScriptedInput()
    {
        int poll = _inputPoll++;
        int stagePoll = _stagePoll++;
        if (_stageCapturePoll == stagePoll && _stageCaptureLabel != null)
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
            else if (pulse.Stage == _scriptStage)
                currentPoll = stagePoll;
            else
                continue;

            if (currentPoll < pulse.Start || currentPoll >= pulse.End) continue;
            if (currentPoll == pulse.Start)
            {
                string location = pulse.Stage == null
                    ? $"absolute poll {poll}"
                    : $"stage '{pulse.Stage}' poll {stagePoll} (absolute {poll})";
                Console.Error.WriteLine(
                    $"[Input] scripted pulse at {location}: " +
                    $"p1=0x{pulse.Pad1Mask:X4} p2=0x{pulse.Pad2Mask:X4}");
                // "*" is a proof-run mode: capture every deterministic input
                // checkpoint, including absolute boot/FMV/menu pulses as well
                // as stage-relative gameplay checkpoints.
                if (_captureScriptedStage == "*" ||
                    _captureScriptedStage == pulse.Stage)
                    HostWindow.RequestDisplayCapture($"{pulse.Stage}_{stagePoll:0000}");
            }
            Controller.State &= (ushort)~pulse.Pad1Mask;
            Controller.State2 &= (ushort)~pulse.Pad2Mask;
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
        var ctrl = pad == 0 ? _pad0 : _pad1;
        if (_sdl == null || ctrl == null) return null;
        for (int b = 0; b < (int)GameControllerButton.Max; b++)
            if (_sdl.GameControllerGetButton(ctrl, (GameControllerButton)b) != 0)
                return b;
        if (Pressed(ctrl, LeftTrigger)) return LeftTrigger;
        if (Pressed(ctrl, RightTrigger)) return RightTrigger;
        for (int b = LeftStickLeft; b <= RightStickDown; b++)
            if (Pressed(ctrl, b)) return b;
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

    static void CloseControllers()
    {
        if (_pad0 != null) { _sdl?.GameControllerClose(_pad0); _pad0 = null; }
        if (_pad1 != null) { _sdl?.GameControllerClose(_pad1); _pad1 = null; }
    }

    static void Rescan()
    {
        if (_sdl == null) return;
        CloseControllers();
        int n = _sdl.NumJoysticks();
        for (int i = 0; i < n; i++)
        {
            if (_sdl.IsGameController(i) != SdlBool.True) continue;
            var ctrl = _sdl.GameControllerOpen(i);
            if (ctrl == null) continue;
            if (_pad0 == null) _pad0 = ctrl;
            else { _pad1 = ctrl; break; }
        }
        Console.WriteLine(
            $"[Input] SDL controllers: joysticks={n} " +
            $"p1={ControllerName(_pad0)} p2={ControllerName(_pad1)}");
    }

    static string ControllerName(GameController* controller)
    {
        if (_sdl == null || controller == null) return "none";
        string name = Marshal.PtrToStringUTF8((nint)_sdl.GameControllerName(controller)) ?? "unknown";
        return $"'{name}'";
    }

    static void PollKeyboard()
    {
        var kb = _keyboard;
        if (kb == null)
        {
            Controller.State = 0xFFFF;
            Controller.State2 = 0xFFFF;
            return;
        }
        Controller.State = KeyState(kb, ConfigManager.Game.Keys);
        Controller.State2 = KeyState(kb, ConfigManager.Game.Keys2);
    }

    static ushort KeyState(IKeyboard kb, KeyBindings cfg)
    {
        ushort s = 0xFFFF;
        void B(string keyName, ushort bit)
        {
            if (Enum.TryParse<Key>(keyName, out var k) && kb.IsKeyPressed(k))
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

    static void PollGamepads()
    {
        if (_sdl == null) return;

        if (_pad0 != null)
        {
            Controller.State = PadState(_pad0, ConfigManager.Game.Pad, Controller.State);
            Controller.LeftX = AxisToByte(_sdl.GameControllerGetAxis(_pad0, GameControllerAxis.Leftx));
            Controller.LeftY = AxisToByte(_sdl.GameControllerGetAxis(_pad0, GameControllerAxis.Lefty));
            Controller.RightX = AxisToByte(_sdl.GameControllerGetAxis(_pad0, GameControllerAxis.Rightx));
            Controller.RightY = AxisToByte(_sdl.GameControllerGetAxis(_pad0, GameControllerAxis.Righty));
        }

        if (_pad1 != null)
        {
            Controller.State2 = PadState(_pad1, ConfigManager.Game.Pad2, Controller.State2);
            Controller.LeftX2 = AxisToByte(_sdl.GameControllerGetAxis(_pad1, GameControllerAxis.Leftx));
            Controller.LeftY2 = AxisToByte(_sdl.GameControllerGetAxis(_pad1, GameControllerAxis.Lefty));
            Controller.RightX2 = AxisToByte(_sdl.GameControllerGetAxis(_pad1, GameControllerAxis.Rightx));
            Controller.RightY2 = AxisToByte(_sdl.GameControllerGetAxis(_pad1, GameControllerAxis.Righty));
        }
        else
        {
            Controller.LeftX2 = Controller.LeftY2 = Controller.RightX2 = Controller.RightY2 = 0x80;
        }
    }

    static ushort PadState(GameController* ctrl, GamepadBindings pad, ushort s)
    {
        s = Apply(ctrl, pad.Cross,    Controller.Cross,    s);
        s = Apply(ctrl, pad.Circle,   Controller.Circle,   s);
        s = Apply(ctrl, pad.Square,   Controller.Square,   s);
        s = Apply(ctrl, pad.Triangle, Controller.Triangle, s);
        s = Apply(ctrl, pad.L1,       Controller.L1,       s);
        s = Apply(ctrl, pad.R1,       Controller.R1,       s);
        s = Apply(ctrl, pad.L2,       Controller.L2,       s);
        s = Apply(ctrl, pad.R2,       Controller.R2,       s);
        s = Apply(ctrl, pad.L3,       Controller.L3,       s);
        s = Apply(ctrl, pad.R3,       Controller.R3,       s);
        s = Apply(ctrl, pad.Start,    Controller.Start,    s);
        s = Apply(ctrl, pad.Select,   Controller.Select,   s);
        s = Apply(ctrl, pad.Up,       Controller.Up,       s);
        s = Apply(ctrl, pad.Down,     Controller.Down,     s);
        s = Apply(ctrl, pad.Left,     Controller.Left,     s);
        s = Apply(ctrl, pad.Right,    Controller.Right,    s);
        return s;
    }

    static ushort Apply(GameController* ctrl, int[] bindings, ushort bit, ushort s)
    {
        foreach (var binding in bindings)
            if (Pressed(ctrl, binding))
                return (ushort)(s & ~bit);
        return s;
    }

    static bool Pressed(GameController* ctrl, int binding)
    {
        if (_sdl == null) return false;
        if (binding == LeftTrigger)
            return _sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Triggerleft) > AxisThreshold;
        if (binding == RightTrigger)
            return _sdl.GameControllerGetAxis(ctrl, GameControllerAxis.Triggerright) > AxisThreshold;
        if (IsStickBinding(binding))
        {
            var (axis, positive) = AxisBinding(binding);
            short v = _sdl.GameControllerGetAxis(ctrl, axis);
            return positive ? v > StickThreshold : v < -StickThreshold;
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
        GameController* controller = pad == 0 ? _pad0 : _pad1;
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
        if (key == Key.F1)  _topBarToggle = true;
        if (key == Key.F11) _fullscreenToggle = true;
    }

}
