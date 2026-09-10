using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hle;

namespace Recompiled;

public static partial class Vigilante82PC
{
    // Process-local development entry. Normal menus and single-player do not
    // enter this path. Multiplayer roster allocation always totals six slots.
    static readonly int SplitHarnessPlayers = int.TryParse(
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_PLAYERS"), out int count)
            && count is >= 2 and <= 4 ? count : 0;
    static bool SplitHarnessStarted;
    static uint SplitLastTrace = uint.MaxValue;
    static readonly uint[] SplitVehicles = new uint[6];
    static uint SplitStorage;

    static uint SplitRumbleAddress(CpuContext c, uint native) => SplitHarnessPlayers >= 3 && SplitStorage != 0
        && native >= c.GP + 0xD38u && native < c.GP + 0xD48u
            ? SplitStorage + 0xA0u + native - (c.GP + 0xD38u) : native;

    // Native bank slots 40/41 are per-player overrides; 42 onward are level
    // objects. Extra players start with empty overrides and use their normal
    // vehicle bank through the original factory fallback.
    static uint SplitPlayerModelAddress(uint native) => SplitHarnessPlayers >= 3 && SplitStorage != 0
        && native >= 0x800C6220u && native < 0x800C6228u
            ? SplitStorage + 0x60u + native - 0x800C6220u : native;

    static uint SplitMatrixAddress(uint native) => SplitHarnessPlayers >= 3 && SplitStorage != 0
        && native >= 0x800BE030u && native < 0x800BE070u
            ? SplitStorage + 0x540u + native - 0x800BE030u : native;

    static uint SplitUpgradeAddress(uint native) => SplitHarnessPlayers >= 3 && SplitStorage != 0
        && native >= 0x8006BB50u && native < 0x8006BCB8u
            ? SplitStorage + 0x800u + native - 0x8006BB50u : native;

    static uint SplitCameraAddress(uint native) => SplitHarnessPlayers >= 3 && SplitStorage != 0
        && native >= 0x8006B8E0u && native < 0x8006B8E8u
            ? SplitStorage + 0x70u + native - 0x8006B8E0u : native;

    static uint SplitInputAddress(uint native)
    {
        if (SplitHarnessPlayers == 0 || SplitStorage == 0 || native < 0x800B4A98u || native >= 0x800B4AC8u)
            return native;
        return SplitStorage + native - 0x800B4A98u;
    }

    static void SplitHarnessPrepare(CpuContext c, IMemory m)
    {
        if (SplitHarnessPlayers == 0) return;
        var registers = c.Snapshot();
        try
        {
            c.SP -= 0x100;
            SplitVehicles[0] = m.ReadU32(0x8006B7E8);
            SplitVehicles[1] = m.ReadU32(0x8006B7EC);
            c.A0 = 4096;
            V82Compat.PcMalloc(c, m);
            SplitStorage = c.V0;
            if (SplitStorage == 0) throw new OutOfMemoryException("four-player state");
            for (uint i = 0; i < 4096; i++) m.WriteU8(SplitStorage + i, 0);
            for (int slot = 2; slot < 6; slot++)
            {
                int id = slot < SplitHarnessPlayers ? -slot - 1 : slot - 1;
                c.A0 = unchecked((uint)id);
                func_8003208C(c, m);
                uint vehicle = c.V0;
                if (vehicle == 0) throw new InvalidOperationException($"No native spawn for participant {slot+1}, id={id}");
                SplitVehicles[slot] = vehicle;
                c.A0 = vehicle; func_8003066C(c, m);
                if (slot < SplitHarnessPlayers)
                {
                    c.A0 = vehicle;
                    func_80103D94(c, m);
                }
            }
            for (int slot = 0; slot < 6; slot++)
            {
                uint vehicle = SplitVehicles[slot];
                Console.Error.WriteLine($"[V82SplitRoster] slot={slot+1} human={slot < SplitHarnessPlayers} vehicle={vehicle:X8} id={(short)m.ReadU16(vehicle+10)} camera={m.ReadU32(vehicle+0xEC):X8}");
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
            Console.Error.WriteLine($"[V82SplitVerified] combatants={active.Count} humans={SplitHarnessPlayers} bots={6-SplitHarnessPlayers}");
            // GP1(03h) is the retail SetDispMask(1) operation, normally issued
            // by the skipped title/selection flow.
            RecompOne.Runtime.Runtime.Gpu?.WriteGp1(0x03000000u);
        }
        finally { c.Restore(registers); }
    }

    static void SplitHarnessInput(CpuContext c, IMemory m)
    {
        if (SplitHarnessPlayers == 0 || SplitStorage == 0) return;
        var registers = c.Snapshot();
        uint tick = m.ReadU32(c.GP + 0x28);
        try
        {
            c.SP -= 0x100;
            // Optional visual fixture uses the retail pickup/attachment path.
            if (tick == 30 && Environment.GetEnvironmentVariable("RECOMPONE_V82_SPLIT_HUD_FIXTURE") == "1")
                for (int player = 0; player < SplitHarnessPlayers; player++)
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
            for (int player = 2; player < SplitHarnessPlayers; player++)
            {
                uint state = SplitStorage + (uint)(player-2)*24;
                // Native digital-pad bit order after func_80015540's byte swap.
                uint phase = (tick + (uint)player * 40) % 240;
                uint buttons = tick < 20 ? 0u : 0x40u;
                if (tick >= 20 && phase < 10) buttons |= player == 2 ? 0x8000u : 0x2000u;
                uint old = m.ReadU32(state + 12) & 0xFFFF;
                uint edges = buttons | ((buttons & ~old) << 16);
                m.WriteU16(state, 2); // Native processed digital controller type.
                m.WriteU32(state + 12, edges);
                c.A0 = edges & 0xF7FFF7FFu;
                c.A2 = m.ReadU32(0x800637EC); c.A3 = m.ReadU32(0x800637F0);
                func_800150B4(c, m);
                m.WriteU32(state + 8, c.V0);
                m.WriteU32(state + 16, 0x80808080);
                if (tick == 30) Console.Error.WriteLine($"[V82SplitInput] player={player+1} type={m.ReadU16(state)} raw={edges:X8} mapped={c.V0:X8}");
            }
        }
        finally { c.Restore(registers); }
    }

    static bool SplitHarnessRender(CpuContext c, IMemory m)
    {
        if (SplitHarnessPlayers < 3 || SplitStorage == 0) return false;
        var registers = c.Snapshot();
        uint frame = m.ReadU32(c.GP + 0x24) & 1;
        V82Compat.BeginSharedTerrainFrame(SplitHarnessPlayers == 4);
        bool pipelinedFeedback = SplitHarnessPlayers == 4 && V82Compat.BeginPipelinedFeedback();
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
            for (int player = 0; player < SplitHarnessPlayers; player++)
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
                if (SplitHarnessPlayers != 4)
                {
                    c.A0 = vehicle; c.A1 = 4; c.A2 = hud;
                    func_80037A00(c, m);
                    c.A0 = vehicle; c.A1 = matrix; c.A2 = hud;
                    func_8003827C(c, m);
                }
                if (SplitHarnessPlayers == 4) SplitDrawMinimalHud(c, m, vehicle, hud, player);
                c.A0 = env; func_8005C5EC(c, m);
                bool previousHud = GpuHle.ViewportHudActive;
                try
                {
                    GpuHle.ViewportHudActive = SplitHarnessPlayers == 4;
                    c.A0 = hud; func_8005C57C(c, m);
                }
                finally { GpuHle.ViewportHudActive = previousHud; }
            }
            if (pipelinedFeedback) FinishFeedback(3);
            c.A0 = 0; func_8005BF88(c, m);
            c.A0 = 0x8006C240u + frame*20;
            LibGpu.PutDispEnv(c, m);
            m.WriteU32(c.GP+0x614, 1);
            m.WriteU32(c.GP+0x618, 1);
        }
        finally { V82Compat.EndSharedTerrainFrame(); c.Restore(registers); }
        return true;
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
        if (SplitHarnessPlayers == 0) return;
        uint tick = m.ReadU32(c.GP + 0x28u);
        if (tick / 30 == SplitLastTrace) return;
        SplitLastTrace = tick / 30;
        Console.Error.WriteLine($"[V82SplitState] tick={tick} s3={c.S3:X8} s4={c.S4:X8} s6={c.S6:X8} input={c.S1:X8} p1={m.ReadU32(0x800B4A70):X8} p2={m.ReadU32(0x800B4A88):X8}");
        if (tick % 120 < 2)
            for (int i = 0; i < 6; i++)
            {
                uint v = SplitVehicles[i];
                if (v != 0) Console.Error.WriteLine($"[V82SplitPose] tick={tick} slot={i+1} x={(int)m.ReadU32(v+0x34)} y={(int)m.ReadU32(v+0x38)} z={(int)m.ReadU32(v+0x3C)} callback={m.ReadU32(v):X8} health={m.ReadU16(v+0x1C)} id={(short)m.ReadU16(v+10)}");
                if (v != 0 && i < SplitHarnessPlayers)
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
        for (int i = 0; i <= path.Length; i++)
            m.WriteU8(c.SP + 0x20u + (uint)i, i < path.Length ? (byte)path[i] : (byte)0);
        Console.Error.WriteLine($"[V82Split] direct-map={path} players={SplitHarnessPlayers} bots={6 - SplitHarnessPlayers}");
        return true;
    }
}
