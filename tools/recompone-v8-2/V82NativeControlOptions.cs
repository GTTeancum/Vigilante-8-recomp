using System.Text;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

/// <summary>
/// PC control configuration embedded in V8:2's retail Controller page.
/// The surrounding page, title, player selector, cursor colors, text renderer,
/// footer compositor, transitions, and input loop all remain SHELL.DLL-owned.
/// </summary>
public static partial class Vigilante82PC
{
    static class V82NativeControlOptions
    {
        const uint PadWord = 0x8006B4ECu;
        const uint ShellUiRoot = 0x80116730u;
        const uint RetailTextLayout = 0x80115FB8u;
        const uint RetailFooterStrings = 0x801009C0u;
        const uint ScratchBase = 0x8011AF00u;
        const uint ScratchLayout = ScratchBase;
        const uint ScratchText = ScratchBase + 0x20u;
        const uint ScratchFooter = ScratchBase + 0xA0u;

        const uint Up = Controller.Up;
        const uint Right = Controller.Right;
        const uint Down = Controller.Down;
        const uint Left = Controller.Left;
        const uint Triangle = Controller.Triangle;
        const uint Circle = Controller.Circle;
        const uint Cross = Controller.Cross;
        const uint Square = Controller.Square;
        const uint HandledInput =
            Up | Right | Down | Left | Triangle | Circle | Cross | Square;
        const uint ProcessedHandledInput = HandledInput << 16;

        static readonly string[] ActionNames =
        [
            "Gas / Accept",
            "Hand Brake / Cancel",
            "Brake",
            "Select Target / Back",
            "Previous Weapon",
            "Next Weapon",
            "Fire Selected Weapon",
            "Machine Gun",
            "L3",
            "R3",
            "Start",
            "Select",
            "Accelerate / Up",
            "Brake / Reverse",
            "Steer Left",
            "Steer Right",
        ];

        static bool _active;
        static bool _entryArmed;
        static bool _gamepad = true;
        static bool _capturing;
        static bool _captureAdd;
        static bool _captureArmed;
        static int _player;
        static int _row;
        static int _action;
        static uint _previousInput;

        public static bool TryDraw(CpuContext c, IMemory m)
        {
            if (!_active)
                return false;

            ConsumeNativePadWord(m);
            c.S5 = (uint)_player;
            DrawPage(c, m);
            return true;
        }

        // Spike for a real VIDEO row: the Options page is data driven off a
        // descriptor in S7, taking the per-row item at [S7 + 0x10] and
        // skipping the row when it is null. Dump the descriptor and the words
        // around it so the table can be walked back to its producer.
        static readonly bool TraceOptionsTable =
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TRACE_OPTIONS_TABLE") == "1";
        static bool _optionsTableDumped;

        static void DumpOptionsTable(CpuContext c, IMemory m)
        {
            if (!TraceOptionsTable || _optionsTableDumped) return;
            uint s7 = c.S7;
            if (s7 < 0x80000000u || s7 >= 0x80800000u) return;
            _optionsTableDumped = true;
            Console.Error.WriteLine($"[OptTable] S7=0x{s7:X8}");
            for (int i = -4; i < 20; i++)
            {
                uint a = (uint)(s7 + i * 4);
                uint v;
                try { v = m.ReadU32(a); } catch { continue; }
                string note = i == 4 ? "   <== [S7+0x10] row item" : "";
                Console.Error.WriteLine(
                    $"[OptTable] [S7{i * 4:+#;-#;+0}] 0x{a:X8} = 0x{v:X8}{note}");
            }
        }

        public static void UpdateState(CpuContext c, IMemory m)
        {
            DumpOptionsTable(c, m);
            // Read the runtime's actual active-low virtual PS1 pad. The game's
            // processed word is deliberately one native frame behind here and
            // is unsuitable for either live remapping or deterministic input.
            uint input = (uint)(~Controller.State & 0xFFFF);
            uint pressed = (input & ~_previousInput) & HandledInput;
            _previousInput = input & HandledInput;
            if (!_active)
            {
                // The same Cross press used to accept "Controllers" remains
                // visible during the page's first draw. Require its release
                // so entering the PC setup is a distinct native menu action.
                if (!_entryArmed)
                {
                    _entryArmed = (input & Cross) == 0u;
                    return;
                }
                if ((pressed & Cross) == 0u)
                    return;

                _active = true;
                _player = (int)(c.S5 & 1u);
                _row = 0;
                _capturing = false;
                UpdateFooter(c, m);
                V82Compat.SignalNativeControlsPage();
            }
            else if (_capturing)
            {
                PollBindingCapture();
            }
            else if ((pressed & Triangle) != 0u)
            {
                _active = false;
                _entryArmed = false;
                _capturing = false;
                SetFooter(c, m, 4, RetailFooterStrings);
                ConsumeNativePadWord(m);
                c.S5 = (uint)_player;
                return;
            }
            else
            {
                if ((pressed & Up) != 0u)
                    _row = (_row + 4) % 5;
                if ((pressed & Down) != 0u)
                    _row = (_row + 1) % 5;
                if ((pressed & Left) != 0u)
                    ChangeCurrent(-1);
                if ((pressed & Right) != 0u)
                    ChangeCurrent(1);

                if (_row == 4 && (pressed & (Cross | Circle)) != 0u)
                {
                    _capturing = true;
                    _captureAdd = (pressed & Circle) != 0u && _gamepad;
                    _captureArmed = false;
                }
                if (_row == 4 && (pressed & Square) != 0u)
                    ClearCurrentBinding();
            }

            // The native page reads the same word later in its loop. Consume
            // only the controls owned by this branch so it cannot also switch
            // player, leave the page, or enter calibration in the same frame.
            ConsumeNativePadWord(m);
            c.S5 = (uint)_player;
        }

        static void ConsumeNativePadWord(IMemory m)
        {
            uint processed = m.ReadU32(PadWord);
            m.WriteU32(PadWord, processed & ~ProcessedHandledInput);
        }

        static void ChangeCurrent(int direction)
        {
            switch (_row)
            {
                case 0:
                    _player = 1 - _player;
                    break;
                case 1:
                    CycleProfile(direction);
                    break;
                case 2:
                    _gamepad = !_gamepad;
                    UpdateFooterForNextFrame = true;
                    break;
                case 3:
                    _action = (_action + ActionNames.Length + direction) %
                        ActionNames.Length;
                    break;
            }
        }

        static bool UpdateFooterForNextFrame;

        static void CycleProfile(int direction)
        {
            int index = Array.IndexOf(
                InputProfiles.Names, ConfigManager.Game.InputProfile);
            if (index < 0)
                index = 0;
            index = (index + InputProfiles.Names.Length + direction) %
                InputProfiles.Names.Length;
            InputProfiles.Apply(ConfigManager.Game, InputProfiles.Names[index]);
            ConfigManager.SaveGame();
        }

        static void PollBindingCapture()
        {
            if (_gamepad)
            {
                int? pressed =
                    V82Compat.GetFirstPressedNativeControlPadButton(_player);
                if (!_captureArmed)
                {
                    _captureArmed = !pressed.HasValue;
                    return;
                }
                if (!pressed.HasValue)
                    return;

                int[] current = GetPadBinding();
                SetPadBinding(_captureAdd && !current.Contains(pressed.Value)
                    ? [.. current, pressed.Value]
                    : [pressed.Value]);
            }
            else
            {
                string? pressed = V82Compat.GetFirstPressedNativeControlKey();
                if (!_captureArmed)
                {
                    _captureArmed = pressed == null;
                    return;
                }
                if (pressed == null)
                    return;
                SetKeyBinding(pressed);
            }

            ConfigManager.Game.InputProfile = InputProfiles.Custom;
            ConfigManager.SaveGame();
            _capturing = false;
        }

        static void ClearCurrentBinding()
        {
            if (_gamepad)
                SetPadBinding([]);
            else
                SetKeyBinding("");
            ConfigManager.Game.InputProfile = InputProfiles.Custom;
            ConfigManager.SaveGame();
        }

        static void DrawPage(CpuContext c, IMemory m)
        {
            if (UpdateFooterForNextFrame)
            {
                UpdateFooter(c, m);
                UpdateFooterForNextFrame = false;
            }

            var snapshot = c.Snapshot();
            try
            {
                // Repaint only the stock controller content field with the
                // exact RGB sampled from OPTIONS.PSX. This is the retail GPU
                // fill primitive, not a host overlay; it replaces the stock
                // rotating pad and descriptor while preserving the original
                // title, player emblems, navigation, and footer layers.
                WriteRect(m, ScratchLayout, 0x00F0, 0x007C, 0x0190, 0x0128);
                c.A0 = ScratchLayout;
                c.A1 = 0x00182018u;
                c.RA = 0x8010A8ECu;
                func_8001ADF8(c, m);

                uint root = m.ReadU32(ShellUiRoot);
                uint childOffset = m.ReadU32(root + 0x10u);
                c.A0 = root + childOffset;
                c.A1 = 1u;
                c.RA = 0x8010A8ECu;
                func_80019294(c, m);
                uint textObject = c.V0;
                uint normalColor = m.ReadU32(textObject + 0x4u);

                DrawRow(c, m, textObject, normalColor, 0, 0x0080,
                    $"Controller {_player + 1}");
                DrawRow(c, m, textObject, normalColor, 1, 0x00AC,
                    $"Preset: {ConfigManager.Game.InputProfile}");
                DrawRow(c, m, textObject, normalColor, 2, 0x00D8,
                    $"Device: {(_gamepad ? "Gamepad" : "Keyboard")}");
                DrawRow(c, m, textObject, normalColor, 3, 0x0104,
                    $"Action: {ActionNames[_action]}");
                DrawRow(c, m, textObject, normalColor, 4, 0x0130,
                    $"Binding: {CurrentBindingLabel()}");

                if (_capturing)
                {
                    SetObjectColor(m, textObject, normalColor, selected: true);
                    DrawText(c, m, textObject,
                        _captureAdd ? "Press another button..." :
                        _gamepad ? "Press a button..." : "Press a key...",
                        0x0160, 0x0000000Au);
                }

                c.A0 = textObject;
                c.RA = 0x8010A8ECu;
                func_80019320(c, m);
            }
            finally
            {
                c.Restore(snapshot);
            }
        }

        static void DrawRow(
            CpuContext c, IMemory m, uint textObject, uint normalColor,
            int row, ushort y, string text)
        {
            bool selected = row == _row;
            SetObjectColor(m, textObject, normalColor, selected);
            DrawText(c, m, textObject,
                selected ? $"\u0084{text}" : $"  {text}", y, 0x00000008u);
        }

        static void SetObjectColor(
            IMemory m, uint textObject, uint normalColor, bool selected)
        {
            uint color = selected
                ? (normalColor & 0xFF000000u) | 0x00006078u
                : normalColor;
            m.WriteU32(textObject + 0x4u, color);
        }

        static void DrawText(
            CpuContext c, IMemory m, uint textObject,
            string text, ushort y, uint flags)
        {
            WriteLatin1(m, ScratchText, text, 95);
            CopyBytes(m, RetailTextLayout, ScratchLayout, 16);
            WriteRect(m, ScratchLayout, 0x0100, y, 0x0180, 0x0024);
            WriteRect(m, ScratchLayout + 8u, 0x0100, y, 0x0180, 0x0024);
            c.A0 = textObject;
            c.A1 = ScratchText;
            c.A2 = ScratchLayout;
            c.A3 = flags;
            c.RA = 0x8010A8ECu;
            func_8001A3B0(c, m);
        }

        static void UpdateFooter(CpuContext c, IMemory m)
        {
            uint cursor = ScratchFooter;
            string[] prompts = _gamepad
                ? ["\u0084select", "\u0082replace", "\u0083back",
                   "\u0080add", "\u0081clear"]
                : ["\u0084select", "\u0082assign", "\u0083back",
                   "\u0081clear"];
            foreach (string prompt in prompts)
                cursor += (uint)WriteLatin1(m, cursor, prompt, 31);
            SetFooter(c, m, prompts.Length, ScratchFooter);
        }

        static void SetFooter(
            CpuContext c, IMemory m, int promptCount, uint strings)
        {
            var snapshot = c.Snapshot();
            try
            {
                c.A0 = (uint)promptCount;
                c.A1 = strings;
                c.RA = 0x8010A8ECu;
                func_80108CB8(c, m);
            }
            finally
            {
                c.Restore(snapshot);
            }
        }

        static string CurrentBindingLabel()
        {
            if (!_gamepad)
            {
                string key = GetKeyBinding();
                return key.Length == 0 ? "Unbound" : key;
            }
            int[] binding = GetPadBinding();
            return binding.Length == 0
                ? "Unbound"
                : string.Join(" / ", binding.Select(PadLabel));
        }

        static string PadLabel(int value) => value switch
        {
            0 => "A", 1 => "B", 2 => "X", 3 => "Y",
            4 => "View", 6 => "Menu", 7 => "LS", 8 => "RS",
            9 => "LB", 10 => "RB", 11 => "D-Pad Up", 12 => "D-Pad Down",
            13 => "D-Pad Left", 14 => "D-Pad Right", 100 => "LT", 101 => "RT",
            102 => "L-Stick Left", 103 => "L-Stick Right",
            104 => "L-Stick Up", 105 => "L-Stick Down",
            106 => "R-Stick Left", 107 => "R-Stick Right",
            108 => "R-Stick Up", 109 => "R-Stick Down",
            _ => $"Button {value}",
        };

        static KeyBindings Keys =>
            _player == 0 ? ConfigManager.Game.Keys : ConfigManager.Game.Keys2;
        static GamepadBindings Pad =>
            _player == 0 ? ConfigManager.Game.Pad : ConfigManager.Game.Pad2;

        static string GetKeyBinding() => _action switch
        {
            0 => Keys.Cross, 1 => Keys.Circle, 2 => Keys.Square,
            3 => Keys.Triangle, 4 => Keys.L1, 5 => Keys.R1,
            6 => Keys.L2, 7 => Keys.R2, 8 => Keys.L3, 9 => Keys.R3,
            10 => Keys.Start, 11 => Keys.Select, 12 => Keys.Up,
            13 => Keys.Down, 14 => Keys.Left, _ => Keys.Right,
        };

        static void SetKeyBinding(string value)
        {
            switch (_action)
            {
                case 0: Keys.Cross = value; break;
                case 1: Keys.Circle = value; break;
                case 2: Keys.Square = value; break;
                case 3: Keys.Triangle = value; break;
                case 4: Keys.L1 = value; break;
                case 5: Keys.R1 = value; break;
                case 6: Keys.L2 = value; break;
                case 7: Keys.R2 = value; break;
                case 8: Keys.L3 = value; break;
                case 9: Keys.R3 = value; break;
                case 10: Keys.Start = value; break;
                case 11: Keys.Select = value; break;
                case 12: Keys.Up = value; break;
                case 13: Keys.Down = value; break;
                case 14: Keys.Left = value; break;
                default: Keys.Right = value; break;
            }
        }

        static int[] GetPadBinding() => _action switch
        {
            0 => Pad.Cross, 1 => Pad.Circle, 2 => Pad.Square,
            3 => Pad.Triangle, 4 => Pad.L1, 5 => Pad.R1,
            6 => Pad.L2, 7 => Pad.R2, 8 => Pad.L3, 9 => Pad.R3,
            10 => Pad.Start, 11 => Pad.Select, 12 => Pad.Up,
            13 => Pad.Down, 14 => Pad.Left, _ => Pad.Right,
        };

        static void SetPadBinding(int[] value)
        {
            switch (_action)
            {
                case 0: Pad.Cross = value; break;
                case 1: Pad.Circle = value; break;
                case 2: Pad.Square = value; break;
                case 3: Pad.Triangle = value; break;
                case 4: Pad.L1 = value; break;
                case 5: Pad.R1 = value; break;
                case 6: Pad.L2 = value; break;
                case 7: Pad.R2 = value; break;
                case 8: Pad.L3 = value; break;
                case 9: Pad.R3 = value; break;
                case 10: Pad.Start = value; break;
                case 11: Pad.Select = value; break;
                case 12: Pad.Up = value; break;
                case 13: Pad.Down = value; break;
                case 14: Pad.Left = value; break;
                default: Pad.Right = value; break;
            }
        }

        static int WriteLatin1(
            IMemory m, uint address, string text, int maxCharacters)
        {
            byte[] bytes = Encoding.Latin1.GetBytes(text);
            int length = Math.Min(bytes.Length, maxCharacters);
            for (int index = 0; index < length; index++)
                m.WriteU8(address + (uint)index, bytes[index]);
            m.WriteU8(address + (uint)length, 0);
            return length + 1;
        }

        static void CopyBytes(
            IMemory m, uint source, uint destination, int count)
        {
            for (int index = 0; index < count; index++)
                m.WriteU8(destination + (uint)index,
                    m.ReadU8(source + (uint)index));
        }

        static void WriteRect(
            IMemory m, uint address,
            ushort x, ushort y, ushort width, ushort height)
        {
            m.WriteU16(address, x);
            m.WriteU16(address + 2u, y);
            m.WriteU16(address + 4u, width);
            m.WriteU16(address + 6u, height);
        }
    }
}
