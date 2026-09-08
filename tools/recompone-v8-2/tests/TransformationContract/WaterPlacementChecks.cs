using System.Numerics;
using System.Reflection;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

namespace Recompiled;

public static partial class Vigilante82PC
{
    static void CheckWaterPlacement()
    {
        var assembly = typeof(V82AutoWaterski).Assembly;
        var type = assembly.GetType("RecompOne.Runtime.Sdk.V82WaterAttachmentPlacement", true)!;
        var blend = type.GetMethod("Blend", BindingFlags.Static | BindingFlags.NonPublic)!;
        float B(byte mode, int ticks) => (float)blend.Invoke(null, [mode, ticks])!;
        Check(B(2,32)==0 && B(2,16)==0 && B(2,0)==1, "water entry has continuous unshifted swap and fully anchored endpoint");
        Check(B(0,32)==1 && B(0,16)==0 && B(0,0)==0, "water exit keeps the current placement then removes it before the wheel swap");
        for (int t=0;t<=16;t++)
            Check(Math.Abs(B(2,t)-B(0,32-t))<0.00001f, "entry and exit use the same reversible trajectory");
        Check(B(2,8)==0.5f && B(3,24)==0.5f, "halfway fold blends entire assembly in both directions");

        var begin = type.GetMethod("Begin", BindingFlags.Static | BindingFlags.Public)!;
        var reset = type.GetMethod("Reset", BindingFlags.Static | BindingFlags.Public)!;
        foreach (bool foldedParent in new[] {false,true})
        {
            reset.Invoke(null,null);
            var m = new PSMemory();
            const uint v=0x80020000, body=0x80020400, mount=0x80020600, child=0x80020800, wrapper=0x80020A00;
            const uint bank=0x80025000, bodyBank=0x80026000, bm=0x80027000, mm=0x80027200, bv=0x80027400, mv=0x80027600, packet=0x80027800;
            void Identity(uint n)
            { m.WriteU16(n+0x20,4096);m.WriteU16(n+0x28,4096);m.WriteU16(n+0x30,4096); }
            foreach (uint n in new[] {v,body,mount,child,wrapper}) Identity(n);
            m.WriteU32(0x800C61C0,bank);m.WriteU16(0x80063F74+24,63);
            m.WriteU8(v+0xAC,2);m.WriteU16(v+0xB4,(ushort)(foldedParent ? 8 : 0));
            m.WriteU32(v+0x5C,bodyBank);m.WriteU32(v+0x10,body);m.WriteU32(v+0x104,mount);
            m.WriteU32(body+0xC,foldedParent ? wrapper : mount);m.WriteU32(body+0x5C,bodyBank);m.WriteU32(body+0x40,bm);
            m.WriteU32(mount+0x5C,bank);m.WriteU16(mount+0x1A,63);m.WriteU32(mount+0x40,mm);m.WriteU32(mount+0x10,child);
            m.WriteU32(child+0x34,500);m.WriteU32(child+0x74,0x12345678);
            if (foldedParent)
            {
                m.WriteU32(wrapper+0x10,mount);m.WriteU32(wrapper+0x34,unchecked((uint)-1000));
                m.WriteU16(wrapper+0x20,unchecked((ushort)-4096));m.WriteU16(wrapper+0x30,unchecked((ushort)-4096));
            }
            else m.WriteU32(mount+0x34,unchecked((uint)-1000));
            void Mesh(uint mesh,uint vertices,short[] values)
            {
                m.WriteU8(mesh+1,16);m.WriteU16(mesh+4,(ushort)(values.Length/3));m.WriteU32(mesh+8,vertices);
                for (int i=0;i<values.Length/3;i++)
                    for (int j=0;j<3;j++)m.WriteU16(vertices+(uint)(i*8+j*2),unchecked((ushort)values[i*3+j]));
            }
            Mesh(bm,bv,[0,-100,-100,0,100,-100,0,0,100]);
            m.WriteU16(bm+6,1);m.WriteU32(bm+0x10,packet);m.WriteU16(packet+6,8);m.WriteU16(packet+8,16);
            Mesh(mm,mv,[0,-5,0,0,5,0,-30,0,0]);
            byte[] Snapshot() => Enumerable.Range(0,0x8000).Select(i=>m.ReadU8(v+(uint)i)).ToArray();
            var original = Snapshot();
            using ((IDisposable)begin.Invoke(null,[m,v])!)
            {
                int actual = (int)m.ReadU32(mount+0x34);
                Check(foldedParent ? Math.Abs(actual+501)<=1 : actual==1,
                    "complete assembly reaches socket, with inverse compensation for rotated transition parent");
                Check(m.ReadU32(child+0x34)==500 && m.ReadU32(child+0x74)==0x12345678, "child animation and physics remain untouched");
                Check(m.ReadU32(mm+8)==mv && (short)m.ReadU16(mv+16)==-30, "source mesh and outboard support vertices are unchanged");
                Check(m.ReadU16(mount+0x20)==4096, "native assembly rotation remains unchanged");
            }
            Check(original.SequenceEqual(Snapshot()), "entire vehicle, child, mesh and body memory restored byte-exactly after draw");
            m.WriteU32(body+4,2);
            Check(begin.Invoke(null,[m,v])==null, "hidden body geometry cannot supply an attachment socket");
            m.WriteU32(body+4,0);
            // A pre-existing overlap must not be pulled through the opposite side.
            if (!foldedParent)
            {
                m.WriteU32(mount+0x34,1);
                Check(begin.Invoke(null,[m,v])==null, "embedded support is preserved instead of seeking the opposite chassis wall");
            }
            m.WriteU32(mount+0x5C,bodyBank);
            Check(begin.Invoke(null,[m,v])==null, "vehicle-authored assemblies are not rebound as shared pontoons");
        }
    }
}
