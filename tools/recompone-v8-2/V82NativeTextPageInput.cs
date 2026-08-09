using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Memory;

namespace Recompiled;

/// <summary>
/// Keeps the Back Story and Credits pages responsive.
///
/// Both are the same body, func_8010E854, and both leave only when
/// <c>func_800117C0() &amp; 0x50900000</c> is non-zero. After a hook-owned page
/// has held the frame -- the Video page is the only one that does -- the
/// guest's pad pipeline goes quiet for that body specifically: the host pad
/// shows the button down and the runtime's own state is correct, but the word
/// the page reads stays zero for as long as it is on screen, so it can never
/// be left. Every other Options page keeps working, including after visiting
/// Video, so this is narrow rather than systemic.
///
/// The cause is somewhere below the shell's pad sampling and is not understood
/// yet; what is measurable is that the page is asking a source that has stopped
/// answering. This supplies the answer from the runtime's own pad, in the
/// shell's own bit order, and only on frames where the retail read came back
/// empty -- so whenever the pipeline is healthy the retail value is used
/// untouched and nothing here participates.
/// </summary>
public static partial class Vigilante82PC
{
    static class V82NativeTextPageInput
    {
        const uint PadWord = 0x8006B4ECu;

        // The processed word's pressed half, in the shell's order rather than
        // Controller's. Observed directly: pressing Triangle yields
        // 0x00100010, and the Options loop moves its cursor on 0x10000000 and
        // 0x40000000 and leaves the screen on 0x00900000.
        const uint ShellPreviousRow = 0x10000000u;
        const uint ShellNextRow = 0x40000000u;
        const uint ShellBack = 0x00100000u;
        const uint ShellMainMenu = 0x00800000u;

        const uint Watched =
            Controller.Up | Controller.Down |
            Controller.Triangle | Controller.Square;

        static uint _previous;

        /// <summary>
        /// Runs on the instruction after <c>jal func_800117C0</c> at
        /// 0x8010E9D4, with the pad word the page is about to test in V0.
        /// </summary>
        public static void SupplyPad(CpuContext c, IMemory m)
        {
            uint input = (uint)(~Controller.State & 0xFFFF);
            uint pressed = (input & ~_previous) & Watched;
            _previous = input & Watched;

            if (c.V0 != 0u || pressed == 0u)
                return;

            uint bits = 0u;
            if ((pressed & Controller.Up) != 0u) bits |= ShellPreviousRow;
            if ((pressed & Controller.Down) != 0u) bits |= ShellNextRow;
            if ((pressed & Controller.Triangle) != 0u) bits |= ShellBack;
            if ((pressed & Controller.Square) != 0u) bits |= ShellMainMenu;

            // The page tests V0; the Options loop that regains control reads
            // the same word out of memory to move the row cursor, and it is
            // just as empty, so both need supplying.
            c.V0 = bits;
            m.WriteU32(PadWord, bits);
        }
    }
}
