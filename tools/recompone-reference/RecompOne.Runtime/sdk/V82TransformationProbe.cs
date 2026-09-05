using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Opt-in, process-local native integration fixture; never host input.</summary>
internal static class V82TransformationProbe
{
    public static readonly bool Enabled =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_TRANSFORMATION_PROBE") == "1";
    static readonly bool Capture =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_TRANSFORMATION_PROBE_IMAGES") == "1";
    static readonly bool PowerupProfile =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_TRANSFORMATION_PROBE_PROFILE") == "powerups";
    static readonly bool AttachmentProfile =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_TRANSFORMATION_PROBE_PROFILE") == "attachment";
    static readonly bool InspectionCamera =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_WATER_INSPECTION_CAMERA") == "1";
    static readonly int InspectionYaw = InspectionAngle("YAW",0,-180,180);
    static readonly int InspectionPitch = InspectionAngle("PITCH",-45,-80,-5);
    static readonly int WaterSiteSeparation = InspectionWaterSeparation();
    static int InspectionWaterSeparation()
    {
        return int.TryParse(Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_WATER_SITE_SEPARATION_UNITS"),out int units)
            && units>=0 && units<=128 ? units<<16 : 0;
    }
    static int InspectionAngle(string axis,int fallback,int minimum,int maximum)
    {
        int degrees=int.TryParse(Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_WATER_INSPECTION_"+axis+"_DEGREES"),out int requested)
            && requested>=minimum && requested<=maximum ? requested : fallback;
        return (int)Math.Round(degrees*4096.0/360.0);
    }
    static readonly int RespawnGeneration = int.TryParse(
        Environment.GetEnvironmentVariable("RECOMPONE_V82_TRANSFORMATION_PROBE_RESPAWN_GENERATION"), out int generation) ? generation : 0;
    sealed record Actor(uint Address, bool Player, uint Callback, (int X, int Y, int Z) Land,
        ushort Health, (int X, int Y, int Z) Water);
    static readonly List<Actor> Actors = [];
    static int _generation;
    static bool _recoveryTriggered;
    static bool _waterAssemblyCaptured;

    public static void Reset() => Actors.Clear();

    public static void Tick(CpuContext c, IMemory memory, int frame,
        IEnumerable<uint> vehicles, uint player)
    {
        if (!Enabled) return;
        var m = Dispatcher.UnwrapMemory(memory);
        if (frame == 1)
        {
            Actors.Clear();
            _recoveryTriggered = false;
            _waterAssemblyCaptured = false;
            _generation++;
            Console.Error.WriteLine($"[TransformationProbe] begin generation={_generation} policy={ConfigManager.Game.V82Transformations}");
        }
        if (PowerupProfile)
        {
            if (frame is 60 or 180 or 300 or 420)
            {
                uint mode = frame == 420 ? 0u : (uint)((frame - 60) / 120 + 1);
                foreach (uint v in new[] { player }.Concat(vehicles.Order()).Distinct().Where(v => Valid(m, v)))
                {
                    var state = c.Snapshot();
                    try
                    {
                        c.A0 = v; c.A1 = mode; c.A2 = mode == 0 ? 0u : 500u;
                        // The real native action called by pickups, WITHOUT
                        // automatic-controller authorization or state writes.
                        Dispatcher.Call(c, m, 0x8003E32C);
                        Console.Error.WriteLine($"[TransformationProbe] native-request generation={_generation} frame={frame} actor={(v == player ? "player" : "ai")} " +
                            $"vehicle=0x{v:X8} requested={mode} mode={m.ReadU8(v+0xAC)} transition={m.ReadU16(v+0xB4)} shield={m.ReadU16(v+0x12C)}");
                    }
                    finally { c.Restore(state); }
                }
            }
            if (frame >= 90 && frame <= 540 && frame % 30 == 0)
                Console.Error.WriteLine($"[TransformationProbe] native-settled generation={_generation} frame={frame} mode={m.ReadU8(player+0xAC)} transition={m.ReadU16(player+0xB4)} health={m.ReadU16(player+0x1C)}");
            if (frame is 150 or 270 or 390 or 540)
                V82WaterAttachmentProbe.Dump(c, m, player, frame);
            if (Capture && _generation == 1 && !_waterAssemblyCaptured &&
                frame > 180 && frame < 270 && m.ReadU8(player + 0xAC) == 2 &&
                m.ReadU16(player + 0xB4) == 0)
            {
                _waterAssemblyCaptured = true;
                V82WaterAttachmentProbe.Dump(c, m, player, frame);
                HostWindow.RequestDisplayCapture($"water_attachment_native_all_{frame}");
            }
            return;
        }
        if (frame == 90)
        {
            if (AttachmentProfile && InspectionCamera)
            {
                // Native camera array used by 8001277C. Keep the normal
                // camera callback and target; only widen/elevate this opt-in
                // inspection viewpoint. No vehicle/physics/render changes.
                uint camera=m.ReadU32(0x8006B8D8);
                if (camera>=0x80010000 && camera<0x80800000-0xDC && m.ReadU32(camera+0x80)==player)
                {
                    uint distance=Math.Max(m.ReadU32(camera+0x9C),0x30000u);
                    m.WriteU32(camera+0x9C,distance);
                    // Native 8004B1B0 adds +92 to the target vehicle yaw;
                    // +90 is elevation. The fixture owns only this camera.
                    m.WriteU16(camera+0x90,unchecked((ushort)InspectionPitch));
                    m.WriteU16(camera+0x92,unchecked((ushort)InspectionYaw));
                    Console.Error.WriteLine($"[TransformationProbe] inspection-camera generation={_generation} frame={frame} camera=0x{camera:X8} owner=0x{player:X8} distance={distance} pitch={InspectionPitch} yaw={InspectionYaw}");
                }
                else Console.Error.WriteLine("[TransformationProbe] fixture-failed inspection camera owner mismatch");
            }
            int plane = I32(m, c.GP + 0xDB0), boundary = I32(m, c.GP + 0xDA0);
            int minX = I32(m, c.GP + 0xDE4), maxX = I32(m, c.GP + 0xDE8);
            int minZ = I32(m, c.GP + 0xDEC), maxZ = I32(m, c.GP + 0xDF0);
            Console.Error.WriteLine($"[TransformationProbe] surface plane={plane} boundary={boundary} bounds=({minX},{maxX},{minZ},{maxZ})");
            if (plane == 0 || maxX <= minX || maxZ <= minZ)
            {
                Console.Error.WriteLine("[TransformationProbe] fixture-failed invalid water/arena bounds");
                return;
            }
            var points = new List<(int X, int Z)>();
            var dryPoints = new List<(int X, int Y, int Z)>();
            for (int iz = 1; iz < 64; iz++)
                for (int ix = 1; ix < 64; ix++)
                {
                    int x = (int)(minX + ((long)maxX - minX) * ix / 64);
                    int z = (int)(minZ + ((long)maxZ - minZ) * iz / 64);
                    int height = Height(c, m, x, z);
                    if (z < boundary && height > (long)plane + 0x6000 &&
                        (!AttachmentProfile || ClearWater(c, m, x, z, plane, boundary))) points.Add((x, z));
                    if (height < (long)plane - 0x10000) dryPoints.Add((x, height - 0x4000, z));
                }
            int px = I32(m, player + 0x34), pz = I32(m, player + 0x3C);
            points.Sort((a, b) => Distance(a, px, pz).CompareTo(Distance(b, px, pz)));
            if (AttachmentProfile && WaterSiteSeparation>0 && points.Count>0)
            {
                // Relocate only the fixture player from the crowded nearest
                // water site. Do not hide/hold level creatures or alter their
                // behavior. Every alternative still passed ClearWater above.
                var nearest=points[0];
                points.RemoveAll(p=>Distance(p,nearest.X,nearest.Z)<(long)WaterSiteSeparation*WaterSiteSeparation);
                Console.Error.WriteLine($"[TransformationProbe] alternate-water-site origin=({nearest.X},{nearest.Z}) separation={WaterSiteSeparation} candidates={points.Count}");
            }
            dryPoints.Sort((a, b) => Distance((a.X,a.Z), px, pz).CompareTo(Distance((b.X,b.Z), px, pz)));
            var candidates = AttachmentProfile ? new[] { player } :
                new[] { player }.Concat(vehicles.Order()).Distinct();
            var chosen = new List<(int X, int Z)>();
            var chosenDry = new List<(int X, int Z)>();
            foreach (uint address in candidates)
            {
                if (!Valid(m, address) || Actors.Count == 2) continue;
                int index = points.FindIndex(p => chosen.All(q => Distance(p, q.X, q.Z) > 0x40000L * 0x40000L));
                int dryIndex = dryPoints.FindIndex(p => chosenDry.All(q => Distance((p.X,p.Z), q.X, q.Z) > 0x40000L * 0x40000L));
                if (index < 0 || dryIndex < 0)
                {
                    Console.Error.WriteLine($"[TransformationProbe] fixture-failed no separated water/dry positions water={points.Count} dry={dryPoints.Count}");
                    Actors.Clear();
                    return;
                }
                var point = points[index]; chosen.Add(point);
                var dryPoint = dryPoints[dryIndex]; chosenDry.Add((dryPoint.X,dryPoint.Z));
                var actor = new Actor(address, address == player, m.ReadU32(address), dryPoint,
                    m.ReadU16(address + 0x1C), (point.X, plane - 0x4000, point.Z));
                Actors.Add(actor);
                Put(m, address, point.X, plane - 0x4000, point.Z);
                if (AttachmentProfile && ConfigManager.Game.V82Transformations == 0)
                {
                    var state = c.Snapshot();
                    try
                    {
                        c.A0 = address; c.A1 = 2; c.A2 = 500;
                        Dispatcher.Call(c, m, 0x8003E32C);
                    }
                    finally { c.Restore(state); }
                }
                Console.Error.WriteLine($"[TransformationProbe] water-inject generation={_generation} frame={frame} actor={(actor.Player ? "player" : "ai")} vehicle=0x{address:X8} pos=({point.X},{plane - 0x4000},{point.Z}) health={actor.Health} mode={m.ReadU8(address + 0xAC)}");
            }
            if (Actors.Count != (AttachmentProfile ? 1 : 2) || !Actors.Any(a => a.Player))
                Console.Error.WriteLine("[TransformationProbe] fixture-failed missing player/AI actors");
        }
        if (frame >= 120 && frame <= 300 && !_recoveryTriggered &&
            _generation == RespawnGeneration && Actors.Any(a => a.Player) &&
            m.ReadU8(player + 0xAC) == V82TransformationPolicy.WaterMode &&
            m.ReadU16(player + 0xB4) == 0)
        {
            _recoveryTriggered = true;
            var state = c.Snapshot();
            try
            {
                c.A0 = player;
                // Force the real recovery event ONLY in this explicit test;
                // native code chooses its recovery timing and destination.
                Dispatcher.Call(c, m, 0x800391AC);
                Console.Error.WriteLine($"[TransformationProbe] native-recovery generation={_generation} frame={frame} vehicle=0x{player:X8} callback=0x{m.ReadU32(player):X8} mode={m.ReadU8(player+0xAC)}");
            }
            finally { c.Restore(state); }
        }
        if (frame == 300 && _generation == RespawnGeneration && !_recoveryTriggered)
            Console.Error.WriteLine("[TransformationProbe] fixture-failed no settled water mode for native recovery");
        if (frame == 360 && _generation != RespawnGeneration)
            foreach (var actor in Actors)
            {
                // Place once on terrain confirmed above water; an AI's
                // initial pose is not necessarily dry. No continuous hold.
                Put(m, actor.Address, actor.Land.X, actor.Land.Y, actor.Land.Z);
                Console.Error.WriteLine($"[TransformationProbe] land-inject generation={_generation} vehicle=0x{actor.Address:X8} frame={frame}");
            }
        if (AttachmentProfile && frame == 630)
            foreach (var actor in Actors)
            {
                Put(m, actor.Address, actor.Water.X, actor.Water.Y, actor.Water.Z);
                if (ConfigManager.Game.V82Transformations == 0)
                {
                    var state=c.Snapshot();
                    try { c.A0=actor.Address; c.A1=2; c.A2=500; Dispatcher.Call(c,m,0x8003E32C); }
                    finally { c.Restore(state); }
                }
                Console.Error.WriteLine($"[TransformationProbe] repeat-water-inject generation={_generation} vehicle=0x{actor.Address:X8} frame={frame}");
            }
        if (AttachmentProfile && frame == 870)
            foreach (var actor in Actors)
            {
                Put(m, actor.Address, actor.Land.X, actor.Land.Y, actor.Land.Z);
                Console.Error.WriteLine($"[TransformationProbe] repeat-land-inject generation={_generation} vehicle=0x{actor.Address:X8} frame={frame}");
            }
        if (frame >= 90 && frame <= (AttachmentProfile ? 1050 : 600) && frame % 30 == 0)
            foreach (var actor in Actors)
            {
                uint v = actor.Address;
                Console.Error.WriteLine($"[TransformationProbe] sample generation={_generation} frame={frame} actor={(actor.Player ? "player" : "ai")} vehicle=0x{v:X8} " +
                    $"mode={m.ReadU8(v + 0xAC)} transition={m.ReadU16(v + 0xB4)} health={m.ReadU16(v + 0x1C)} " +
                    $"callback=0x{m.ReadU32(v):X8} initial-callback=0x{actor.Callback:X8} " +
                    $"pos=({I32(m,v+0x34)},{I32(m,v+0x38)},{I32(m,v+0x3C)}) plane={I32(m,c.GP+0xDB0)} " +
                    $"shield={m.ReadU16(v+0x12C)} flying={V82VehicleRegistry.UsesFlyingController(m,v)}");
            }
        if (Capture && AttachmentProfile && _generation == 1 && frame == 150)
        {
            if (m.ReadU8(player + 0xAC) == 2 && m.ReadU16(player + 0xB4) == 0)
                HostWindow.RequestDisplayCapture($"water_attachment_policy{ConfigManager.Game.V82Transformations}_{frame}");
            else
                Console.Error.WriteLine("[TransformationProbe] fixture-failed attachment capture requires assembled water mode");
        }
        if (Capture && !AttachmentProfile && _generation == 1 && frame is 270 or 540)
            HostWindow.RequestDisplayCapture($"auto_waterski_{(frame == 270 ? "water" : "land")}_{frame}");
        if (AttachmentProfile && frame==150)
            foreach (uint other in vehicles.Distinct().Where(v=>v!=player && Valid(m,v)))
                V82WaterAttachmentProbe.Dump(c,m,other,frame);
        if (frame is 90 or 150 or 270 or 540)
            V82WaterAttachmentProbe.Dump(c, m, player, frame);
    }

    static bool Valid(IMemory m, uint p) => p >= 0x80010000 && p < 0x80800000 - 0x200 &&
        m.ReadU8(p + 8) == 2 && m.ReadU16(p + 0x1C) != 0;
    static long Distance((int X, int Z) a, int x, int z) =>
        ((long)a.X - x) * ((long)a.X - x) + ((long)a.Z - z) * ((long)a.Z - z);
    static int I32(IMemory m, uint p) => unchecked((int)m.ReadU32(p));
    static bool ClearWater(CpuContext c, IMemory m, int x, int z, int plane, int boundary)
    {
        // Test fixture only: keep the entire support footprint away from the
        // shore, not just the vehicle center. Do not hold or heal the actor.
        const int clearance = 0x40000;
        for (int dz = -clearance; dz <= clearance; dz += clearance)
            for (int dx = -clearance; dx <= clearance; dx += clearance)
                if (z + dz >= boundary || Height(c, m, x + dx, z + dz) <= (long)plane + 0x10000)
                    return false;
        return true;
    }
    static int Height(CpuContext c, IMemory m, int x, int z)
    {
        var state = c.Snapshot();
        try
        {
            c.A0 = unchecked((uint)x); c.A1 = unchecked((uint)z);
            Dispatcher.Call(c, m, 0x8001B750);
            return unchecked((int)c.V0);
        }
        finally { c.Restore(state); }
    }
    static void Put(IMemory m, uint v, int x, int y, int z)
    {
        for (uint i = 0; i < 9; i++) m.WriteU16(v + 0x20 + i * 2, (ushort)(i % 4 == 0 ? 0x1000 : 0));
        m.WriteU32(v + 0x34, unchecked((uint)x));
        m.WriteU32(v + 0x38, unchecked((uint)y));
        m.WriteU32(v + 0x3C, unchecked((uint)z));
        m.WriteU32(v + 0x4C, unchecked((uint)x));
        m.WriteU32(v + 0x50, unchecked((uint)y));
        m.WriteU32(v + 0x54, unchecked((uint)z));
        StopMotion(m, v);
    }
    static void StopMotion(IMemory m, uint v)
    {
        for (uint offset = 0x74; offset <= 0x98; offset += 4) m.WriteU32(v + offset, 0);
    }
}
