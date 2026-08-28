using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class LibGpu
{
    static int _rejectedGameplayOtPackets;

    public static void MoveImage(CpuContext c, IMemory m)
    {
        var gpu = Runtime.Gpu;
        uint rect = c.A0;
        int sx = S16(m, rect + 0x00u);
        int sy = S16(m, rect + 0x02u);
        int w = S16(m, rect + 0x04u);
        int h = S16(m, rect + 0x06u);
        if (gpu == null || w <= 0 || h <= 0)
        {
            c.V0 = 0xFFFFFFFFu;
            return;
        }

        gpu.WriteGp0(0x80000000u);
        gpu.WriteGp0(((uint)(ushort)sy << 16) | (ushort)sx);
        gpu.WriteGp0(((uint)(ushort)c.A2 << 16) | (ushort)c.A1);
        gpu.WriteGp0(((uint)(ushort)h << 16) | (ushort)w);
        c.V0 = 0u;
    }

    public static void DrawOTag(CpuContext c, IMemory m)
    {
        var gpu = Runtime.Gpu;
        if (gpu == null) return;

        uint ramAddressMask =
            (Runtime.Mode == RunMode.Devkit
                ? MemoryMap.DevkitRamSize
                : MemoryMap.RetailRamSize) - 4u;
        uint addr = c.A0 & ramAddressMask;
        int bucketCount = CountOrderingTableBuckets(
            m, addr, ramAddressMask);
        int bucketDepth = bucketCount;
        gpu.BeginOrderingTable();
        for (int guard = 0; guard < 0x100000; guard++)
        {
            uint header = m.ReadU32(addr);
            uint count = header >> 24;
            if (count == 0)
                gpu.SetOrderingTableDepth(bucketDepth--);
            gpu.SetOrderingTablePacket(addr, count);
            if (!GpuHle.GameplayActive ||
                IsSafeGameplayOrderingTablePacket(
                    m, addr, count, out string reason))
            {
                for (uint i = 0; i < count; i++)
                {
                    uint source = addr + 4u + i * 4u;
                    gpu.WriteGp0(m.ReadU32(source), source);
                }
            }
            else if (_rejectedGameplayOtPackets++ < 32)
            {
                uint command = count > 0 ? m.ReadU32(addr + 4u) : 0u;
                string words = string.Join(
                    ',',
                    Enumerable.Range(0, (int)Math.Min(count, 16u))
                        .Select(i => $"{m.ReadU32(addr + 4u + (uint)i * 4u):X8}"));
                Console.Error.WriteLine(
                    $"[V82GpuPacket] rejected malformed gameplay OT packet " +
                    $"address=0x{addr:X6} words={count} " +
                    $"command=0x{command:X8} reason={reason} data={words}");
            }
            uint next = header & 0xFFFFFFu;
            if (next == 0xFFFFFFu || (next & 0x800000u) != 0) break;
            addr = next & ramAddressMask;
        }
        gpu.EndOrderingTable();
    }

    static bool IsSafeGameplayOrderingTablePacket(
        IMemory m,
        uint address,
        uint wordCount,
        out string reason)
    {
        uint offset = 0;
        while (offset < wordCount)
        {
            uint command = m.ReadU32(address + 4u + offset * 4u);
            uint opcode = command >> 24;
            if (opcode is >= 0xA0 and <= 0xDF)
            {
                reason = $"VRAM transfer opcode 0x{opcode:X2}";
                return false;
            }

            int length = Gpu.CommandLength(command);
            if (length == Gpu.LenImageLoad)
            {
                reason = "image upload embedded in draw list";
                return false;
            }
            if (length == Gpu.LenPolyline)
            {
                bool terminated = false;
                for (uint i = offset + 1; i < wordCount; i++)
                {
                    uint word = m.ReadU32(address + 4u + i * 4u);
                    if ((word & 0xF000F000u) != 0x50005000u) continue;
                    offset = i + 1;
                    terminated = true;
                    break;
                }
                if (terminated) continue;
                reason = "unterminated polyline";
                return false;
            }
            if (length <= 0 || offset + (uint)length > wordCount)
            {
                reason =
                    $"truncated opcode 0x{opcode:X2} needs={length} " +
                    $"remaining={wordCount - offset}";
                return false;
            }
            offset += (uint)length;
        }

        reason = "";
        return offset == wordCount;
    }

    static int CountOrderingTableBuckets(
        IMemory m, uint start, uint ramAddressMask)
    {
        int buckets = 0;
        uint addr = start;
        for (int guard = 0; guard < 0x100000; guard++)
        {
            uint header = m.ReadU32(addr);
            if ((header >> 24) == 0)
                buckets++;
            uint next = header & 0xFFFFFFu;
            if (next == 0xFFFFFFu || (next & 0x800000u) != 0)
                break;
            addr = next & ramAddressMask;
        }
        return Math.Max(1, buckets);
    }

    public static void DrawSync(CpuContext c, IMemory m) => c.V0 = 0;

    public static void PutDrawEnv(CpuContext c, IMemory m)
    {
        var gpu = Runtime.Gpu;
        if (gpu == null) { c.V0 = c.A0; return; }

        uint env = c.A0;
        short clipX = S16(m, env + 0x00), clipY = S16(m, env + 0x02);
        short clipW = S16(m, env + 0x04), clipH = S16(m, env + 0x06);
        short ofsX = S16(m, env + 0x08), ofsY = S16(m, env + 0x0A);
        short twX = S16(m, env + 0x0C), twY = S16(m, env + 0x0E);
        short twW = S16(m, env + 0x10), twH = S16(m, env + 0x12);
        ushort tpage = m.ReadU16(env + 0x14);
        byte dtd = m.ReadU8(env + 0x16);
        byte dfe = m.ReadU8(env + 0x17);
        byte isbg = m.ReadU8(env + 0x18);
        byte r0 = m.ReadU8(env + 0x19), g0 = m.ReadU8(env + 0x1A), b0 = m.ReadU8(env + 0x1B);

        gpu.WriteGp0(GetCs(clipX, clipY));
        gpu.WriteGp0(GetCe((short)(clipX + clipW - 1), (short)(clipY + clipH - 1)));
        gpu.WriteGp0(GetOfs(ofsX, ofsY));
        gpu.WriteGp0(GetMode(dfe, dtd, tpage));
        gpu.WriteGp0(GetTw(twX, twY, twW, twH));
        gpu.WriteGp0(0xE6000000u);

        if (isbg != 0)
        {
            int margin = GpuHle.WideMargin(clipW);
            int w = Math.Clamp(clipW + margin * 2, 0, VramShadow.Width - 1);
            int h = Math.Clamp((int)clipH, 0, VramShadow.Height - 1);
            int x = clipX - margin - ofsX, y = clipY - ofsY;
            gpu.WriteGp0(0x60000000u | ((uint)b0 << 16) | ((uint)g0 << 8) | r0);
            gpu.WriteGp0(((uint)(ushort)y << 16) | (ushort)x);
            gpu.WriteGp0(((uint)(ushort)h << 16) | (ushort)w);
        }

        c.V0 = c.A0;
    }

    public static void PutDispEnv(CpuContext c, IMemory m)
    {
        var gpu = Runtime.Gpu;
        if (gpu == null) { c.V0 = c.A0; return; }

        uint env = c.A0;
        short dispX = S16(m, env + 0x00), dispY = S16(m, env + 0x02);
        short dispW = S16(m, env + 0x04), dispH = S16(m, env + 0x06);
        short scrX = S16(m, env + 0x08), scrY = S16(m, env + 0x0A);
        short scrW = S16(m, env + 0x0C), scrH = S16(m, env + 0x0E);
        byte isinter = m.ReadU8(env + 0x10);
        byte isrgb24 = m.ReadU8(env + 0x11);
        bool pal = gpu.Pal;

        Log.Gpu(
            $"PutDispEnv caller=0x{c.RA:X8} env=0x{env:X8} " +
            $"disp={dispX},{dispY} {dispW}x{dispH} " +
            $"screen={scrX},{scrY} {scrW}x{scrH} " +
            $"interlace={isinter} rgb24={isrgb24}");

        gpu.WriteGp1(0x05000000u | (((uint)dispY & 0x3FF) << 10) | ((uint)dispX & 0x3FF));

        int hStart = scrX * 10 + 0x260;
        int vStart = scrY + (pal ? 0x13 : 0x10);
        int hEnd = hStart + (scrW != 0 ? scrW * 10 : 2560);
        int vEnd = vStart + (scrH != 0 ? scrH : 240);
        hStart = Math.Clamp(hStart, 500, 3290);
        hEnd = Math.Clamp(hEnd, hStart + 0x50, 3290);
        vStart = Math.Clamp(vStart, 0x10, pal ? 310 : 256);
        vEnd = Math.Clamp(vEnd, vStart + 2, pal ? 312 : 258);
        gpu.WriteGp1(0x06000000u | (((uint)hEnd & 0xFFF) << 12) | ((uint)hStart & 0xFFF));
        gpu.WriteGp1(0x07000000u | (((uint)vEnd & 0x3FF) << 10) | ((uint)vStart & 0x3FF));

        uint mode = 0x08000000u;
        if (pal) mode |= 0x8;
        if (isrgb24 != 0) mode |= 0x10;
        if (isinter != 0) mode |= 0x20;
        if (dispW <= 280) { }
        else if (dispW <= 352) mode |= 1;
        else if (dispW <= 400) mode |= 0x40;
        else if (dispW <= 560) mode |= 2;
        else mode |= 3;
        if (dispH > (pal ? 288 : 256)) mode |= 0x24;
        gpu.WriteGp1(mode);

        GpuHle.NotifyDisplay(dispX, dispY, dispW, dispH);
        c.V0 = c.A0;
    }

    static short S16(IMemory m, uint addr) => (short)m.ReadU16(addr);

    static uint GetCs(short x, short y)
    {
        x = short.Clamp(x, 0, VramShadow.Width - 1);
        y = short.Clamp(y, 0, VramShadow.Height - 1);
        return 0xE3000000u | (((uint)y & 0x3FF) << 10) | ((uint)x & 0x3FF);
    }

    static uint GetCe(short x, short y)
    {
        x = short.Clamp(x, 0, VramShadow.Width - 1);
        y = short.Clamp(y, 0, VramShadow.Height - 1);
        return 0xE4000000u | (((uint)y & 0x3FF) << 10) | ((uint)x & 0x3FF);
    }

    static uint GetOfs(short x, short y)
        => 0xE5000000u | (((uint)y & 0x7FF) << 11) | ((uint)x & 0x7FF);

    static uint GetMode(int dfe, int dtd, ushort tpage)
        => (dtd != 0 ? 0xE1000200u : 0xE1000000u) | (dfe != 0 ? 0x400u : 0u) | ((uint)tpage & 0x9FF);

    static uint GetTw(short x, short y, short w, short h)
    {
        uint c0 = ((uint)x & 0xFF) >> 3;
        uint c1 = ((uint)y & 0xFF) >> 3;
        uint c2 = ((uint)(-w) & 0xFF) >> 3;
        uint c3 = ((uint)(-h) & 0xFF) >> 3;
        return 0xE2000000u | (c1 << 15) | (c0 << 10) | (c3 << 5) | c2;
    }
}
