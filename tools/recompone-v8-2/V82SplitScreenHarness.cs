using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host;

namespace Recompiled;

public static partial class Vigilante82PC
{
    // Shared native multiplayer integration, entered by the local menu or an
    // explicit process-local development harness. Roster allocation totals six.
    static readonly int SplitHarnessPlayers = int.TryParse(
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_PLAYERS"), out int count)
            && count is >= 2 and <= 4 ? count : 0;
    static bool SplitHarnessStarted;
    static uint SplitLastTrace = uint.MaxValue;
    static readonly uint[] SplitVehicles = new uint[6];
    static uint SplitStorage;
    static int SplitMenuPlayers;
    static uint SplitLastPauseState;
    static byte[]? SplitRematchProfile;
    static byte[]? SplitRematchRoster;
    static uint[]? SplitRematchScores;
    static byte[][]? SplitRematchPlayerRecords;
    static bool SplitRestoringPlayers;
    static uint SplitFixtureAwardAddress;
    static byte SplitFixtureAwardExpected;
    static void SplitResetSession()
    {
        SplitRematchProfile = null; SplitRematchRoster = null; SplitRematchScores = null;
        SplitRematchPlayerRecords = null; SplitRestoringPlayers = false;
    }

    static void SplitRestoreRematch(CpuContext c, IMemory m)
    {
        if (m.ReadU32(c.SP + 0x6C) == 0 || LocalInputSession.PlayerCount < 2 || SplitRematchRoster == null) return;
        if (LocalInputSession.PlayerCount >= 3 && (SplitRematchProfile == null || SplitRematchScores == null)) return;
        SplitMenuPlayers = LocalInputSession.PlayerCount;
        SplitRestoringPlayers = SplitMenuPlayers >= 3;
        SplitAllocate(c, m);
        if (SplitRestoringPlayers)
            for (uint i = 0; i < SplitRematchProfile!.Length; i++) m.WriteU8(SplitStorage + 0x800u + i, SplitRematchProfile[i]);
        for (uint i = 0; i < 6; i++)
        {
            m.WriteU8(c.GP + 0x1104u + i, SplitRematchRoster[i]);
            m.WriteU8(c.GP + 0x1128u + i, (byte)i);
            m.WriteU8(0x8006B802u + i, 1);
        }
        if (SplitRestoringPlayers)
            for (uint i = 0; i < 4; i++) m.WriteU32(SplitStorage + 0xF00u + i*4, SplitRematchScores![i]);
        Console.Error.WriteLine($"[LocalRematch] restored humans={SplitMenuPlayers} roster={string.Join(',', SplitRematchRoster)}");
    }
    static bool SplitVersus(CpuContext c, IMemory m) => SplitPlayers >= 3 && SplitStorage != 0 && m.ReadU8(c.GP + 0x31) == 5;
    static uint SplitResultAddress(CpuContext c, IMemory m, uint address)
    {
        if (!SplitVersus(c, m)) return address;
        if (address >= 0x8006B7E8u && address < 0x8006B7F8u)
            return SplitStorage + 0xF20u + address - 0x8006B7E8u;
        if (address >= c.GP + 0xCB8u && address < c.GP + 0xCC8u)
            return SplitStorage + 0xF00u + address - (c.GP + 0xCB8u);
        return address;
    }
    static uint SplitAwardAddress(CpuContext c, uint address) => SplitPlayers >= 3 && SplitStorage != 0
        && address >= c.GP + 0xC90u && address < c.GP + 0xC98u
            ? SplitStorage + 0xF30u + address - (c.GP + 0xC90u) : address;

    static uint SplitPlayerProgress(CpuContext c, IMemory m, int player, uint native)
    {
        if (!SplitVersus(c, m) || player < 0 || player >= SplitPlayers || SplitVehicles[player] == 0) return native;
        return V82VehicleRegistry.UpgradeStatusForObject(c, m, SplitVehicles[player], native);
    }
    static uint SplitPlayerRecord(CpuContext c, IMemory m, int player)
    {
        uint type = m.ReadU8(c.GP + 0x1104u + (uint)player);
        uint native = SplitUpgradeAddress(0x8006B9E8u + (uint)player * 180 + type * 10);
        return SplitPlayerProgress(c, m, player, native);
    }
    static void SplitCapturePlayerRecords(CpuContext c, IMemory m)
    {
        if (!SplitVersus(c, m)) return;
        SplitRematchPlayerRecords = new byte[SplitPlayers][];
        for (int player = 0; player < SplitPlayers; player++)
        {
            uint address = SplitPlayerRecord(c, m, player);
            SplitRematchPlayerRecords[player] = Enumerable.Range(0, 10).Select(i => m.ReadU8(address + (uint)i)).ToArray();
        }
    }

    static void SplitResultFormat(CpuContext c, IMemory m)
    {
        if (!SplitVersus(c, m) || SplitPlayers != 3) return;
        // Native four-player score markup, with the absent fourth column omitted.
        byte[] format = System.Text.Encoding.Latin1.GetBytes("\u0001\u0080\u0080\u0001\n\u0007 \u0005PLAYER 1\u0007\u00a0\u0005PLAYER 2\n\u0004\u00c0\u0001\u0080\u0080\u0080\u0007 \u0005%i\u0007`\u0005:\u0007\u00a0\u0005%i\u0001\u0080\u0080\u0001\n\u0007`\u0005PLAYER 3\n\u0004\u00c0\u0001\u0080\u0080\u0080\u0007`\u0005%i\0");
        for (uint i = 0; i < format.Length; i++) m.WriteU8(SplitStorage + 0xF40u + i, format[i]);
        c.A1 = SplitStorage + 0xF40;
    }

    static bool SplitTwoResultObserved;
    static void SplitNativeResultContext(CpuContext c, IMemory m)
    {
        if (LocalInputSession.PlayerCount == 2 && c.S6 != 0)
        {
            LocalMultiplayerInput.MenuContext();
            // Retail queues result packets before submitting them later in the
            // frame, like its pause modal. Keep that submission out of HUD anchoring.
            GpuHle.NativeModalHold = Math.Max(4, GpuHle.NativeModalHold);
        }
    }
    static void SplitCheckRound(CpuContext c, IMemory m)
    {
        if (LocalInputSession.PlayerCount == 2 && m.ReadU8(c.GP + 0x31) is 6 or 7 &&
            Environment.GetEnvironmentVariable("RECOMPONE_V82_COOP_RESULT_TEST") == "1")
        {
            if (c.S6 != 0)
            {
                if (!SplitTwoResultObserved)
                {
                    SplitTwoResultObserved = true;
                    LocalMultiplayerInput.SignalStage("multiplayer_results");
                    Console.Error.WriteLine($"[LocalCoopResult] mode={m.ReadU8(c.GP+0x31)} object={c.S6:X8}");
                }
            }
            else
            {
                SplitTwoResultObserved = false;
                if (m.ReadU32(c.GP + 0x28) >= 300 && m.ReadU32(c.GP+0xC74) == 0)
                {
                    var enemies = new List<uint>();
                    uint node = m.ReadU32(c.GP+0x1040);
                    for (int visits=0; node!=0 && m.ReadU32(node)!=0; visits++)
                    {
                        if (visits>=4096) throw new InvalidOperationException("Invalid fixture object list");
                        uint obj=m.ReadU32(node+8);
                        if (obj!=0 && m.ReadU32(obj)==0x8003C118 && obj!=m.ReadU32(0x8006B7E8) && obj!=m.ReadU32(0x8006B7EC) && m.ReadU16(obj+0x1C)!=0) enemies.Add(obj);
                        node=m.ReadU32(node);
                    }
                    var saved=c.Snapshot();
                    try
                    {
                        c.SP -= 0x100;
                        foreach (uint enemy in enemies)
                        {
                            c.A0 = enemy; c.A1 = unchecked((uint)-0x7FFF);
                            c.A2 = enemy + 0x34; c.A3 = 1;
                            func_80039DCC(c, m);
                        }
                    }
                    finally { c.Restore(saved); }
                    if (enemies.Count>0) Console.Error.WriteLine($"[LocalCoopFixture] destroyed enemies={enemies.Count}");
                }
            }
            return;
        }
        // Test-only trigger/observation for the unchanged retail two-player
        // result path. Read its real human pointers and scores on each round.
        if (LocalInputSession.PlayerCount == 2 && m.ReadU8(c.GP + 0x31) == 5 &&
            int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_RESULT_TEST"), out int twoWinner) && twoWinner is 1 or 2)
        {
            if (m.ReadU32(c.GP + 0xC74) == 0)
            {
                SplitTwoResultObserved = false;
                uint loser = m.ReadU32(0x8006B7E8u + (uint)(2-twoWinner)*4);
                if (m.ReadU32(c.GP + 0x28) >= 180 && loser != 0 && m.ReadU16(loser + 0x1C) != 0)
                {
                    var registers = c.Snapshot();
                    try { c.SP -= 0x100; c.A0 = loser; func_80038870(c, m); }
                    finally { c.Restore(registers); }
                    Console.Error.WriteLine($"[LocalResultFixture] native2 winner={twoWinner}");
                }
            }
            else if (c.S6 != 0 && !SplitTwoResultObserved)
            {
                SplitTwoResultObserved = true;
                LocalMultiplayerInput.SignalStage("multiplayer_results");
                Console.Error.WriteLine($"[LocalResult] native2 object={c.S6:X8} scores={m.ReadU32(c.GP+0xCB8)},{m.ReadU32(c.GP+0xCBC)}");
            }
            return;
        }
        if (!SplitVersus(c, m) || SplitVehicles[0] == 0 || m.ReadU32(c.GP + 0xC74) != 0) return;
        uint tick = m.ReadU32(c.GP + 0x28);
        if (int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_RESULT_TEST"), out int fixtureWinner)
            && fixtureWinner >= 0 && fixtureWinner <= SplitPlayers && tick >= 180)
        {
            var saved = c.Snapshot();
            try
            {
                c.SP -= 0x100;
                for (int player = 0; player < SplitPlayers; player++)
                    if (player + 1 != fixtureWinner && (tick >= 240 || player == 0) && m.ReadU16(SplitVehicles[player] + 0x1C) != 0)
                    {
                        c.A0 = SplitVehicles[player]; func_80038870(c, m);
                        Console.Error.WriteLine($"[LocalResultFixture] destroyed player={player+1} tick={tick}");
                    }
            }
            finally { c.Restore(saved); }
        }
        int alive = 0, winner = -1;
        for (int player = 0; player < SplitPlayers; player++)
            if (m.ReadU16(SplitVehicles[player] + 0x1C) != 0) { alive++; winner = player; }
        if (alive > 1) return;
        m.WriteU32(c.GP + 0xC74, 1);
        Console.Error.WriteLine($"[LocalResult] ended humans={SplitPlayers} alive={alive} winner={winner+1}");
    }

    static void SplitResultDrawText(CpuContext c, IMemory m)
    {
        if (!SplitVersus(c, m) || c.S2 < SplitPlayers) return;
        byte[] text = System.Text.Encoding.Latin1.GetBytes("\u0001 ` \b \u0007`\u0005DRAW!\n\0");
        for (uint i = 0; i < text.Length; i++) m.WriteU8(c.SP + 0x20u + i, text[i]);
    }
    static uint SplitNativeFooterScratch;
    static void SplitResultFooter(CpuContext c, IMemory m)
    {
        if (LocalInputSession.PlayerCount != 2 && !SplitVersus(c, m)) return;
        LocalMultiplayerInput.MenuContext();
        var original = new System.Text.StringBuilder();
        for (uint i = 0; i < 128 && m.ReadU8(c.A1 + i) != 0; i++) original.Append((char)m.ReadU8(c.A1+i));
        string footer = InputPromptLabels.Format(original.ToString(), ConfigManager.Game, 0,
            LocalMultiplayerInput.UsesGamepad(0), true, true);
        byte[] bytes = System.Text.Encoding.Latin1.GetBytes(footer + '\0');
        if (bytes.Length > 192) throw new InvalidOperationException("Result footer exceeds native scratch capacity");
        uint scratch = SplitStorage + 0xF40;
        if (LocalInputSession.PlayerCount == 2)
        {
            var saved = c.Snapshot();
            c.A0 = 192; V82Compat.PcMalloc(c, m); scratch = c.V0;
            c.Restore(saved);
            if (scratch == 0) throw new OutOfMemoryException("Native result footer");
            SplitNativeFooterScratch = scratch;
        }
        for (uint i = 0; i < bytes.Length; i++) m.WriteU8(scratch + i, bytes[i]);
        c.A1 = scratch;
    }
    static int SplitPlayers => SplitHarnessPlayers != 0 ? SplitHarnessPlayers : SplitMenuPlayers;

    static bool SplitCreateResultPackets(CpuContext c, IMemory m)
    {
        // Native strcat has copied the footer into its result text by here.
        if (SplitNativeFooterScratch != 0)
        {
            var saved = c.Snapshot();
            c.A0 = SplitNativeFooterScratch; V82Compat.PcFree(c, m);
            SplitNativeFooterScratch = 0; c.Restore(saved);
        }
        bool nativeTwo = LocalInputSession.PlayerCount == 2 && m.ReadU8(c.GP + 0x31) == 5;
        if (!nativeTwo && !SplitVersus(c, m)) return false;
        int players = nativeTwo ? 2 : SplitPlayers;
        uint font = c.A0, source = c.A1;
        int winner = (int)c.S2;
        var plain = new System.Text.StringBuilder();
        for (uint i = 0; i < 319; i++)
        {
            byte value = m.ReadU8(source + i);
            if (value == 0) break;
            if (value == 10) plain.Append('\n');
            else if (value < 32) i += value switch { 1 => 3u, 2 or 3 or 4 or 7 or 8 => 1u, 14 => 2u, _ => 0u };
            else plain.Append((char)value);
        }
        var registers = c.Snapshot();
        uint buffer = 0, packets = 0;
        try
        {
            c.SP -= 0x100;
            c.A0 = 2048; V82Compat.PcMalloc(c, m); buffer = c.V0;
            if (buffer == 0) throw new OutOfMemoryException("Result layout text");
            void Write(string value)
            {
                byte[] bytes = System.Text.Encoding.Latin1.GetBytes(value + '\0');
                if (bytes.Length > 2048) throw new InvalidOperationException("Result layout exceeds scratch buffer");
                for (uint i = 0; i < bytes.Length; i++) m.WriteU8(buffer + i, bytes[i]);
            }
            int Width(string value)
            {
                Write(value); var saved = c.Snapshot();
                c.A0 = font; c.A1 = buffer; func_80019614(c, m);
                int width = (int)c.V0; c.Restore(saved); return width;
            }
            var layout = new System.Text.StringBuilder("\u000f");
            void Text(string value, int x, int y, bool gold = false)
            {
                if (x < 32 || x > 287) throw new InvalidOperationException($"Result text position outside native layout: {x}");
                layout.Append('\u0007').Append((char)(x-32)).Append('\b').Append((char)y)
                    .Append(gold ? "\u0001\u0080\u0080\u0001" : "\u0001\u0080\u0080\u0080").Append(value).Append('\n');
            }
            void Center(string value, int center, int y, bool gold = false) => Text(value, center - Width(value)/2, y, gold);
            Center(winner < players ? $"PLAYER {winner+1} WINS!" : "DRAW!", 160, 24, true);
            if (winner < players)
            {
                int y = 56;
                foreach (string label in new[] { "Completion time:", "Whammies:", "Cars Totaled:" })
                {
                    string? line = plain.ToString().Split('\n').FirstOrDefault(line => line.StartsWith(label, StringComparison.Ordinal));
                    if (line == null) throw new InvalidOperationException($"Native results omitted {label}");
                    string value = line[label.Length..].Trim();
                    Text(label, 40, y); Text(value, 280 - Width(value), y); y += 20;
                }
            }
            for (int player = 0; player < players; player++)
            {
                int center = players == 3 && player == 2 ? 160 : player % 2 == 0 ? 96 : 224;
                int y = player < 2 ? 126 : 170;
                Center($"PLAYER {player+1}", center, y, true);
                uint score = nativeTwo ? c.GP + 0xCB8u + (uint)player*4 : SplitStorage + 0xF00u + (uint)player*4;
                Center(m.ReadU32(score).ToString(), center, y+18);
            }
            string button(ushort action) => InputPromptLabels.Button(ConfigManager.Game, 0, action, LocalMultiplayerInput.UsesGamepad(0), true, true);
            Center($"{button(Controller.Circle)} restart    {button(Controller.Cross)} quit", 160, 220);
            Write(layout.ToString());
            c.A0 = font; c.A1 = buffer; c.A2 = 32; c.A3 = 0;
            func_800198B8(c, m); packets = c.V0;
            Console.Error.WriteLine($"[LocalResultLayout] players={players} winner={(winner < players ? winner+1 : 0)} scoreColumns={players}");
        }
        finally
        {
            if (buffer != 0) { c.A0 = buffer; V82Compat.PcFree(c, m); }
            c.Restore(registers); c.V0 = packets;
        }
        return true;
    }

    static void SplitAllocate(CpuContext c, IMemory m)
    {
        if (SplitStorage != 0) return;
        var saved = c.Snapshot();
        c.A0 = 4096;
        V82Compat.PcMalloc(c, m);
        SplitStorage = c.V0;
        c.Restore(saved);
        if (SplitStorage == 0) throw new OutOfMemoryException("local multiplayer state");
        for (uint i = 0; i < 4096; i++) m.WriteU8(SplitStorage + i, 0);
    }

    static void SplitMenuArm(CpuContext c, IMemory m, int players, uint excluded)
    {
        var saved = c.Snapshot();
        try
        {
            c.SP -= 0x100;
            SplitMenuPlayers = players;
            SplitAllocate(c, m);
            var occupied = Enumerable.Range(0, players)
                .Select(p => (int)m.ReadU8(c.GP + 0x1104u + (uint)p)).ToHashSet();
            for (int slot = players; slot < 6; slot++)
            {
                // Retail shell random car selection: 18 * RNG / 32768,
                // reject locked and already selected types.
                int type;
                do
                {
                    func_8002AC5C(c, m);
                    type = (int)(c.V0 * 18 >> 15);
                } while ((excluded & (1u << type)) != 0 || occupied.Contains(type));
                occupied.Add(type);
                m.WriteU8(c.GP + 0x1104u + (uint)slot, (byte)type);
            }
            for (uint slot = 0; slot < 6; slot++)
            {
                m.WriteU8(c.GP + 0x1128u + slot, (byte)slot);
                m.WriteU8(0x8006B802u + slot, 1);
            }
            Console.Error.WriteLine($"[LocalMatch] selected humans={players} bots={6-players} types={string.Join(',', Enumerable.Range(0, 6).Select(p => m.ReadU8(c.GP + 0x1104u + (uint)p)))}");
        }
        finally { c.Restore(saved); }
    }

    static void SplitRelease(CpuContext c, IMemory m)
    {
        if (SplitStorage != 0)
        {
            if (SplitMenuPlayers >= 2)
                SplitRematchRoster = Enumerable.Range(0, 6).Select(i => m.ReadU8(c.GP + 0x1104u + (uint)i)).ToArray();
            if (SplitMenuPlayers >= 3)
            {
                // Native SP+6C decides restart after teardown. Retain value
                // data only; never retain vehicle, camera or allocator pointers.
                SplitRematchProfile = Enumerable.Range(0, 360).Select(i => m.ReadU8(SplitStorage + 0x800u + (uint)i)).ToArray();
                SplitRematchScores = Enumerable.Range(0, 4).Select(i => m.ReadU32(SplitStorage + 0xF00u + (uint)i*4)).ToArray();
            }
            var saved = c.Snapshot();
            c.A0 = SplitStorage;
            V82Compat.PcFree(c, m);
            c.Restore(saved);
        }
        SplitStorage = 0;
        SplitMenuPlayers = 0;
        Array.Clear(SplitVehicles);
        SplitLastTrace = uint.MaxValue;
        SplitLastPauseState = 0;
    }

    static uint SplitRumbleAddress(CpuContext c, uint native) => SplitPlayers >= 3 && SplitStorage != 0
        && native >= c.GP + 0xD38u && native < c.GP + 0xD48u
            ? SplitStorage + 0xA0u + native - (c.GP + 0xD38u) : native;

    // Native bank slots 40/41 are per-player overrides; 42 onward are level
    // objects. Extra players start with empty overrides and use their normal
    // vehicle bank through the original factory fallback.
    static uint SplitPlayerModelAddress(uint native) => SplitPlayers >= 3 && SplitStorage != 0
        && native >= 0x800C6220u && native < 0x800C6228u
            ? SplitStorage + 0x60u + native - 0x800C6220u : native;

    static uint SplitMatrixAddress(uint native) => SplitPlayers >= 3 && SplitStorage != 0
        && native >= 0x800BE030u && native < 0x800BE070u
            ? SplitStorage + 0x540u + native - 0x800BE030u : native;

    static uint SplitUpgradeAddress(uint native) => SplitPlayers >= 3 && SplitStorage != 0
        && native >= 0x8006BB50u && native < 0x8006BCB8u
            ? SplitStorage + 0x800u + native - 0x8006BB50u : native;

    static uint SplitCameraAddress(uint native) => SplitPlayers >= 3 && SplitStorage != 0
        && native >= 0x8006B8E0u && native < 0x8006B8E8u
            ? SplitStorage + 0x70u + native - 0x8006B8E0u : native;

    static uint SplitInputAddress(uint native)
    {
        if (SplitPlayers == 0 || SplitStorage == 0 || native < 0x800B4A98u || native >= 0x800B4AC8u)
            return native;
        return SplitStorage + native - 0x800B4A98u;
    }

    static void SplitHarnessPrepare(CpuContext c, IMemory m)
    {
        string? initTrace = Environment.GetEnvironmentVariable("RECOMPONE_SPLIT_INIT_TRACE");
        if (!string.IsNullOrEmpty(initTrace))
        {
            var data = new Dictionary<string, uint[]>();
            uint[] Read(uint address, int bytes) => Enumerable.Range(0, bytes / 4)
                .Select(i => m.ReadU32(address + (uint)i * 4)).ToArray();
            data["gp"] = Read(c.GP, 0x1300);
            data["registers"] = new[] { c.S0, c.S1, c.S2, c.S3, c.S4, c.S5, c.S6, c.S7, c.SP, c.GP };
            for (int p = 0; p < 2; p++)
            {
                uint vehicle = m.ReadU32(0x8006B7E8u + (uint)p * 4);
                if (vehicle == 0) continue;
                data[$"player{p+1}"] = Read(vehicle, 0x300);
                uint camera = m.ReadU32(vehicle + 0xEC);
                if (camera != 0) data[$"camera{p+1}"] = Read(camera, 0x90);
            }
            System.IO.File.WriteAllText(initTrace, System.Text.Json.JsonSerializer.Serialize(data));
        }
        if (SplitPlayers == 0) return;
        var registers = c.Snapshot();
        try
        {
            c.SP -= 0x100;
            SplitVehicles[0] = m.ReadU32(0x8006B7E8);
            SplitVehicles[1] = m.ReadU32(0x8006B7EC);
            SplitAllocate(c, m);
            // LOAD's native color pass covers only its first two humans.
            // Extend the same color/bank calls before constructing P3/P4.
            for (int player = 2; player < SplitMenuPlayers; player++)
            {
                uint type = m.ReadU8(c.GP + 0x1104u + (uint)player);
                if (type >= 18) continue; // Registry-owned banks apply their saved paint.
                c.A0 = (uint)player; c.A1 = type; func_800365E0(c, m);
                uint color = c.V0;
                if (color == 0) continue;
                c.A0 = (uint)player; c.A1 = type; func_80036558(c, m);
                if (c.V0 != 0) type += 21;
                c.A0 = m.ReadU32(0x800C6178u + type * 4);
                c.A1 = color | 0x80000000u;
                func_8001EF34(c, m);
            }
            for (int slot = 2; slot < 6; slot++)
            {
                int id = slot < SplitPlayers ? -slot - 1 : slot - 1;
                c.A0 = unchecked((uint)id);
                func_8003208C(c, m);
                uint vehicle = c.V0;
                if (vehicle == 0) throw new InvalidOperationException($"No native spawn for participant {slot+1}, id={id}");
                SplitVehicles[slot] = vehicle;
                c.A0 = vehicle; func_8003066C(c, m);
                if (slot < SplitPlayers)
                {
                    c.A0 = vehicle;
                    func_80103D94(c, m);
                }
            }
            for (uint player = 0; player < 4; player++)
                m.WriteU32(SplitStorage + 0xF20u + player * 4, player < SplitPlayers ? SplitVehicles[player] : 0);
            SplitFixtureAwardAddress = 0;
            if (SplitRestoringPlayers && SplitRematchPlayerRecords != null)
            {
                for (int player = 0; player < SplitPlayers; player++)
                {
                    uint address = SplitPlayerRecord(c, m, player);
                    for (uint i = 0; i < 10; i++) m.WriteU8(address + i, SplitRematchPlayerRecords[player][i]);
                }
            }
            SplitRestoringPlayers = false;
            if (int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_RESULT_TEST"), out int awardWinner) && awardWinner >= 3 && awardWinner <= SplitPlayers)
            {
                uint player = (uint)(awardWinner - 1), type = m.ReadU8(c.GP + 0x1104u + player);
                uint profile = SplitPlayerRecord(c, m, (int)player);
                SplitFixtureAwardAddress = profile + 6;
                byte before = m.ReadU8(SplitFixtureAwardAddress);
                SplitFixtureAwardExpected = (byte)Math.Min(100, before + 3);
                m.WriteU8(SplitAwardAddress(c, c.GP + 0xC88u + player * 4), 3);
                Console.Error.WriteLine($"[LocalResultFixture] award player={awardWinner} before={before} expected={SplitFixtureAwardExpected}");
            }
            for (int slot = 0; slot < 6; slot++)
            {
                uint vehicle = SplitVehicles[slot];
                Console.Error.WriteLine($"[V82SplitRoster] slot={slot+1} human={slot < SplitPlayers} vehicle={vehicle:X8} id={(short)m.ReadU16(vehicle+10)} camera={m.ReadU32(vehicle+0xEC):X8}");
            }
            var active = new HashSet<uint>();
            uint node = m.ReadU32(c.GP + 0x1040);
            for (int visits = 0; node != 0 && m.ReadU32(node) != 0; visits++)
            {
                if (visits >= 4096) throw new InvalidOperationException("Invalid native active-object list");
                uint obj = m.ReadU32(node + 8);
                if (obj != 0 && m.ReadU32(obj) == 0x8003C118u) active.Add(obj);
                node = m.ReadU32(node);
            }
            if (active.Count != 6 || SplitVehicles.Distinct().Count() != 6 || SplitVehicles.Any(v => !active.Contains(v)))
                throw new InvalidOperationException($"Expected exactly six active combatants, found {active.Count}");
            Console.Error.WriteLine($"[V82SplitVerified] combatants={active.Count} humans={SplitPlayers} bots={6-SplitPlayers}");
            if (SplitMenuPlayers != 0) LocalMultiplayerInput.SignalStage("multiplayer_match");
            // GP1(03h) is the retail SetDispMask(1) operation, normally issued
            // by the skipped title/selection flow.
            RecompOne.Runtime.Runtime.Gpu?.WriteGp1(0x03000000u);
        }
        finally { c.Restore(registers); }
    }

    static void SplitHarnessInput(CpuContext c, IMemory m)
    {
        if (SplitPlayers == 0 || SplitStorage == 0) return;
        var registers = c.Snapshot();
        uint tick = m.ReadU32(c.GP + 0x28);
        try
        {
            c.SP -= 0x100;
            // Optional visual fixture uses the retail pickup/attachment path.
            if (tick == 30 && Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_HUD_FIXTURE") == "1")
                for (int player = 0; player < SplitPlayers; player++)
                {
                    uint vehicle = SplitVehicles[player];
                    c.A0 = vehicle; c.A1 = 1u << (25 + player);
                    func_8003A500(c, m);
                    m.WriteU8(vehicle + 0xAE, 0);
                    uint weapon = m.ReadU32(vehicle + 0x120);
                    if (weapon == 0) throw new InvalidOperationException("HUD fixture pickup failed");
                    m.WriteU16(weapon + 0x1C, (ushort)(player == 0 ? 7 : player == 1 ? 12 : player == 2 ? 99 : 100));
                    Console.Error.WriteLine($"[V82SplitHudFixture] player={player+1} kind={m.ReadU8(weapon+9)} ammo={m.ReadU16(weapon+0x1C)}");
                }
            for (int player = 2; player < SplitPlayers; player++)
            {
                uint state = SplitStorage + (uint)(player-2)*24;
                // Native digital-pad bit order after func_80015540's byte swap.
                uint buttons;
                if (SplitHarnessPlayers != 0)
                {
                    uint phase = (tick + (uint)player * 40) % 240;
                    buttons = tick < 20 ? 0u : 0x40u;
                    if (tick >= 20 && phase < 10) buttons |= player == 2 ? 0x8000u : 0x2000u;
                }
                else
                {
                    uint raw = Controller.LocalConnected[player] ? (ushort)~Controller.GetState(player) : 0u;
                    buttons = ((raw & 255) << 8) | (raw >> 8);
                }
                uint old = m.ReadU32(state + 12) & 0xFFFF;
                uint edges = buttons | ((buttons & ~old) << 16);
                ushort controllerType = (ushort)(SplitHarnessPlayers != 0 || Controller.LocalConnected[player] ? 2 : 0);
                if (m.ReadU16(state) != controllerType)
                    Console.Error.WriteLine($"[LocalController] player={player+1} connected={controllerType != 0}");
                m.WriteU16(state, controllerType);
                m.WriteU32(state + 12, edges);
                c.A0 = edges & 0xF7FFF7FFu;
                c.A2 = m.ReadU32(0x800637EC); c.A3 = m.ReadU32(0x800637F0);
                func_800150B4(c, m);
                m.WriteU32(state + 8, c.V0);
                uint pending = SplitStorage + 0xE00u + (uint)player * 4;
                // Native GP+D18 menu input keeps the raw pad edges. The
                // vehicle mapping deliberately removes Start and remaps fire.
                m.WriteU32(pending, m.ReadU32(pending) | edges);
                m.WriteU32(state + 16, 0x80808080);
                if (tick == 30) Console.Error.WriteLine($"[V82SplitInput] player={player+1} type={m.ReadU16(state)} raw={edges:X8} mapped={c.V0:X8}");
            }
        }
        finally { c.Restore(registers); }
    }

    static bool SplitHarnessRender(CpuContext c, IMemory m)
    {
        if (SplitPlayers < 3 || SplitStorage == 0) return false;
        uint pauseState = c.S4;
        uint resultObject = c.S6;
        if (resultObject != 0) LocalMultiplayerInput.MenuContext();
        uint frameSteps = m.ReadU32(c.SP + 0x18);
        bool pausedFrame = pauseState != 0;
        var registers = c.Snapshot();
        uint frame = m.ReadU32(c.GP + 0x24) & 1;
        V82Compat.BeginSharedTerrainFrame(SplitPlayers == 4);
        bool pipelinedFeedback = SplitPlayers == 4 && V82Compat.BeginPipelinedFeedback();
        void FinishFeedback(int player)
        {
            // Only CPU preparation overlaps the prior view's transfer. Keep
            // GPU world/effect/HUD order identical, including cross-view reads.
            var saved = c.Snapshot();
            uint env = SplitStorage+0x100u+(uint)player*0x5C;
            uint hud = SplitStorage+0x80u+(uint)player*4;
            m.WriteU32(hud,0xFFFFFF);
            c.A0=env; func_8005C5EC(c,m);
            V82Compat.CompleteFeedbackView(c,m,player,hud);
            SplitDrawMinimalHud(c,m,SplitVehicles[player],hud,player);
            bool previousHud = GpuHle.ViewportHudActive;
            try { GpuHle.ViewportHudActive=true; c.A0=hud; func_8005C57C(c,m); }
            finally { GpuHle.ViewportHudActive=previousHud; c.Restore(saved); }
        }
        try
        {
            // Each native call gets its own argument/save area below the main
            // loop's stack locals; no generated callee may overwrite them.
            c.SP -= 0x100;
            for (int player = 0; player < SplitPlayers; player++)
            {
                c.A0 = 0; func_8005BF88(c, m);
                c.A0 = (uint)player & 1; func_80014B3C(c, m);
                uint env = SplitStorage + 0x100u + (uint)player * 0x5C;
                uint x = (uint)(player % 2) * 160, y = frame*240 + (uint)(player/2)*120;
                c.A0 = env; c.A1 = x; c.A2 = y; c.A3 = 160;
                m.WriteU32(c.SP+0x10, 120); func_8005EB6C(c, m);
                c.A0 = env; func_8005C5EC(c, m);
                c.A0 = 160; c.A1 = 120; c.A2 = 80; c.A3 = 60;
                func_8002DF30(c, m);
                uint vehicle = SplitVehicles[player];
                uint camera = m.ReadU32(vehicle + 0xEC);
                c.A0 = camera; c.A1 = (uint)m.ReadU16(camera + 0x1C) / 2;
                func_8002D278(c, m);
                uint matrix = SplitStorage + 0x500u + (uint)player * 32;
                for (uint i = 0; i < 32; i += 4) m.WriteU32(matrix+i, m.ReadU32(c.GP+0xF00+i));
                if (player < 2)
                    for (uint i = 0; i < 32; i += 4) m.WriteU32(0x800BDFF0u+(uint)player*32+i, m.ReadU32(matrix+i));
                func_80031678(c, m);
                if (pipelinedFeedback && player > 0)
                {
                    FinishFeedback(player-1);
                    c.A0=env; func_8005C5EC(c,m);
                }
                uint world = m.ReadU32(c.GP + 0xCE0) + 0x3FFC;
                uint hud = SplitStorage + 0x80u + (uint)player*4;
                m.WriteU32(hud, 0xFFFFFF);
                c.A0 = world; func_8005C57C(c, m);
                c.A0 = 0; func_8005BF88(c, m);
                m.WriteU32(hud, 0xFFFFFF);
                c.A0 = x; c.A1 = y; c.A2 = hud;
                if (pipelinedFeedback)
                {
                    V82Compat.QueueCurrentFeedback(c, m);
                    continue;
                }
                V82Compat.ProcessCurrentExpandedEdgePool(c, m);
                SplitDrawMinimalHud(c, m, vehicle, hud, player);
                c.A0 = env; func_8005C5EC(c, m);
                bool previousHud = GpuHle.ViewportHudActive;
                try
                {
                    GpuHle.ViewportHudActive = true;
                    c.A0 = hud; func_8005C57C(c, m);
                }
                finally { GpuHle.ViewportHudActive = previousHud; }
            }
            if (pipelinedFeedback) FinishFeedback(3);
            if (SplitPlayers == 3)
            {
                // No camera redraws the fourth quadrant after a full-screen
                // modal. Clear it on both display buffers before overlays.
                uint empty = SplitStorage + 0xE50u, env = SplitStorage + 0xE80u;
                c.A0 = env; c.A1 = 160; c.A2 = frame * 240 + 120; c.A3 = 160;
                m.WriteU32(c.SP + 0x10, 120); func_8005EB6C(c, m);
                c.A0 = env; func_8005C5EC(c, m);
                // Native 8001ADF8's flat TILE packet. Draw it, rather than
                // clearing VRAM only, so the enhanced display is cleared too.
                m.WriteU32(empty, 0x03FFFFFF); m.WriteU32(empty + 4, 0x60000000);
                m.WriteU32(empty + 8, 0); m.WriteU32(empty + 12, (120u << 16) | 160u);
                bool previousHud = GpuHle.ViewportHudActive;
                try { GpuHle.ViewportHudActive = true; c.A0 = empty; func_8005C520(c, m); }
                finally { GpuHle.ViewportHudActive = previousHud; }
            }
            {
                uint overlay = SplitStorage + 0xE40u, env = SplitStorage + 0xE80u;
                m.WriteU32(overlay, 0xFFFFFF);
                c.A0 = env; c.A1 = 0; c.A2 = frame * 240; c.A3 = 320;
                m.WriteU32(c.SP + 0x10, 240); func_8005EB6C(c, m);
                c.A0 = env; func_8005C5EC(c, m);
                if (pauseState == 0)
                {
                    if (resultObject != 0)
                    {
                        c.A0 = resultObject; c.A1 = overlay; func_800191E4(c, m);
                    }
                    else
                    {
                        // Native 80013FC8 drains all three message queues before
                        // constructing the result object owned by main's S6.
                        c.A0 = 0x8006BB90; c.A1 = m.ReadU32(c.GP + 0xC78);
                        c.A2 = overlay; c.A3 = frameSteps; func_8001A864(c, m);
                        if (c.V0 == 0 && m.ReadU32(c.GP + 0xC74) != 0 &&
                            m.ReadU16(0x8006BB90) == 0 && m.ReadU16(0x8006BC98) == 0 && m.ReadU16(0x8006BDA0) == 0)
                        {
                            c.A0 = m.ReadU32(c.GP + 0xC78); func_80012930(c, m);
                            resultObject = c.V0;
                            if (SplitFixtureAwardAddress != 0)
                            {
                                byte actual = m.ReadU8(SplitFixtureAwardAddress);
                                if (actual != SplitFixtureAwardExpected) throw new InvalidOperationException($"Extra-player award expected {SplitFixtureAwardExpected}, got {actual}");
                                Console.Error.WriteLine($"[LocalResultFixture] award applied={actual}");
                            }
                            LocalMultiplayerInput.SignalStage("multiplayer_results");
                            Console.Error.WriteLine($"[LocalResult] object={resultObject:X8} scores={string.Join(',', Enumerable.Range(0, SplitPlayers).Select(p => m.ReadU32(SplitStorage + 0xF00u + (uint)p*4)))}");
                        }
                    }
                    bool previousOverlayHud = GpuHle.ViewportHudActive;
                    int previousModalHold = GpuHle.NativeModalHold;
                    try
                    {
                        GpuHle.ViewportHudActive = true;
                        // Result packets span the display, so the gameplay HUD
                        // edge/caption heuristics must not move individual glyphs.
                        if (resultObject != 0) GpuHle.NativeModalHold = Math.Max(1, previousModalHold);
                        c.A0 = overlay; func_8005C57C(c, m);
                    }
                    finally { GpuHle.ViewportHudActive = previousOverlayHud; GpuHle.NativeModalHold = previousModalHold; }
                }
            }
            if (pauseState != 0)
            {
                // The native pause routine owns the menu, audio suspension and
                // resume/quit state. Draw it once across the complete display.
                uint overlay = SplitStorage + 0xE40u, env = SplitStorage + 0xE80u;
                m.WriteU32(overlay, 0xFFFFFF);
                c.A0 = env; c.A1 = 0; c.A2 = frame * 240; c.A3 = 320;
                m.WriteU32(c.SP + 0x10, 240); func_8005EB6C(c, m);
                c.A0 = env; func_8005C5EC(c, m);
                c.A0 = m.ReadU32(c.GP + 0xC78); c.A1 = overlay; c.A2 = pauseState;
                func_80011C58(c, m);
                pauseState = c.V0;
                if (pauseState != SplitLastPauseState)
                {
                    Console.Error.WriteLine($"[LocalPause] state={pauseState:X8} owner={pauseState & 7} tick={m.ReadU32(c.GP+0x28)}");
                    SplitLastPauseState = pauseState;
                }
                c.A0 = overlay; func_8005C57C(c, m);
            }
            c.A0 = 0; func_8005BF88(c, m);
            c.A0 = 0x8006C240u + frame*20;
            LibGpu.PutDispEnv(c, m);
            m.WriteU32(c.GP+0x614, 1);
            m.WriteU32(c.GP+0x618, 1);
            // The regular tick loop waits for VBlank; its paused branch does
            // not. The retail DrawSync/display callback supplies that frame
            // there. Our synchronous split display must deliver it explicitly.
            if (pausedFrame) RecompOne.Runtime.Runtime.PresentFrame();
        }
        finally { V82Compat.EndSharedTerrainFrame(); c.Restore(registers); c.S4 = pauseState; c.S6 = resultObject; }
        return true;
    }

    static uint SplitPauseInputAddress(CpuContext c, uint address) => SplitPlayers >= 3 && SplitStorage != 0
        && address >= c.GP + 0xD20u && address < c.GP + 0xD28u
            ? SplitStorage + 0xE18u + address - (c.GP + 0xD20u) : address;

    static void SplitMergeMenuInput(CpuContext c, IMemory m)
    {
        if (SplitPlayers < 3 || SplitStorage == 0) return;
        for (int player = 2; player < SplitPlayers; player++)
        {
            uint pending = SplitStorage + 0xE00u + (uint)player * 4;
            uint buttons = m.ReadU32(pending);
            if (!Controller.LocalConnected[player] && SplitHarnessPlayers == 0 && m.ReadU32(c.GP + 0xD00) == 0)
                buttons |= 0x08000000;
            m.WriteU32(SplitStorage + 0xE10u + (uint)player * 4, buttons);
            c.S1 |= buttons;
            m.WriteU32(pending, 0);
        }
    }

    static void SplitResolvePauseOwner(CpuContext c, IMemory m)
    {
        if (SplitPlayers < 3 || SplitStorage == 0) return;
        // Paused main-loop input jumps directly here, bypassing the normal
        // simulation-tick aggregate. Consume its freshly polled menu image.
        if (m.ReadU32(c.GP + 0xD00) != 0) SplitMergeMenuInput(c, m);
        if (c.S4 != 10) return;
        if (((m.ReadU32(c.GP + 0xD18) | m.ReadU32(c.GP + 0xD1C)) & 0x08000000) != 0) return;
        for (int player = 2; player < SplitPlayers; player++)
            if ((m.ReadU32(SplitStorage + 0xE10u + (uint)player * 4) & 0x08000000) != 0)
            {
                c.S4 = (uint)(9 + player);
                break;
            }
    }

    // Reuse retail HUD textures and font, with only the pill and selected
    // secondary visible. Each viewport owns its sprite copies in scratch RAM.
    static void SplitDrawMinimalHud(CpuContext c, IMemory m, uint vehicle, uint hud, int player)
    {
        uint scratch = SplitStorage + 0xA00u + (uint)player * 0x80;
        uint plate = scratch, fill = scratch + 24, icon = scratch + 48;
        uint buffer = m.ReadU32(c.GP + 0x20) * 24;
        SplitCopyHudSprite(m, 0x800CD49C, plate);
        SplitCopyHudSprite(m, 0x800CD6E0 + buffer, fill);
        ushort height = m.ReadU16(plate + 0x16);
        uint top = 116u - height;
        m.WriteU16(plate + 0xC, 4);
        m.WriteU16(plate + 0xE, (ushort)top);
        uint maximumFill = (uint)height - 7;
        uint health = m.ReadU16(vehicle + 0x1C), maximum = m.ReadU16(vehicle + 0x1E);
        uint front = m.ReadU32(vehicle + 0xF8), rear = m.ReadU32(vehicle + 0xFC);
        if (health != 0 && front != 0 && rear != 0)
            health = (uint)m.ReadU16(front + 0x1C) + m.ReadU16(rear + 0x1C);
        uint filled = maximum == 0 || health == 0 ? 0 : Math.Min(maximumFill, 3 + health * (maximumFill - 3) / maximum);
        if (filled != 0 && (filled > maximumFill / 4 || (m.ReadU32(c.GP + 0x28) & 31) < 20))
        {
            m.WriteU16(fill + 0xC, 8);
            m.WriteU16(fill + 0xE, (ushort)(top + 4 + maximumFill - filled));
            m.WriteU8(fill + 0x11, (byte)(m.ReadU8(c.GP + 0x1164) + maximumFill - filled));
            m.WriteU16(fill + 0x16, (ushort)filled);
            c.A0 = hud; c.A1 = fill; func_8005ECCC(c, m);
        }
        // AddPrim prepends: the pill backing must draw before its colored fill.
        c.A0 = hud; c.A1 = plate; func_8005ECCC(c, m);

        uint selected = m.ReadU8(vehicle + 0xAE);
        uint weapon = m.ReadU32(vehicle + 0x120u + selected * 4);
        if (weapon == 0) return;
        SplitCopyHudSprite(m, 0x800CDF10 + buffer, icon);
        uint descriptor = m.ReadU32(weapon + 0x7C);
        c.A0 = icon; c.A1 = descriptor != 0 ? descriptor : c.GP + 0x1168;
        func_80018F20(c, m);
        if (descriptor == 0)
        {
            m.WriteU16(icon + 0x16, 12);
            m.WriteU8(icon + 0x11, (byte)(m.ReadU8(icon + 0x11) + ((sbyte)m.ReadU8(weapon + 9) - 1) * 12));
        }
        m.WriteU16(icon + 0xC, (ushort)(156 - m.ReadU16(icon + 0x14)));
        m.WriteU16(icon + 0xE, (ushort)(102 - m.ReadU16(icon + 0x16)));
        c.A0 = hud; c.A1 = icon; func_8005ECCC(c, m);
        string ammo = m.ReadU16(weapon + 0x1C).ToString(System.Globalization.CultureInfo.InvariantCulture);
        uint text = scratch + 72;
        for (int i = 0; i <= ammo.Length; i++)
            m.WriteU8(text + (uint)i, i < ammo.Length ? (byte)ammo[i] : (byte)0);
        c.A0 = m.ReadU32(c.GP + 0x1160); c.A1 = text;
        c.A2 = (uint)(156 - ammo.Length * 8); c.A3 = 104;
        m.WriteU32(c.SP + 0x10, hud);
        func_8001A660(c, m);
    }

    static void SplitCopyHudSprite(IMemory m, uint source, uint destination)
    {
        for (uint offset = 0; offset < 24; offset += 4)
            m.WriteU32(destination + offset, m.ReadU32(source + offset));
    }

    static void SplitHarnessTrace(CpuContext c, IMemory m)
    {
        if (SplitPlayers == 0) return;
        uint tick = m.ReadU32(c.GP + 0x28u);
        if (tick / 30 == SplitLastTrace) return;
        SplitLastTrace = tick / 30;
        Console.Error.WriteLine($"[V82SplitState] tick={tick} s3={c.S3:X8} s4={c.S4:X8} s6={c.S6:X8} input={c.S1:X8} p1={m.ReadU32(0x800B4A70):X8} p2={m.ReadU32(0x800B4A88):X8}");
        if (tick % 120 < 2)
            for (int i = 0; i < 6; i++)
            {
                uint v = SplitVehicles[i];
                if (v != 0) Console.Error.WriteLine($"[V82SplitPose] tick={tick} slot={i+1} x={(int)m.ReadU32(v+0x34)} y={(int)m.ReadU32(v+0x38)} z={(int)m.ReadU32(v+0x3C)} callback={m.ReadU32(v):X8} health={m.ReadU16(v+0x1C)} id={(short)m.ReadU16(v+10)}");
                if (v != 0 && i < SplitPlayers)
                {
                    uint camera = m.ReadU32(v+0xEC);
                    Console.Error.WriteLine($"[V82SplitCamera] tick={tick} slot={i+1} camera={camera:X8} owner={m.ReadU32(camera+0x80):X8} flags={m.ReadU32(camera+4):X8} x={(int)m.ReadU32(camera+0x4C)} y={(int)m.ReadU32(camera+0x50)} z={(int)m.ReadU32(camera+0x54)}");
                }
            }
    }

    static bool SplitHarnessLaunch(CpuContext c, IMemory m)
    {
        if (SplitHarnessPlayers == 0) return false;
        if (SplitHarnessStarted)
            throw new InvalidOperationException("Split harness unexpectedly returned to the shell");
        SplitHarnessStarted = true;
        var registers = c.Snapshot();
        c.SP -= 0x100;
        // L800135C8 performs these before opening the shell. In particular,
        // func_8001B608 initializes the terrain's fallback page and page table.
        // Direct entry may skip selection, but must retain native match setup.
        func_8001B608(c, m);
        func_80014894(c, m);
        c.A0 = 0;
        func_800324A0(c, m);
        func_8001532C(c, m); // The retail title normally starts pad polling.
        c.Restore(registers);
        m.WriteU32(0x8006A830, ConfigManager.Game.V82CheatFlags & ~64u);
        m.WriteU8(c.GP + 0x31u, 5); // Native versus rules.
        m.WriteU8(c.GP + 0x32u, 0);
        m.WriteU32(c.GP + 0x2Cu, 2);
        m.WriteU32(c.GP + 0xD0Cu, 0);
        for (uint slot = 0; slot < 6; slot++)
        {
            m.WriteU8(c.GP + 0x1104u + slot, (byte)slot);
            m.WriteU8(c.GP + 0x1128u + slot, (byte)slot);
            m.WriteU8(0x8006B802u + slot, 1);
        }
        string path = Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_MAP")
            ?? "Levels\\Bayou.exp";
        if (path.Length > 55 || path.Contains("..") || Path.IsPathRooted(path))
            throw new InvalidOperationException("Split harness map must be a relative native level path");
        // Files are case-insensitive, but LOAD resolves the arena callback
        // using an exact path-stem/export match. Take its spelling from the
        // native DLL, just as the normal selector's authored path does.
        path = path.Replace('/', '\\');
        var cd = RecompOne.Runtime.Runtime.Cd
            ?? throw new InvalidOperationException("Split harness requires loaded game files");
        string stem = Path.GetFileNameWithoutExtension(path);
        string export = V82ArenaRegistry.ArenaExportName(cd.Fs.ReadFile(Path.ChangeExtension(path, ".dll")), stem);
        path = Path.Combine(Path.GetDirectoryName(path) ?? "", export + ".exp");
        for (int i = 0; i <= path.Length; i++)
            m.WriteU8(c.SP + 0x20u + (uint)i, i < path.Length ? (byte)path[i] : (byte)0);
        Console.Error.WriteLine($"[V82Split] direct-map={path} players={SplitHarnessPlayers} bots={6 - SplitHarnessPlayers}");
        return true;
    }
}
