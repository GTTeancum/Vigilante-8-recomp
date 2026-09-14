using System.Text;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

public static partial class Vigilante82PC
{
    static partial class V82LocalMenu
    {
        static uint _scratch;
        public static int[] Devices { get; private set; } = [];

        sealed class MenuScope(CpuContext cpu, IMemory memory, uint address) : IDisposable
        {
            public void Dispose()
            {
                var saved = cpu.Snapshot();
                cpu.A0 = address;
                V82Compat.PcFree(cpu, memory);
                cpu.Restore(saved);
                _scratch = 0;
            }
        }

        public static IDisposable Enter(CpuContext c, IMemory m)
        {
            SplitResetSession();
            Devices = [];
            LocalInputSession.Reset();
            var saved = c.Snapshot();
            c.A0 = 2048;
            V82Compat.PcMalloc(c, m);
            _scratch = c.V0;
            c.Restore(saved);
            if (_scratch == 0) throw new OutOfMemoryException("local menu text");
            Write(m, _scratch, "MULTIPLAYER");
            return new MenuScope(c, m, _scratch);
        }

        public static void Label(CpuContext c, IMemory m)
        {
            if (c.S3 == 0)
            {
                if (LocalInputSession.PlayerCount != 0) { LocalInputSession.Reset(); Devices = []; }
                if (c.S0 == 1) c.A1 = _scratch;
            }
        }

        static void Write(IMemory m, uint address, string text)
        {
            byte[] bytes = Encoding.Latin1.GetBytes(text + '\0');
            if (bytes.Length > 512) throw new ArgumentOutOfRangeException(nameof(text));
            for (uint i = 0; i < bytes.Length; i++) m.WriteU8(address + i, bytes[i]);
        }

        public static void Join(CpuContext c, IMemory m)
        {
            var saved = c.Snapshot();
            uint menuStack = c.SP;
            uint titleFont = c.S4;
            uint smallFont = 0;
            ushort[] held = Enumerable.Range(0, LocalInputSession.DeviceCount).Select(p => (ushort)~LocalMultiplayerInput.DeviceState(p)).ToArray();
            bool[] connected = new bool[LocalInputSession.DeviceCount];
            var join = new LocalJoinState(held);
            bool previousEnter = LocalMultiplayerInput.EnterHeld;
            c.SP -= 0x100;
            uint Call(Action setup, Action<CpuContext, IMemory> function)
            {
                var registers = c.Snapshot();
                setup();
                c.RA = 0x80104360;
                function(c, m);
                uint result = c.V0;
                c.Restore(registers);
                return result;
            }
            void Text(uint font, string text, ushort x, ushort y, ushort width, uint flags = 8)
            {
                Write(m, _scratch + 64, text);
                uint layout = _scratch + 640;
                for (uint i = 0; i < 16; i++) m.WriteU8(layout + i, m.ReadU8(0x80115FB8 + i));
                for (uint offset = 0; offset <= 8; offset += 8)
                {
                    m.WriteU16(layout + offset, x); m.WriteU16(layout + offset + 2, y);
                    m.WriteU16(layout + offset + 4, width); m.WriteU16(layout + offset + 6, 36);
                }
                Call(() => { c.A0 = font; c.A1 = _scratch + 64; c.A2 = layout; c.A3 = flags; }, func_8001A3B0);
            }
            try
            {
                // Use the existing artwork decoder rather than allocating a
                // second full-screen VRAM copy alongside the retail caches.
                Call(() => { c.A0 = m.ReadU32(menuStack + 0x44); c.A1 = 0; c.A2 = m.ReadU32(menuStack + 0x4C); }, func_8001B05C);
                Call(() => { c.A0 = m.ReadU32(menuStack + 0x40); c.A1 = 58; c.A2 = 280; }, func_8001B05C);
                uint root = m.ReadU32(0x80116730);
                smallFont = Call(() => { c.A0 = root + m.ReadU32(root + 0x10); c.A1 = 1; }, func_80019294);
                LocalMultiplayerInput.SignalStage("multiplayer_join");
                Console.Error.WriteLine("[LocalJoin] opened: A joins, P1 Start/Enter confirms");
                while (!join.Confirmed && !join.Cancelled)
                {
                    LocalMultiplayerInput.MenuContext();
                    // The rotating model changes the draw environment each
                    // frame; restore the retail 640x480 UI clip before text.
                    Call(() => { }, func_8001AAFC);
                    Call(() => c.A0 = 0, func_80054C4C);
                    Call(() => { c.A0 = m.ReadU32(menuStack + 0x40); c.A1 = 58; c.A2 = 280; }, func_8001B05C);
                    Call(() => { c.A0 = m.ReadU32(menuStack + 0x44); c.A1 = 0; c.A2 = m.ReadU32(menuStack + 0x4C); }, func_8001B05C);
                    Text(titleFont, "JOIN GAME", 58, 280, 360, 0x4440);
                    for (int player = 0; player < 4; player++)
                    {
                        ushort y = (ushort)(320 + player * 24);
                        string key = InputPromptLabels.Button(ConfigManager.Game, 0, Controller.Cross, false, false, true).ToUpperInvariant();
                        string invitation = join.Devices.Contains(LocalInputSession.KeyboardDevice) ? "A TO JOIN" : $"A / {key} TO JOIN";
                        string state = player >= join.Devices.Count ? invitation
                            : LocalMultiplayerInput.DeviceConnected(join.Devices[player])
                                ? join.Devices[player] == LocalInputSession.KeyboardDevice ? "KEYBOARD - X: LEAVE" : "GAMEPAD - B: LEAVE"
                                : "RECONNECT";
                        Text(smallFont, $"P{player + 1}  {state}", 58, y, 360, 0);
                    }
                    string back = join.Devices.Count > 0 && join.Devices[0] == LocalInputSession.KeyboardDevice ? "S" : "Y";
                    Text(smallFont, join.Devices.Count >= 2 ? $"P1 START / ENTER: CONTINUE   {back}: BACK" : "REQUIRES AT LEAST TWO PLAYERS", 58, (ushort)m.ReadU32(menuStack + 0x4C), 560, 0);
                    // Keep the native frame pump; this page has no row cursor.
                    Call(() => { }, func_800149AC);
                    Call(() => { }, func_8002DE18);
                    Call(() => c.A0 = m.ReadU32(0x8006B4D0) + 0x3FFC, func_8005C57C);
                    Call(() => c.A0 = 0, func_80054C4C);
                    Call(() => c.A0 = 0, func_8005BF88);
                    Call(() => { }, func_80015540);
                    for (int device = 0; device < held.Length; device++)
                    {
                        held[device] = (ushort)~LocalMultiplayerInput.DeviceState(device);
                        connected[device] = LocalMultiplayerInput.DeviceConnected(device);
                    }
                    bool enter = LocalMultiplayerInput.EnterHeld;
                    int oldCount = join.Devices.Count;
                    join.Update(held, connected, enter && !previousEnter);
                    previousEnter = enter;
                    if (oldCount != join.Devices.Count)
                        Console.Error.WriteLine($"[LocalJoin] devices={string.Join(',', join.Devices.Select(p => p + 1))}");
                }
                if (join.Confirmed)
                {
                    Devices = join.Devices.ToArray();
                    LocalInputSession.Begin(Devices);
                }
                // A confirmation/back press belongs to this page only. The
                // retail menu also accepts Start, so wait for release before
                // handing its input image back to that menu.
                do
                {
                    Call(() => c.A0 = 0, func_80054C4C);
                    Call(() => c.A0 = 0, func_8005BF88);
                    Call(() => { }, func_80015540);
                } while (Enumerable.Range(0, 4).Any(p =>
                    ((ushort)~Controller.GetState(p) & (Controller.Start | Controller.Cross |
                        Controller.Circle | Controller.Triangle | Controller.Square)) != 0));
                // 80015104 writes an eight-entry pad ring (GP+CF8), while
                // 80015540 consumes one sample at GP+D08. Discard samples
                // owned by this page, then rebuild the retail edge/combined
                // images from the released sample before the next page.
                m.WriteU8(c.GP + 0xD08, m.ReadU8(c.GP + 0xCF8));
                Call(() => { }, func_80015540);
                Console.Error.WriteLine($"[LocalJoin] {(join.Confirmed ? "confirmed" : "cancelled")} humans={Devices.Length}");
                LocalMultiplayerInput.SignalStage(join.Confirmed ? "multiplayer_join_confirmed" : "multiplayer_join_cancelled");
            }
            finally
            {
                if (smallFont != 0) Call(() => c.A0 = smallFont, func_80019320);
                c.Restore(saved);
                m.WriteU32(menuStack + 0x48, 0xFFFFFFFF);
            }
            c.S3 = 0; c.S2 = 1; c.S7 = 2;
            c.S5 = join.Confirmed ? 5u : 1000u;
            if (join.Confirmed && Devices.Length == 2)
            {
                // Retain the retail Versus / Cooperative / Quest choices.
                c.S3 = 6; c.S2 = 6; c.S7 = 8; c.S5 = 1000;
                LocalMultiplayerInput.SignalStage("multiplayer_mode");
            }
        }

    }
}
