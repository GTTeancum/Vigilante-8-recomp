using System.Text;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host.Window;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

/// <summary>
/// The Video page behind the appended Options row (TO-DO item 4).
///
/// func_8010EA88 is the Options screen's outer loop. It redraws the row list,
/// dispatches the selected row through a seven-entry jump table at 0x80101180
/// guarded by <c>sltiu v0, s1, 7</c>, then reads the pad to move the cursor.
/// Row seven has no table entry, so <see cref="Dispatch"/> is an inline hook
/// on that bound check with a branch to the loop's common continuation at
/// 0x8010ECE0: it claims the row when the cursor sits on it and lets the
/// retail table run otherwise.
///
/// That outer loop is not a frame loop -- it contains no VSync. Every retail
/// sub-page owns its own inner loop instead (Audio's runs 0x8010B9AC-0x8010C17C
/// around <c>VSync(0)</c> and func_80015540), and returns only when the pad
/// carries one of the bits that moves the row cursor or leaves the screen.
/// Branching past the dispatch without supplying that loop simply stops the
/// game presenting, so this page runs one too, exiting on the same 0x50900000
/// mask the Back Story and Credits pages use.
///
/// The drawing follows V82NativeControlOptions, the working precedent for
/// retail-native content inside a shell page: the same content-field fill, the
/// same text object and text call, and the same footer compositor.
/// </summary>
public static partial class Vigilante82PC
{
    static class V82NativeVideoPage
    {
        const uint PadWord = 0x8006B4ECu;
        const uint ShellUiRoot = 0x80116730u;
        const uint RetailTextLayout = 0x80115FB8u;
        const uint RetailFooterStrings = 0x801009C0u;

        // Clear of V82NativeVideoOption's row string at 0x8011AE80 and of
        // V82NativeControlOptions' block, which starts at 0x8011AF00 and whose
        // footer strings reach 0x8011B040 in its five-prompt state -- 0x8011B000
        // sits inside that. This page needs under 0x100 bytes.
        const uint ScratchBase = 0x8011B100u;
        const uint ScratchLayout = ScratchBase;
        const uint ScratchText = ScratchBase + 0x20u;
        const uint ScratchFooter = ScratchBase + 0xA0u;

        // Any address inside the shell overlay serves as the return address
        // for a call issued from a hook; the controls page uses this one.
        const uint ReturnAddress = 0x8010A8ECu;

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

        // Content field of the Options panel, in the same doubled coordinate
        // space the controls page uses, and the RGB sampled from OPTIONS.PSX.
        const ushort FieldX = 0x00F0;
        const ushort FieldY = 0x007C;
        const ushort FieldWidth = 0x0190;
        const ushort FieldHeight = 0x0128;
        const uint FieldColor = 0x00182018u;

        // Six rows at the retail 0x2C spacing run 0x0080-0x014C and leave the
        // hint a clear line of its own; a seventh would land on top of it.
        const ushort FirstRowY = 0x0080;
        const ushort RowSpacing = 0x002C;
        const ushort HintY = 0x0188;
        const int VisibleRows = 6;

        const string Cursor = "";

        // The processed pad bits that end a sub-page: the two the outer loop
        // uses to move the row cursor (0x10000000 previous, 0x40000000 next)
        // and the pair that leaves Options (0x00900000). Back Story and
        // Credits exit on exactly this mask.
        const uint PageExitMask = 0x50900000u;

        // The same four controls on the runtime's own pad. Arming has to watch
        // these rather than the processed word: leaving editing consumes that
        // word, so it reads clear for a frame while the button is still down,
        // and the exit would arm and then immediately fire on the same press.
        const uint ExitButtons = Up | Down | Triangle | Square;

        static bool _editing;
        static bool _exitArmed;
        static int _row;
        static int _top;
        static uint _previousInput;

        /// <summary>
        /// Inline hook on the jump-table bound check at 0x8010EC58. Returns
        /// true to branch past the retail dispatch to 0x8010ECE0.
        /// </summary>
        public static bool Dispatch(CpuContext c, IMemory m)
        {
            if ((int)c.S1 != V82NativeVideoOption.VideoRowIndex)
                return false;

            var snapshot = c.Snapshot();
            try { RunPage(c, m); }
            finally { c.Restore(snapshot); }
            return true;
        }

        /// <summary>
        /// The page's own frame loop, in the shape every retail sub-page uses.
        /// </summary>
        static void RunPage(CpuContext c, IMemory m)
        {
            _editing = false;
            // The press that walked the cursor onto this row is still in the
            // processed word, and it is one of the bits that would end the
            // page again. Arm on the first frame that shows the mask clear.
            _exitArmed = false;
            _row = 0;
            _top = 0;
            // Same reason for the edge detector: seed it from what is held
            // right now so a carried-over press is never read as a new one.
            _previousInput = (uint)(~Controller.State & 0xFFFF) & HandledInput;
            V82Compat.SignalNativeVideoPage();
            UpdateFooter(c, m);

            while (true)
            {
                RedrawRowList(c, m);
                DrawPage(c, m);
                uint pad = PumpFrame(c, m);

                uint input = (uint)(~Controller.State & 0xFFFF);
                uint pressed = (input & ~_previousInput) & HandledInput;
                _previousInput = input & HandledInput;

                if (!_editing)
                {
                    // Browsing: the row list still owns the cursor, so hand
                    // the frame back and let the outer loop act on the press.
                    if (!_exitArmed)
                        _exitArmed = (input & ExitButtons) == 0u;
                    else if ((pad & PageExitMask) != 0u)
                    {
                        SetFooter(c, m, 4, RetailFooterStrings);
                        return;
                    }
                    if ((pressed & Cross) != 0u)
                    {
                        _editing = true;
                        UpdateFooter(c, m);
                    }
                    continue;
                }

                if ((pressed & Triangle) != 0u)
                {
                    // Triangle is also the shell's "leave Options" bit, so
                    // returning to browsing has to disarm the exit until the
                    // button is seen released.
                    _editing = false;
                    _exitArmed = false;
                    UpdateFooter(c, m);
                }
                else
                {
                    if ((pressed & Up) != 0u)
                        _row = Wrap(_row - 1, Rows.Length);
                    if ((pressed & Down) != 0u)
                        _row = Wrap(_row + 1, Rows.Length);
                    if ((pressed & Left) != 0u)
                        Change(-1);
                    if ((pressed & (Right | Cross | Circle)) != 0u)
                        Change(1);
                    ScrollIntoView();
                }

                // While editing, every control this page understands belongs
                // to it. Clear them from the processed word so the outer loop
                // cannot also move the row cursor or leave the screen.
                ConsumeNativePadWord(m);
            }
        }

        /// <summary>
        /// The outer loop draws the row list once per iteration, and this page
        /// holds the frame for many, so the list has to be rebuilt here or the
        /// window shift that puts Video on the last plate stays half-applied.
        /// func_80108B48 direct, not func_80108D1C, because the latter also
        /// resets the footer to the retail prompts.
        /// </summary>
        static void RedrawRowList(CpuContext c, IMemory m)
        {
            var snapshot = c.Snapshot();
            try
            {
                c.A0 = 0u;
                c.A1 = (uint)V82NativeVideoOption.VideoRowIndex;
                c.RA = ReturnAddress;
                func_80108B48(c, m);
            }
            finally
            {
                c.Restore(snapshot);
            }
        }

        /// <summary>
        /// One frame: present, then service the pad. VSync is what reaches the
        /// host's present and input poll; func_80015540 returns the processed
        /// word it has just refreshed at <see cref="PadWord"/>.
        /// </summary>
        static uint PumpFrame(CpuContext c, IMemory m)
        {
            c.A0 = 0u;
            c.RA = ReturnAddress;
            func_80054C4C(c, m);
            c.RA = ReturnAddress;
            func_80015540(c, m);
            return c.V0;
        }

        static void ScrollIntoView()
        {
            if (_row < _top)
                _top = _row;
            else if (_row >= _top + VisibleRows)
                _top = _row - VisibleRows + 1;
            _top = Math.Clamp(_top, 0, Math.Max(0, Rows.Length - VisibleRows));
        }

        static void ConsumeNativePadWord(IMemory m)
        {
            uint processed = m.ReadU32(PadWord);
            m.WriteU32(PadWord, processed & ~ProcessedHandledInput);
        }

        static void DrawPage(CpuContext c, IMemory m)
        {
            var snapshot = c.Snapshot();
            try
            {
                // Retail GPU fill of the stock content field, not a host
                // overlay, so the panel border, title and footer layers the
                // shell owns are left untouched.
                WriteRect(m, ScratchLayout,
                    FieldX, FieldY, FieldWidth, FieldHeight);
                c.A0 = ScratchLayout;
                c.A1 = FieldColor;
                c.RA = ReturnAddress;
                func_8001ADF8(c, m);

                uint root = m.ReadU32(ShellUiRoot);
                uint childOffset = m.ReadU32(root + 0x10u);
                c.A0 = root + childOffset;
                c.A1 = 1u;
                c.RA = ReturnAddress;
                func_80019294(c, m);
                uint textObject = c.V0;
                uint normalColor = m.ReadU32(textObject + 0x4u);

                int last = Math.Min(_top + VisibleRows, Rows.Length);
                for (int index = _top; index < last; index++)
                {
                    Row row = Rows[index];
                    bool selected = _editing && index == _row;
                    ushort y = (ushort)(
                        FirstRowY + (index - _top) * RowSpacing);
                    string prefix = selected ? Cursor : "  ";
                    SetObjectColor(m, textObject, normalColor, selected);
                    DrawText(c, m, textObject,
                        prefix + row.Label + ": " + row.Value(),
                        y, 0x00000008u);
                }

                SetObjectColor(m, textObject, normalColor, selected: false);
                DrawText(c, m, textObject, Hint(), HintY, 0x0000000Au);

                c.A0 = textObject;
                c.RA = ReturnAddress;
                func_80019320(c, m);
            }
            finally
            {
                c.Restore(snapshot);
            }
        }

        static string Hint()
        {
            if (!_editing)
                return "Press X to configure";
            int below = Rows.Length - (_top + VisibleRows);
            return below > 0 ? $"{below} more below" : "";
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
            if (text.Length == 0)
                return;
            WriteLatin1(m, ScratchText, text, 95);
            CopyBytes(m, RetailTextLayout, ScratchLayout, 16);
            WriteRect(m, ScratchLayout, 0x0100, y, 0x0180, 0x0024);
            WriteRect(m, ScratchLayout + 8u, 0x0100, y, 0x0180, 0x0024);
            c.A0 = textObject;
            c.A1 = ScratchText;
            c.A2 = ScratchLayout;
            c.A3 = flags;
            c.RA = ReturnAddress;
            func_8001A3B0(c, m);
        }

        static void UpdateFooter(CpuContext c, IMemory m)
        {
            uint cursor = ScratchFooter;
            string[] prompts = _editing
                ? ["select", "change", "back"]
                : ["configure"];
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
                c.RA = ReturnAddress;
                func_80108CB8(c, m);
            }
            finally
            {
                c.Restore(snapshot);
            }
        }

        // ---- the option set --------------------------------------------

        sealed record Row(string Label, Func<string> Value, Action<int> Change);

        static readonly string[] Presets = ["Original", "Enhanced"];
        static readonly string[] Resolutions =
            ["1280x720", "1920x1080", "2560x1440", "3840x2160"];
        static readonly string[] AntiAliasingModes = ["Off", "FXAA"];
        static readonly string[] LevelOfDetailModes = ["Stock", "Maximum"];
        static readonly int[] MsaaModes = [0, 2, 4, 8];
        static readonly int[] AnisotropicModes = [1, 2, 4, 8, 16];
        static readonly int[] InternalScales = [1, 2, 3, 4];

        static ViewConfig View => ConfigManager.View;

        static readonly Row[] Rows =
        [
            new("Preset", () => View.ResolveGraphicsPreset(), direction =>
            {
                // "Custom" is a reported state, not a selectable one, so
                // cycling out of it lands on a real preset rather than
                // re-applying the values the player just chose.
                int index = Array.IndexOf(
                    Presets, View.ResolveGraphicsPreset());
                index = index < 0
                    ? (direction > 0 ? 0 : Presets.Length - 1)
                    : Wrap(index + direction, Presets.Length);
                View.ApplyGraphicsPreset(Presets[index]);
                V82Compat.ApplyGraphicsConfiguration();
                SaveView();
            }),
            new("Resolution", () => View.OutputResolution, direction =>
            {
                View.OutputResolution =
                    Cycle(Resolutions, View.OutputResolution, direction);
                V82Compat.SetOutputResolution(View.OutputResolution);
                SaveView();
            }),
            new("Fullscreen", () => OnOff(View.Fullscreen), _ =>
            {
                View.Fullscreen = !View.Fullscreen;
                V82Compat.SetFullscreen(View.Fullscreen);
                SaveView();
            }),
            new("Widescreen", () => OnOff(View.Widescreen), _ =>
            {
                View.Widescreen = !View.Widescreen;
                SaveCustom();
            }),
            new("Internal 3D", () => $"{View.InternalResolutionScale}x",
            direction =>
            {
                int scale = Cycle(
                    InternalScales, View.InternalResolutionScale, direction);
                View.InternalResolutionScale = scale;
                View.HighResolution3D = scale > 1;
                V82Compat.ApplyGraphicsConfiguration();
                SaveCustom();
            }),
            new("Anti-aliasing", () => View.AntiAliasing, direction =>
            {
                View.AntiAliasing =
                    Cycle(AntiAliasingModes, View.AntiAliasing, direction);
                SaveCustom();
            }),
            new("MSAA",
            () => View.MsaaSamples <= 1 ? "Off" : $"{View.MsaaSamples}x",
            direction =>
            {
                View.MsaaSamples =
                    Cycle(MsaaModes, View.MsaaSamples, direction);
                SaveCustom();
            }),
            new("Anisotropic", () => $"{View.AnisotropicFiltering}x",
            direction =>
            {
                View.AnisotropicFiltering = Cycle(
                    AnisotropicModes, View.AnisotropicFiltering, direction);
                SaveCustom();
            }),
            new("Texture smoothing", () => OnOff(View.TextureSmoothing), _ =>
            {
                View.TextureSmoothing = !View.TextureSmoothing;
                SaveCustom();
            }),
            new("Mipmaps", () => OnOff(View.TextureMipmaps), _ =>
            {
                View.TextureMipmaps = !View.TextureMipmaps;
                SaveCustom();
            }),
            new("Level of detail", () => View.LevelOfDetail, direction =>
            {
                View.LevelOfDetail =
                    Cycle(LevelOfDetailModes, View.LevelOfDetail, direction);
                SaveCustom();
            }),
            new("Draw distance",
            () => View.ExtendedDrawDistance ? "Extended" : "Stock", _ =>
            {
                View.ExtendedDrawDistance = !View.ExtendedDrawDistance;
                SaveCustom();
            }),
            new("Fog", () => OnOff(View.EnhancedFog), _ =>
            {
                View.EnhancedFog = !View.EnhancedFog;
                SaveCustom();
            }),
            new("Shadows", () => OnOff(View.EnhancedShadows), _ =>
            {
                View.EnhancedShadows = !View.EnhancedShadows;
                SaveCustom();
            }),
            new("Particles", () => OnOff(View.EnhancedParticles), _ =>
            {
                View.EnhancedParticles = !View.EnhancedParticles;
                SaveCustom();
            }),
            new("HUD anchoring", () => OnOff(View.HudAnchoring), _ =>
            {
                View.HudAnchoring = !View.HudAnchoring;
                SaveCustom();
            }),
        ];

        static void Change(int direction) => Rows[_row].Change(direction);

        static string OnOff(bool value) => value ? "On" : "Off";

        static int Wrap(int index, int count) => (index + count) % count;

        static T Cycle<T>(T[] values, T current, int direction)
        {
            int index = Array.IndexOf(values, current);
            if (index < 0)
                index = 0;
            return values[Wrap(index + direction, values.Length)];
        }

        static void SaveCustom()
        {
            View.MarkGraphicsCustom();
            SaveView();
        }

        static void SaveView() => ConfigManager.SaveView(PanelManager.Panels);

        // ---- shared memory helpers --------------------------------------

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
