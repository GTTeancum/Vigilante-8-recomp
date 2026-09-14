using RecompOne.Runtime;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class JunctionAttachmentChecks
{
    public static int Run()
    {
        var m = new PSMemory();
        const uint bin=0x80110000, mesh=0x80120000, edge=0x80130000,
            node=0x80140000, bank=0x80150000;
        int checks=0;
        void Check(bool value,string why){if(!value)throw new Exception(why);checks++;}
        m.WriteU32(bank+4,bin);
        m.WriteU32(node+12,bank);m.WriteU32(node+24,mesh);
        m.WriteU8(node+17,1);m.WriteU16(node+18,65535);m.WriteU32(node+28,edge);
        m.WriteU32(edge,node);m.WriteU32(edge+28,unchecked((uint)-500000));
        // Start control is +0x10/+0x14. End control must not be selected.
        m.WriteU32(edge+20,unchecked((uint)-500000));
        m.WriteU32(bin,1);m.WriteU32(bin+4,0x200);m.WriteU32(bin+24,2);
        m.WriteU16(bin+28+26,1);m.WriteU16(bin+56,0x7FF);
        m.WriteU32(bin+56+12,unchecked((uint)-262197));
        m.WriteU16(bin+56+24,65535);m.WriteU16(bin+56+26,65535);
        m.WriteU32(bin+0x200,0x100);m.WriteU16(bin+0x310,4);m.WriteU32(bin+0x314,0x100);
        m.WriteU8(mesh+1,7);m.WriteU16(mesh+4,5);m.WriteU32(mesh+8,mesh+0x100);
        m.WriteU16(mesh+6,4);m.WriteU32(mesh+0x10,bin+0x400);
        (short X,short Z)[] vertices=[(-199,-511),(200,-511),(200,200),(-199,200),(0,-510)];
        for(int i=0;i<vertices.Length;i++)
        {
            m.WriteU16(mesh+0x100+(uint)i*8,unchecked((ushort)vertices[i].X));
            m.WriteU16(mesh+0x104+(uint)i*8,unchecked((ushort)vertices[i].Z));
        }
        for(int i=0;i<4;i++)
        {
            uint p=bin+0x400+(uint)i*12;
            m.WriteU16(p+4,(ushort)(i*8));m.WriteU16(p+6,(ushort)(((i+1)%4)*8));m.WriteU16(p+8,32);
        }
        m.WriteU16(0x80065C92,4096);
        bool wasGameplay=GpuHle.GameplayActive,wasActive=GpuHle.Active;
        float oldScale=Gte.PreciseViewScale;
        try
        {
            GpuHle.Active=true;GpuHle.GameplayActive=true;V82JunctionAttachments.Reset();
            for(int i=0;i<8;i++)Gte.WriteControl(i,0);
            Gte.WriteControl(0,4096);Gte.WriteControl(2,4096);Gte.WriteControl(4,4096);
            Gte.WriteControl(7,1000);Gte.WriteControl(24,160u<<16);Gte.WriteControl(25,120u<<16);Gte.WriteControl(26,256);
            Gte.PreciseViewScale=2;
            PreciseGteVertexData Project()
            {
                Gte.Write(0,unchecked((ushort)-199));Gte.Write(1,unchecked((ushort)-511));
                Gte.Execute(0x4A180001);
                if(!Gte.TryGetStoreVertex(14,out var value))throw new Exception("Missing precise vertex");
                return value;
            }
            var before=Project();uint flags=Gte.ReadControl(31);
            V82JunctionAttachments.Enter(m,node);
            var previous=V82JunctionAttachments.BeginMesh(mesh);
            Check(V82JunctionAttachments.ActiveOffsets?.Count==2,"Only the two connected boundary vertices may move");
            Check(!V82JunctionAttachments.ActiveOffsets!.ContainsKey((0,0,-510)),"Nearby interior vertex is not an attachment");
            var after=Project();
            Check(after.PackedScreenPosition==before.PackedScreenPosition,"Attachment metadata must preserve native SXY");
            Check(after.Depth==before.Depth && Gte.ReadControl(31)==flags,"Attachment metadata must preserve native SZ and FLAG");
            Check(MathF.Abs(after.ViewZ-975.79296875f)<0.0001f,"Enhanced boundary reaches the authored 16.16 connector plane");
            var nested=V82JunctionAttachments.BeginMesh(mesh+0x1000);
            Check(V82JunctionAttachments.ActiveOffsets==null,"Unrelated mesh cannot inherit junction offsets");
            V82JunctionAttachments.EndMesh(nested);
            Check(V82JunctionAttachments.ActiveOffsets?.Count==2,"Nested mesh restores the outer junction");
            V82JunctionAttachments.EndMesh(previous);V82JunctionAttachments.Exit();
            Check(V82JunctionAttachments.ActiveOffsets==null,"Junction offsets end with the mesh scope");
            Check(unchecked((short)m.ReadU16(mesh+0x104))==-511,"Native source vertex is unchanged");
            V82JunctionAttachments.Reset();V82JunctionAttachments.BeginMesh(mesh);
            Check(V82JunctionAttachments.ActiveOffsets==null,"Scene reset retires cached geometry");
            // The loader rotates the mesh and each attachment independently.
            // A quarter-turn checks signed Q12 rotation in a second axis.
            V82JunctionAttachments.Reset();
            m.WriteU16(node+22,1024);m.WriteU16(0x80066C90,4096);m.WriteU16(0x80066C92,0);
            m.WriteU32(edge+16,unchecked((uint)-500000));m.WriteU32(edge+20,0);
            for(int i=0;i<vertices.Length;i++)
            {
                m.WriteU16(mesh+0x100+(uint)i*8,unchecked((ushort)vertices[i].Z));
                m.WriteU16(mesh+0x104+(uint)i*8,unchecked((ushort)-vertices[i].X));
            }
            V82JunctionAttachments.Enter(m,node);V82JunctionAttachments.BeginMesh(mesh);
            Check(V82JunctionAttachments.ActiveOffsets?.Count==2,"Rotated source attachment retains the same boundary topology");
            Check(V82JunctionAttachments.ActiveOffsets!.TryGetValue((-511,0,199),out var delta) &&
                MathF.Abs(delta.X+1.103515625f)<0.0001f && delta.Z==0,
                "Rotated connection uses the native signed Q12 child transform");
        }
        finally{V82JunctionAttachments.Reset();GpuHle.Active=wasActive;GpuHle.GameplayActive=wasGameplay;Gte.PreciseViewScale=oldScale;}
        return checks;
    }
}
