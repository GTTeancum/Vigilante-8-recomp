using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class NativeVramPlacement
{
    // Loading runs on the game thread. Reuse traversal storage between requests.
    static readonly Stack<uint> Pending = new();

    public static uint FindFreeLeaf(CpuContext c, IMemory m, uint root)
    {
        uint width = c.A0, height = c.A1, maskX = c.A2 - 1, maskY = c.A3 - 1;
        // Called after the original allocator creates its 0x38-byte frame.
        uint limitX = m.ReadU32(c.SP + 0x48), limitY = m.ReadU32(c.SP + 0x4C);
        if (width == 0 || height == 0 || width > limitX || height > limitY) return root;
        uint best = root;
        ulong bestScore = ulong.MaxValue;
        Pending.Clear();
        Pending.Push(root);
        while (Pending.Count != 0)
        {
            uint node = Pending.Pop();
            if (node == 0) continue;
            uint kind = m.ReadU32(node + 8);
            if (kind is 2 or 3 or 4)
            {
                Pending.Push(m.ReadU32(node + 0x14));
                Pending.Push(m.ReadU32(node + 0x10));
                continue;
            }
            // Kind 4 pins a split against coalescing; its children can still
            // contain free space. Occupied leaves (kind 1) remain excluded.
            if (kind != 0) continue;
            uint x = m.ReadU16(node), y = m.ReadU16(node + 2);
            uint w = m.ReadU16(node + 4), h = m.ReadU16(node + 6);
            if (width > w || height > h) continue;
            uint dx = (x & maskX) > limitX - width ? unchecked(0u - x) & maskX : 0;
            uint dy = (y & maskY) > limitY - height ? unchecked(0u - y) & maskY : 0;
            if (width + dx > w || height + dy > h) continue;
            uint widthRemainder = w - width - dx, heightRemainder = h - height - dy;
            // CLUTs use the shortest free strips. Images fill columns from
            // left to right, in descending reserved width, preserving tall
            // columns for page-constrained textures loaded later by the level.
            ulong score = height == 1
                ? (ulong)heightRemainder * 1024 + widthRemainder
                : (ulong)x * 1024 + y;
            if (score < bestScore)
            {
                best = node;
                bestScore = score;
            }
        }
        // Only choose the leaf. The original code still aligns, splits, marks,
        // uploads and frees it, and handles a request with no fitting leaf.
        return best;
    }
}
