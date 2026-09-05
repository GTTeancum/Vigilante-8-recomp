using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Read-only, explicitly enabled native attachment hierarchy trace.</summary>
internal static class V82WaterAttachmentProbe
{
    static readonly bool Enabled = Environment.GetEnvironmentVariable(
        "RECOMPONE_V82_WATER_ATTACHMENT_TRACE") == "1";

    public static void Dump(CpuContext c, IMemory m, uint vehicle, int frame)
    {
        if (!Enabled || vehicle == 0) return;
        m = Dispatcher.UnwrapMemory(m);
        Console.Error.WriteLine($"[WaterAttachment] frame={frame} vehicle=0x{vehicle:X8} " +
            $"type={m.ReadU8(vehicle+0xDC)} mode={m.ReadU8(vehicle+0xAC)} " +
            $"transition={m.ReadU16(vehicle+0xB4)} body-bank=0x{m.ReadU32(vehicle+0x5C):X8} " +
            $"body-height={I(m,vehicle+0xE4)} world={Vector(m,vehicle+0x34)}");
        for (uint index = 0; index < 6; index++)
        {
            uint node = m.ReadU32(vehicle + 0x104 + index * 4);
            if (node == 0) continue;
            uint parent;
            var saved = c.Snapshot();
            try
            {
                c.A0 = node;
                // +14 is a parent-or-previous-sibling link. Use the native
                // parent walker instead of mistaking a sibling for the root.
                Dispatcher.Call(c, m, 0x8002CD78);
                parent = c.V0;
            }
            finally { c.Restore(saved); }
            Console.Error.WriteLine($"[WaterAttachment] frame={frame} mount={index} " +
                $"node=0x{node:X8} parent=0x{parent:X8} parent-is-vehicle={parent==vehicle} " +
                $"mesh=0x{m.ReadU32(node+0x40):X8} kind={m.ReadU16(node+0xA)} " +
                $"position={Vector(m,node+0x34)} local={Vector(m,node+0x4C)} " +
                $"spring-offset={I(m,node+0x90)} travel={I(m,node+0x84)} " +
                $"attachment-offset={I(m,node+0x78)}");
            DumpParts(m, node, frame, index, 0, new HashSet<uint>());
        }
        if (frame == 150) SaveGeometry(m, vehicle, frame);
    }

    static void SaveGeometry(IMemory m, uint vehicle, int frame)
    {
        string? directory = Environment.GetEnvironmentVariable("RECOMPONE_CAPTURE_DIR");
        if (string.IsNullOrEmpty(directory) || !Directory.Exists(directory)) return;
        var nodes = new List<object>();
        var visited = new HashSet<uint>();
        void Visit(uint node, uint parent, int depth)
        {
            if (node < 0x80010000 || node >= 0x80800000 - 0x80 || depth > 24 ||
                visited.Count >= 256 || !visited.Add(node)) return;
            uint mesh = m.ReadU32(node + 0x40);
            var vertices = new List<int[]>();
            int shift = -1;
            if (mesh >= 0x80010000 && mesh < 0x80800000 - 0x1C)
            {
                shift = m.ReadU8(mesh + 1);
                int count = m.ReadU16(mesh + 4);
                uint source = m.ReadU32(mesh + 8);
                if (count <= 4096 && source >= 0x80010000 && source < 0x80800000 - count * 8)
                    for (uint i = 0; i < count; i++)
                        vertices.Add(new[] { (int)(short)m.ReadU16(source+i*8),
                            (int)(short)m.ReadU16(source+i*8+2), (int)(short)m.ReadU16(source+i*8+4) });
            }
            nodes.Add(new { address=node, parent, bank=m.ReadU32(node+0x5C),
                slot=m.ReadU16(node+0x1A), flags=m.ReadU32(node+4), mesh,
                alternateMesh=m.ReadU32(node+0x68), scaleShift=shift,
                position=new[] { I(m,node+0x34),I(m,node+0x38),I(m,node+0x3C) },
                matrix=Enumerable.Range(0,9).Select(i=>(int)(short)m.ReadU16(node+0x20+(uint)i*2)).ToArray(),
                vertices });
            uint child=m.ReadU32(node+0x10);
            for (int i=0; i<256 && child>=0x80010000 && child<0x80800000-0x80; i++)
            {
                if (visited.Contains(child)) break;
                Visit(child,node,depth+1);
                child=m.ReadU32(child+0xC);
            }
        }
        Visit(vehicle,0,0);
        string path=Path.Combine(directory,$"water_geometry_type{m.ReadU8(vehicle+0xDC)}_{vehicle:X8}_{frame}.json");
        if (File.Exists(path)) return;
        File.WriteAllText(path,System.Text.Json.JsonSerializer.Serialize(new {
            vehicle, frame, type=m.ReadU8(vehicle+0xDC), mode=m.ReadU8(vehicle+0xAC),
            transition=m.ReadU16(vehicle+0xB4),
            mounts=Enumerable.Range(0,6).Select(i=>m.ReadU32(vehicle+0x104+(uint)i*4)).ToArray(), nodes,
            lastRenderFit=V82WaterAttachmentFit.DescribeCached(m,vehicle) }));
        Console.Error.WriteLine($"[WaterAttachment] geometry={path} nodes={nodes.Count}");
    }

    static void DumpParts(IMemory m, uint node, int frame, uint mount, int depth, HashSet<uint> visited)
    {
        if (node < 0x80010000 || node >= 0x80800000 - 0x80 ||
            depth > 16 || visited.Count >= 64 || !visited.Add(node)) return;
        Console.Error.WriteLine($"[WaterAttachmentPart] frame={frame} mount={mount} depth={depth} " +
            $"node=0x{node:X8} bank=0x{m.ReadU32(node+0x5C):X8} slot={m.ReadU16(node+0x1A)} " +
            $"mesh=0x{m.ReadU32(node+0x40):X8} flags=0x{m.ReadU32(node+4):X8} " +
            $"position={Vector(m,node+0x34)} local={Vector(m,node+0x4C)} " +
            $"rotation=({(short)m.ReadU16(node+0x44)},{(short)m.ReadU16(node+0x46)},{(short)m.ReadU16(node+0x48)})");
        uint child = m.ReadU32(node + 0x10);
        for (int count = 0; count < 64 && child >= 0x80010000 && child < 0x80800000 - 0x80; count++)
        {
            if (visited.Contains(child)) break;
            DumpParts(m, child, frame, mount, depth + 1, visited);
            child = m.ReadU32(child + 0xC);
        }
    }

    static int I(IMemory m, uint p) => unchecked((int)m.ReadU32(p));
    static string Vector(IMemory m, uint p) => $"({I(m,p)},{I(m,p+4)},{I(m,p+8)})";
}
