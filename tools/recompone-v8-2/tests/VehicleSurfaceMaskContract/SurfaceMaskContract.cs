using System.Buffers.Binary;
using System.Reflection;
using System.Numerics;
using System.Text.Json;
using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using Silk.NET.OpenGL;

// Feed the software GP0 raster and Enhanced exactly the same synthetic packets.
// Compare interior pixels (not antialiased edges) both before and after an E6
// check-mask draw. No game, media, external textures or real display required.
static class SurfaceMaskContract
{
    const int W = 128, H = 160, tile = 8;
    const ushort bg = 0x7800, green = 0x03c0;
    const uint vehicles = 0x10000, terrain = 0x12000, world = 0x14000;
    static readonly ushort[] Colors = [0x001e, 0x801e, 0, 0x8000];
    enum Surface { Reflection, Gloss, Glass, Terrain, Body }

    public static int Run(GL gl, int requested, bool replacement)
    {
        if (requested is not (0 or 2 or 4)) throw new ArgumentOutOfRangeException(nameof(requested));
        var data = Directory.CreateTempSubdirectory("v8-surface-mask-");
        foreach (string key in new[] { "RECOMPONE_USER_DATA_DIR", "RECOMPONE_MOD_DIR", "RECOMPONE_TEXTURE_PACK_DIR" })
            Environment.SetEnvironmentVariable(key, data.FullName);
        Environment.SetEnvironmentVariable("RECOMPONE_DISABLE_STOCK_PAINT_CORRECTION", "1");
        if (replacement) WritePack(data.FullName);
        var cfg = ConfigManager.View;
        cfg.MsaaSamples = requested;
        cfg.EnhancedDepthBuffer = cfg.HighResolution3D = true;
        cfg.Ps1Dithering = cfg.EnhancedFog = cfg.TextureSmoothing = cfg.EnhancedParticles = false;
        cfg.HighResolutionTextures = replacement;
        GlVram.Scale = 1;
        GpuHle.WideAspect = GpuHle.BaseAspect;
        GpuHle.GameplayActive = true;
        var backend = new EnhancedGlBackend(gl);
        try
        {
            backend.InitGl();
            if (!backend.Ready) throw new Exception("Enhanced startup failed");
            GpuHle.NotifyDisplay(0, 0, W, H);
            GpuHle.RegisterVehiclePacketRange(vehicles, vehicles + 0x1000);
            GpuHle.RegisterVehicleReflectionPacket(vehicles);
            GpuHle.BeginTerrainRoutePacketWrites();
            GpuHle.ObservePacketWrite(terrain);
            GpuHle.EndTerrainRoutePacketWrites();

            GpuHle.Backend = backend;
            List<string> failures = new();
            int checks = 0;
            foreach (bool consumer in new[] { false, true })
            {
                // Capture a whole scene only after BOTH draws; never read back or
                // resolve between producer and consumer to make the bug disappear.
                GpuHle.Active = false;
                ushort[] native = Scene(new Gpu(), null, consumer);
                GpuHle.Active = true;
                ushort[] enhanced = Scene(new Gpu(), backend, consumer);
                int i = 0;
                foreach (Surface surface in Enum.GetValues<Surface>())
                foreach (int depth in new[] { 0, 1, 2 })
                foreach (bool set in new[] { false, true })
                foreach (ushort texel in Colors)
                {
                    int x = X(i) + 2, y = Y(i++) + 2, at = y * W + x;
                    ushort expected = Expected(surface, set, texel, consumer);
                    // Ordinary surfaces get a yellow DDS in place of a red native
                    // texel; kind-12 is deliberately excluded from HD replacement.
                    // The RGB changes, but the PS1 write mask must not.
                    bool changed = replacement &&
                        surface is not (Surface.Reflection or Surface.Gloss) &&
                        (texel & 0x7fff) != 0;
                    ushort glExpected = changed ? Expected(surface,set,
                        (ushort)(texel | green), consumer) : expected;
                    if (native[at] != expected || enhanced[at] != glExpected)
                        failures.Add($"{surface} bpp={4 << depth} set={set} texel={texel:x4} overlay={consumer}: native-wanted={expected:x4} GL-wanted={glExpected:x4}, GP0={native[at]:x4}, GL={enhanced[at]:x4}");
                    checks++;
                }
            }
            // Two mask-checking writes use identical draw state and overlap.
            // The second must see the bit written by the first, not an older
            // resolve shared by both primitives. Do not flush between them.
            ushort Overlap(Gpu g, bool enhanced, int kind)
            {
                GpuHle.Active = enhanced;
                void GP(params uint[] words) { foreach (uint word in words) g.WriteGp0(word); }
                GP(0xe3000000,0xe4000000 | (uint)(W-1) | (uint)(H-1)<<10,0xe6000000,
                    kind == 2 ? 0x020000f0u : 0x02000000u,0,XY(W,H));
                g.BeginOrderingTable(); g.SetOrderingTableDepth(80); g.SetOrderingTablePacket(world,3);
                if (kind == 0)
                    GP(0xe6000003,0x600000f0,XY(8,8),XY(24,24),0x6000f000,XY(16,16),XY(24,24));
                else if (kind == 1)
                {
                    // Production recognizes each full-view OT polygon as a
                    // deferred screen effect; a disjoint UI rect starts replay.
                    GP(0xe1000020,0xe6000003,
                        0x220000f0,0,XY(320,0),XY(0,240),
                        0x2200f000,0,XY(320,0),XY(0,240),
                        0xe6000000,0x60000000,XY(W-2,H-2),XY(1,1));
                }
                else
                {
                    // The subtractive hardware emulation has two internal
                    // passes. Neither may mask the other on this same command.
                    GP(0xa0000000,XY(640,0),XY(1,1),0x8010);
                    g.SetOrderingTablePacket(vehicles,7);
                    GP(0xe6000002,0x27808080,XY(8,8),0,XY(60,8),0x014a0000,XY(8,60),0);
                }
                g.EndOrderingTable();
                if (!enhanced) return g.Vram[20*1024+20];
                ushort[] pixel=new ushort[1];
                backend.ReadVram(20,20,1,1,pixel);
                return pixel[0];
            }
            for (int kind=0; kind<3; kind++)
            {
                ushort soft=Overlap(new Gpu(),false,kind), hw=Overlap(new Gpu(),true,kind);
                ushort exp = kind == 2 ? (ushort)0x800e : (ushort)0x801e;
                if(soft != exp || hw != soft)
                    failures.Add($"mask boundary {kind}: expected={exp:x4}, GP0={soft:x4}, GL={hw:x4}");
                checks++;
            }
            var rt = (GlDisplayRt?)typeof(EnhancedGlBackend).GetField("_kTarget",
                BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(backend);
            if (rt is null || (requested == 0) != (rt.Samples == 0)) throw new Exception("Wrong target/MSAA");
            // The direct terrain marker reconstructs the actual Dreamcast
            // textured leaves, whose shader returns before generic PS1 lighting.
            // This is a separate path from the ordinary software route polygons.
            foreach (bool stp in new[] {false,true})
            foreach (bool set in new[] {false,true})
            {
                ushort bit = (ushort)(stp ? 0x8000 : 0);
                backend.FillRect(0, 0, W, H, 0);
                rt.ClearDepth(gl); // same patch pose in four independent cases
                backend.SetDrawEnv(new HleDrawEnv { ClipX1=W-1, ClipY1=H-1, SetMask=set });
                backend.WriteVram(640, 0, 1, 1, [(ushort)(bit | 0x001e)]);
                var samples = new DreamcastTerrainGeometry.Sample[25];
                const float z = 2100f;
                for (int xx=0; xx<5; xx++)
                for (int yy=0; yy<5; yy++)
                    samples[xx*5+yy] = new(new((32+8*xx-64)*z/64,
                        (64+8*yy-80)*z/64,z),0,new(255),new(0));
                var patch = new GpuHle.TerrainPatchGeometry(samples,Vector3.Zero,64,80,64);
                var descriptor = new GpuHle.TerrainTextureDescriptor(0,0,0,0,0,0x10a,0,0,255,255,255);
                var distance = new GpuHle.TerrainQuadDistanceColors(default,default,default,default,false,true,patch);
                var tiles = new GpuHle.TerrainCellTextures(Enumerable.Repeat(descriptor,16).ToArray(),4,true,distance);
                const uint marker = 0x20000;
                GpuHle.RegisterCoarseTerrainPacket(marker,tiles,false,"synthetic",0,0);
                var flags = new PrimFlags { Material=HleMaterialKind.TerrainRoute, PacketAddress=marker, OtIndex=80 };
                HleVertex v = default;
                backend.DrawTri(v,v,v,flags);
                ushort[] observed = new ushort[1]; backend.ReadVram(42,73,1,1,observed);
                int expected = (set || stp ? 0x8000 : 0) | 0x001e | (replacement ? green : 0);
                if (observed[0]!=expected)
                    failures.Add($"generated terrain stp={stp} set={set}: expected={expected:x4}, GL={observed[0]:x4}");
                checks++;
            }
            // Genuine buffered vehicle shadow path: overlapping, coherent
            // subtractive commands form two source primitives even with identical
            // GL state. Keep their mask boundary when replayed over terrain.
            backend.FillRect(0,0,W,H,0x001e);
            rt.ClearDepth(gl);
            backend.WriteVram(640,0,2,1,[0x8010,0x800a]);
            var draw = new HleDrawEnv { ClipX1=W-1, ClipY1=H-1, CheckMask=true };
            backend.SetDrawEnv(draw);
            var shadow = new PrimFlags { Material=HleMaterialKind.Subtractive, Vehicle=true,
                Textured=true, SemiTrans=true, RawTexture=true, TPage=0x14a, OtIndex=80 };
            HleVertex ShadowVertex(int x,int y, short u) => new()
            {
                X=x,Y=y,U=u, Z=80, HasGteZ=true, HasCoherentGteZ=true
            };
            foreach (short u in new short[] {0,1})
                backend.DrawTri(ShadowVertex(8,8,u),ShadowVertex(60,8,u),ShadowVertex(8,60,u),shadow);
            var pending = (System.Collections.IList)typeof(EnhancedGlBackend).GetField(
                "_deferredVehicleSubtractive", BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(backend)!;
            if (pending.Count==0) throw new Exception("Missing buffered shadow control");
            draw.CheckMask=false; backend.SetDrawEnv(draw);
            backend.DrawRect(new HleRect { X=W-2,Y=H-2,W=1,H=1 },new PrimFlags {Material=HleMaterialKind.Ui});
            ushort[] overlap = new ushort[1]; backend.ReadVram(20,20,1,1,overlap);
            if (overlap[0] != 0x800e)
                failures.Add($"buffered vehicle shadows: expected=800e, GL={overlap[0]:x4}");
            checks++;
            Console.WriteLine($"driver={gl.GetStringS(StringName.Renderer)} req={requested} actual={rt.Samples} HD={replacement} checks={checks} failures={failures.Count}");
            foreach (string msg in failures) Console.Error.WriteLine("FAIL " + msg);
            if (gl.GetError() != GLEnum.NoError) throw new Exception("OpenGL error");
            return failures.Count == 0 ? 0 : 1;
        }
        finally
        {
            GpuHle.Active = GpuHle.GameplayActive = false;
            GpuHle.Backend = null;
            GpuHle.ResetSceneTracking();
            backend.Dispose();
            data.Delete(true);
        }
    }

    static ushort[] Scene(Gpu gpu, EnhancedGlBackend? backend, bool consumer)
    {
        void GP(params uint[] words) { foreach (uint word in words) gpu.WriteGp0(word); }
        void Upload(int xx, int yy, params ushort[] pixels)
        {
            GP(0xa0000000, XY(xx,yy), XY(pixels.Length,1));
            for (int j=0; j<pixels.Length; j+=2)
                GP((uint)pixels[j] | (j+1 < pixels.Length ? (uint)pixels[j+1] << 16 : 0u));
        }
        void ClearMask() => GP(0xe6000000);
        // A blue original background makes transparent black vs. masked black,
        // replacement vs. blend, and later write protection directly observable.
        ClearMask();
        GP(0xe3000000, 0xe4000000 | (uint)(W-1) | (uint)(H-1)<<10, 0x02f00000, 0, XY(W,H));
        Upload(512,0,0x3210); Upload(576,0,0x0100,0x0302); Upload(640,0,Colors);
        Upload(384,300,Colors); // indexed paths share identical authored 5551 texels
        uint clut = 24u | (300u << 6);
        gpu.BeginOrderingTable();
        gpu.SetOrderingTableDepth(80);
        int i = 0;
        foreach (Surface surface in Enum.GetValues<Surface>())
        foreach (int depth in new[] { 0, 1, 2 })
        foreach (bool set in new[] { false, true })
        for (uint u = 0; u < Colors.Length; u++)
        {
            GP(0xe6000000 | (set ? 1u : 0u));
            uint packet = surface switch
            {
                Surface.Reflection or Surface.Gloss => vehicles,
                Surface.Glass => vehicles+64, Surface.Terrain => terrain, _ => world
            };
            gpu.SetOrderingTablePacket(packet, 7);
            int x = X(i), y = Y(i++);
            uint page = (uint)(8 + depth + 128 * depth);
            bool semi = surface is Surface.Gloss or Surface.Glass;
            GP(semi ? 0x27808080u : 0x25808080u, XY(x,y), u | clut<<16,
                XY(x+tile,y), u | page<<16, XY(x,y+tile), u);
        }
        backend?.Flush();
        if (consumer)
        {
            // One ordinary mask-checking surface covers all lanes. Resolve must
            // preserve protected samples AND commit new color to the live draw FBO.
            gpu.SetOrderingTablePacket(world, 3);
            GP(0xe6000002, 0x6000f000, 0, XY(W,H));
            backend?.Flush();
        }
        gpu.EndOrderingTable();
        if (backend is null)
        {
            var native = new ushort[W*H];
            for (int yy=0; yy<H; yy++) Array.Copy(gpu.Vram,1024*yy,native,yy*W,W);
            return native;
        }
        var pixels = new ushort[W*H]; backend.ReadVram(0,0,W,H,pixels); return pixels;
    }

    static ushort Expected(Surface surf, bool set, ushort texel, bool consume)
    {
        if(texel==0) return consume ? green : bg;
        bool stp=(texel&0x8000)!=0, masked=stp||set;
        if(consume && !masked) return green;
        int c=texel&0x7fff;
        if(stp && surf is Surface.Glass or Surface.Gloss)
            c=(c & 0x7bde)>>1 | (bg>>1);
        return (ushort)(c|(masked ? 0x8000 : 0));
    }
    static void WritePack(string dir)
    {
        // Same apparent opacity is not the same as a PS1 bit: this deliberately
        // inverts DDS alpha while changing red into yellow to prove actual use.
        byte[] dds = new byte[128 + 8];
        void U32(int offset,uint value) => BinaryPrimitives.WriteUInt32LittleEndian(
            dds.AsSpan(offset),value);
        U32(0,0x20534444); U32(4,124); U32(8,0x100f); U32(12,1); U32(16,2); U32(20,8);
        U32(76,32); U32(80,0x41); U32(88,32); U32(92,0xff); U32(96,0xff00);
        U32(100,0xff0000); U32(104,0xff000000); U32(108,0x1000);
        U32(128,0xff00f7f7); U32(132,0x0000f7f7);
        File.WriteAllBytes(Path.Combine(dir,"synthetic.dds"),dds);
        string Key(byte stp)
        {
            ulong h=14695981039346656037UL;
            foreach(byte b in new byte[]{1,0,1,0,247,0,0,stp}) h=unchecked((h^b)*1099511628211UL);
            return h.ToString("x16");
        }
        var entries = new[]{Key(0),Key(255)}.Select((key,x) =>
            new { key, image="synthetic.dds", x, y=0, width=1, height=1 });
        File.WriteAllText(Path.Combine(dir,"manifest.json"),
            JsonSerializer.Serialize(new { format=3, entries }));
    }
    static uint XY(int x,int y) => (uint)x | ((uint)y<<16);
    static int X(int i) => (i%16)*tile;
    static int Y(int i) => (i/16)*tile;
}
