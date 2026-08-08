using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace Recompiled;

/// <summary>
/// A VIDEO row appended to the retail Options list (TO-DO item 4).
///
/// The list is drawn by func_80108B48: seven string pointers at 0x80115E60,
/// walked with a stride of four, and a row count compiled in as
/// <c>S0 &lt; 7</c>. A constant cannot be widened by a hook, but the loop does
/// not need to be replaced either -- at its exit every piece of state a row
/// needs is still live, so an eighth row can simply be drawn there:
///
///   S1      the text object, whose word at +4 carries the row colour
///   SP+0x10 the layout rect, whose Y has already been advanced past row seven
///   S4      the selected row index
///   S3      non-zero when the page is drawn in its disabled/greyed state
///
/// The colour selection below mirrors the loop's own, so the appended row
/// highlights and greys exactly like the retail seven.
/// </summary>
public static partial class Vigilante82PC
{
    static class V82NativeVideoOption
    {
        // Retail draws each row through this text call with a width code of 8.
        const uint DrawText = 0x8001A3B0u;

        // Scratch well above the shell's own use, matching the convention the
        // controls page already follows for its borrowed layout and footer.
        const uint ScratchString = 0x8011AE80u;

        const uint DisabledColor = 0x00576044u;
        const uint SelectedColor = 0x00808080u;
        const uint NormalColorBits = 0x0000607Cu;

        /// <summary>Index the appended row occupies in the retail list.</summary>
        public const int RowIndex = 7;

        static bool _stringWritten;

        // The retail list is laid out for seven rows: X=40, Y=172, stride 34,
        // written into the layout rect at SP+0x10 just before the loop. An
        // eighth row runs off the bottom of the panel, so lift the start by
        // one row height. Both are tunable while the surrounding art is being
        // redesigned.
        static readonly int StartY = EnvInt("RECOMPONE_V82_OPTIONS_START_Y", 138);
        static readonly int RowStride = EnvInt("RECOMPONE_V82_OPTIONS_ROW_STRIDE", 34);

        static int EnvInt(string name, int fallback) =>
            int.TryParse(Environment.GetEnvironmentVariable(name), out int v)
                ? v : fallback;

        /// <summary>Runs once, immediately before the row loop.</summary>
        public static void AdjustLayout(CpuContext c, IMemory m)
        {
            IMemory raw = Dispatcher.UnwrapMemory(m);
            raw.WriteU16(c.SP + 0x12u, (ushort)StartY);
            raw.WriteU16(c.SP + 0x16u, (ushort)RowStride);
        }

        static void EnsureString(IMemory m)
        {
            if (_stringWritten) return;
            _stringWritten = true;
            // The retail rows are mixed case ("Game Status", "Controllers").
            const string label = "Video";
            for (int i = 0; i < label.Length; i++)
                m.WriteU8(ScratchString + (uint)i, (byte)label[i]);
            m.WriteU8(ScratchString + (uint)label.Length, 0);
        }

        public static void AppendRow(CpuContext c, IMemory m)
        {
            IMemory raw = Dispatcher.UnwrapMemory(m);
            EnsureString(raw);

            uint textObject = c.S1;
            if (textObject < 0x80000000u) return;

            uint carried = raw.ReadU32(textObject + 4u) & 0xFF000000u;
            uint color =
                c.S3 != 0u ? DisabledColor
                : (int)c.S4 == RowIndex ? SelectedColor
                : carried | NormalColorBits;
            if (c.S3 == 0u && (int)c.S4 == RowIndex) color = carried | SelectedColor;
            raw.WriteU32(textObject + 4u, color);

            var snapshot = c.Snapshot();
            c.A0 = textObject;
            c.A1 = ScratchString;
            c.A2 = c.SP + 0x10u;
            c.A3 = 8u;
            Dispatcher.Call(c, m, DrawText);
            c.Restore(snapshot);
        }
    }
}
