using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace Recompiled;

/// <summary>
/// A VIDEO row added to the retail Options list (TO-DO item 4), sitting under
/// Audio so the two audio/video pages are together and the informational rows
/// stay last: Game Status, Memory Card, Difficulty, Controllers, Audio,
/// <b>Video</b>, Back Story, Credits.
///
/// The list is drawn by func_80108B48: seven string pointers at 0x80115E60,
/// walked with a stride of four by S2, a row count compiled in as
/// <c>S0 &lt; 7</c>, the selected index in S4, and a layout rect on the stack
/// starting at Y=172 with a row height of 34.
///
/// The seven row plates behind that text are **not drawn by this loop, or by
/// any other shell code**. They are geometry on the Options panel, which the
/// screen renders as a mesh of small textured quads; moving the loop's layout
/// leaves them exactly where they were, and no table of their positions and no
/// seven-iteration draw loop exists anywhere in SHELL.DLL. So there are seven
/// plates, an eighth cannot be produced from code, and eight rows cannot all
/// be on screen at once.
///
/// They do not need to be. The loop draws seven rows wherever S2 points, so
/// this hook points it into an eight-entry table of its own and slides the
/// window by one when the cursor reaches the last row. Rows zero through six
/// render exactly as retail; on Credits the list scrolls a single step, Game
/// Status leaves the top and Credits occupies the last plate. Every row sits
/// on a plate at its retail position and the panel art is untouched.
/// </summary>
public static partial class Vigilante82PC
{
    static class V82NativeVideoOption
    {
        /// <summary>Index the appended row occupies in the list.</summary>
        public const int VideoRowIndex = 5;

        /// <summary>Highest index the cursor may reach (Credits).</summary>
        public const int LastRowIndex = 7;

        const int RetailRows = 7;
        const int TotalRows = RetailRows + 1;
        const uint RetailRowStrings = 0x80115E60u;

        // Past the shell overlay image, which ends at 0x8011AED8, and clear of
        // V82NativeControlOptions (0x8011AF00, reaching 0x8011B040 with five
        // footer prompts) and V82NativeVideoPage (0x8011B100, under 0x100).
        const uint ScratchTable = 0x8011B200u;
        const uint ScratchString = ScratchTable + 0x20u;
        const uint ScratchRect = ScratchTable + 0x40u;

        const uint ReturnAddress = 0x8010A8ECu;

        // Plate interiors, measured off a stock capture. Each plate has a gold
        // border at X=28..31 and 212..217 with a bevel either side of it, and
        // the flat interior runs X=35..209, Y=179..202 for the first, stepping
        // 34; the fill is inset to clear the rounded corners. The colour is the
        // one the content field is repainted with elsewhere.
        const ushort PlateX = 39;
        const ushort PlateWidth = 169;
        const ushort FirstPlateY = 181;
        const ushort PlateStride = 34;
        const ushort PlateHeight = 21;
        const uint PlateInterior = 0x00182018u;

        // The retail layout: X=40, Y=172, row height 34. Kept tunable because
        // the surrounding art is still open, but the defaults are now retail
        // and the window below is what makes eight rows fit.
        static readonly int StartY = EnvInt("RECOMPONE_V82_OPTIONS_START_Y", 172);
        static readonly int RowStride = EnvInt("RECOMPONE_V82_OPTIONS_ROW_STRIDE", 34);

        static bool _tableWritten;
        static uint _jumpTableBase;
        static int _lastTop;
        static int _eraseBuilds;

        static int EnvInt(string name, int fallback) =>
            int.TryParse(Environment.GetEnvironmentVariable(name), out int v)
                ? v : fallback;

        /// <summary>
        /// Runs immediately before the row loop, where S2 has just been set to
        /// the retail string table and S4 holds the selected index.
        /// </summary>
        public static void AdjustLayout(CpuContext c, IMemory m)
        {
            IMemory raw = Dispatcher.UnwrapMemory(m);
            EnsureTable(raw);

            raw.WriteU16(c.SP + 0x12u, (ushort)StartY);
            raw.WriteU16(c.SP + 0x16u, (ushort)RowStride);

            // One step of scroll is all eight entries need: only the last row
            // falls outside the seven plates.
            int selected = (int)c.S4;
            int top = selected >= LastRowIndex ? 1 : 0;
            c.S2 = ScratchTable + (uint)top * 4u;
            // The loop compares its 0..6 counter against S4, so the highlight
            // has to be the row's position in the window, not in the list.
            c.S4 = (uint)(selected - top);

            // Only when the window has actually moved. Leaving the retail
            // seven alone keeps their pixels byte-identical to stock; the
            // count covers both draw buffers converging after a change.
            if (top != _lastTop)
            {
                _lastTop = top;
                _eraseBuilds = 4;
            }
            if (_eraseBuilds > 0)
            {
                _eraseBuilds--;
                ErasePlateText(c, m, raw);
            }
        }

        /// <summary>
        /// Nothing repaints this panel between builds: the shell draws it once
        /// and leaves it, which is why the retail seven can be written straight
        /// over themselves forever without anyone noticing. A window that
        /// changes which label sits in which slot does get noticed -- the old
        /// text stays under the new one -- so the plate interiors are cleared
        /// first, the same way the content field is repainted on every page.
        /// </summary>
        static void ErasePlateText(CpuContext c, IMemory m, IMemory raw)
        {
            var snapshot = c.Snapshot();
            try
            {
                for (int row = 0; row < RetailRows; row++)
                {
                    raw.WriteU16(ScratchRect, PlateX);
                    raw.WriteU16(ScratchRect + 2u,
                        (ushort)(FirstPlateY + row * PlateStride));
                    raw.WriteU16(ScratchRect + 4u, PlateWidth);
                    raw.WriteU16(ScratchRect + 6u, PlateHeight);
                    c.A0 = ScratchRect;
                    c.A1 = PlateInterior;
                    c.RA = ReturnAddress;
                    func_8001ADF8(c, m);
                }
            }
            finally
            {
                c.Restore(snapshot);
            }
        }

        static void EnsureTable(IMemory m)
        {
            if (_tableWritten) return;
            _tableWritten = true;

            // The retail rows are mixed case ("Game Status", "Controllers").
            const string label = "Video";
            for (int i = 0; i < label.Length; i++)
                m.WriteU8(ScratchString + (uint)i, (byte)label[i]);
            m.WriteU8(ScratchString + (uint)label.Length, 0);

            // Copy rather than hard-code, so a localised or modded string table
            // still supplies the retail seven. Video is spliced in after Audio,
            // pushing Back Story and Credits down a slot.
            for (uint i = 0; i < VideoRowIndex; i++)
                m.WriteU32(ScratchTable + i * 4u,
                    m.ReadU32(RetailRowStrings + i * 4u));
            m.WriteU32(ScratchTable + VideoRowIndex * 4u, ScratchString);
            for (uint i = VideoRowIndex; i < RetailRows; i++)
                m.WriteU32(ScratchTable + (i + 1) * 4u,
                    m.ReadU32(RetailRowStrings + i * 4u));
        }

        /// <summary>
        /// Runs after the <c>and v0, v0, v1</c> at 0x8010ED18 in the Options
        /// frame loop, where V0 is non-zero exactly when the cursor's
        /// next-row input is down and S1 holds the selected index.
        ///
        /// The clamp on the next instruction is <c>slti v0, s1, 6</c>, a
        /// compiled constant that stops the cursor at the seventh row. Rather
        /// than decode which bit of the shell's processed pad word means
        /// "down", step onto the last row here: the retail clamp then declines
        /// to step again, so the two can never both fire.
        /// </summary>
        public static void ExtendCursorRange(CpuContext c, IMemory m)
        {
            if (c.V0 != 0u && (int)c.S1 == LastRowIndex - 1)
                c.S1 = (uint)LastRowIndex;
        }

        /// <summary>
        /// Runs after the <c>sltiu v0, s1, 7</c> at 0x8010EC58, the bound on
        /// the page jump table, and before S3 is added to the scaled index.
        ///
        /// Two things have to change there. The bound has to admit the eighth
        /// index, or Credits never dispatches. And because Video is spliced in
        /// at five, indices six and seven have to reach Back Story and Credits,
        /// which are the retail table's entries five and six -- so S3 points at
        /// a remapped copy. Entry five of that copy is never read:
        /// V82NativeVideoPage.Dispatch claims the row before the table is
        /// touched.
        /// </summary>
        public static void WidenDispatch(CpuContext c, IMemory m)
        {
            // The dispatch is `v0 = [s3 + s1*4]`, so biasing the base by one
            // entry remaps the tail of the table without copying it anywhere:
            // indices six and seven then read the retail entries five and six,
            // which are Back Story and Credits. Index five never gets here --
            // V82NativeVideoPage.Dispatch claims the row first -- and indices
            // below it are unshifted. Nothing is written to guest memory and
            // the entries stay whatever the loader relocated them to.
            uint s3 = c.S3;
            if (s3 != _jumpTableBase && s3 != _jumpTableBase - 4u)
                _jumpTableBase = s3;
            c.S3 = (int)c.S1 > VideoRowIndex
                ? _jumpTableBase - 4u
                : _jumpTableBase;
            c.V0 = c.S1 < (uint)TotalRows ? 1u : 0u;
        }
    }
}
