using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class ProjectileOracleChecks
{
    public static int Run()
    {
        int checks=0;
        void Check(bool value,string label) {if(!value) throw new Exception(label);checks++;}
        // Independent original and sequel memory layouts. The oracle executes
        // the original generated instructions, never the port's implementation.
        var old=new PSMemory();var current=new PSMemory();
        var oc=new CpuContext{SP=0x78000};var nc=new CpuContext{SP=0x78000};
        const uint obj=0x61000,next=0x62000,socket=0x63000,bank=0x64000;
        int oldAlloc=0,newAlloc=0,oldFree=0,newFree=0;
        var originalKinds=new List<uint>();var nativeKinds=new List<uint>();
        var services=(Dictionary<uint,Action<CpuContext,IMemory>>)typeof(Dispatcher).GetField("_hostFunctions",System.Reflection.BindingFlags.NonPublic|System.Reflection.BindingFlags.Static)!.GetValue(null)!;
        void Bind(uint a,Action<CpuContext,IMemory> f)=>services[a]=f;
        Bind(0x8001ac44,(c,m)=>{oldAlloc++;originalKinds.Add(c.A1);c.V0=next;});
        Bind(0x8002C17C,(c,m)=>{newAlloc++;nativeKinds.Add(c.A1);c.V0=next;});
        Bind(0x8001b038,(c,m)=>c.V0=socket);Bind(0x8002C5F4,(c,m)=>c.V0=socket);
        Bind(0x8002036c,(c,m)=>c.V0=0);Bind(0x8003066C,(c,m)=>c.V0=0);
        Bind(0x800205f8,(c,m)=>{oldFree++;});Bind(0x800309A0,(c,m)=>{newFree++;});
        void Transform(CpuContext c,IMemory m)
        {for(uint i=0;i<12;i+=4)m.WriteU32(c.A2+i,m.ReadU32(c.A0+0x14+i)+m.ReadU32(c.A1+i));}
        Bind(0x80043408,Transform);Bind(0x80024148,Transform);
        void Invoke(int mode)
        {
            oc.A0=obj;oc.A1=(uint)mode;oc.A2=0;OriginalProjectileOracle.LAB_800359c0(oc,old);
            nc.A0=obj;nc.A1=(uint)mode;nc.A2=0;Dispatcher.Call(nc,current,V8OriginalSpecials.ProjectileCallback);
            Check(oc.V0==nc.V0,"Original callback return");
            Check(oc.SP==nc.SP&&oc.SP==0x78000,"Original stack preservation");
        }
        foreach(ushort generation in new ushort[]{0,1,8})
        foreach(ushort delay in new ushort[]{0,1,2,3})
        {
            old.WriteU32(obj+0x58,bank);current.WriteU32(obj+0x5C,bank);
            old.WriteU16(obj+0xA,58);current.WriteU16(obj+0x1A,58);
            old.WriteU16(obj+6,65535);current.WriteU16(obj+0xA,65535);
            old.WriteU16(obj+0xC,50);current.WriteU16(obj+0x1C,50);
            for(uint i=0;i<32;i+=4){old.WriteU32(obj+0x10+i,4096+i);current.WriteU32(obj+0x20+i,4096+i);}
            for(uint i=0;i<12;i+=4)
            {
                uint velocity=unchecked((uint)(-128+(int)i));
                old.WriteU32(obj+0x88+i,velocity);current.WriteU32(obj+0x88+i,velocity);
                old.WriteU32(socket+4+i,17+i);current.WriteU32(socket+4+i,17+i);
            }
            old.WriteU32(obj+0x80,0x123456);current.WriteU32(obj+0x80,0x123456);
            foreach(var mem in new[]{old,current}){mem.WriteU16(obj+0x94,generation);mem.WriteU16(obj+0x96,delay);}
            int before=oldAlloc;Invoke(0);
            Check(oldAlloc==newAlloc,"Original allocation cadence");
            Check(old.ReadU16(obj+0x96)==current.ReadU16(obj+0x96),"Original propagation countdown wrap");
            for(uint i=0;i<12;i+=4)
            {
                Check(old.ReadU32(obj+0x24+i)==current.ReadU32(obj+0x34+i),"Original moving segment position");
                Check(old.ReadU32(obj+0x48+i)==current.ReadU32(obj+0x4C+i),"Original collision position");
            }
            if(oldAlloc==before)continue;
            Check(originalKinds.SequenceEqual(nativeKinds),"Original beam model kind");
            Check(old.ReadU16(next+0x94)==current.ReadU16(next+0x94),"Original generation decrement");
            Check(old.ReadU16(next+0x96)==current.ReadU16(next+0x96),"Original child delay");
            Check(old.ReadU16(next+0xC)==current.ReadU16(next+0x1C),"Original child damage");
            Check(old.ReadU32(next+0x80)==current.ReadU32(next+0x80),"Original owner propagation");
            for(uint i=0;i<32;i+=4)Check(old.ReadU32(next+0x10+i)==current.ReadU32(next+0x20+i),"Original child transform");
        }
        Invoke(5);Check(oldFree==newFree&&oldFree==1,"Original animation-driven retirement");
        // Collision is not retirement or a generic knockback. It creates the
        // original spark effect and HUD flash, and leaves animation in charge.
        const uint hit=0x65000,record=0x66000;
        old.WriteU32(record,hit);current.WriteU32(record,hit);
        old.WriteU8(hit+4,2);current.WriteU8(hit+8,2);
        old.WriteU16(hit+6,65535);current.WriteU16(hit+0xA,65535);
        int oldHud=0,newHud=0,oldFx=0,newFx=0,oldFlash=0,newFlash=0;
        var registryEntries=(System.Collections.IEnumerable)typeof(V82VehicleRegistry).GetField("Entries",System.Reflection.BindingFlags.NonPublic|System.Reflection.BindingFlags.Static)!.GetValue(null)!;
        foreach(var entry in registryEntries)
            if((int)entry.GetType().GetProperty("OriginalSpecialType")!.GetValue(entry)! == 7)
                entry.GetType().GetProperty("BodyRuntime")!.SetValue(entry,bank);
        Bind(0x80040234,(c,m)=>{oldFx++;c.V0=next;});
        Bind(0x8002C17C,(c,m)=>{newFx++;Check(c.A1==(uint)V82VehicleRegistry.OriginalImpactKindForBank(bank),"Source impact metadata survives without vehicle identity");c.V0=next;});
        Bind(0x8004410c,(c,m)=>c.V0=1);Bind(0x8001DD9C,(c,m)=>c.V0=1);
        Bind(0x8004483c,(c,m)=>Check(c.A2==0x41,"Original collision sound 0x41"));
        Bind(0x8001E580,(c,m)=>Check(c.A2==3,"Imported original collision sound slot"));
        Bind(0x8003fea8,(c,m)=>{oldFlash++;Check(c.A1==0x08C0C000,"Original flash color");});
        Bind(0x8002E22C,(c,m)=>c.V0=1);
        Bind(0x8004E338,(c,m)=>{newFlash++;Check(c.A0==0x08C0C000,"Source flash color mapping");});
        Bind(0x80012050,(c,m)=>{oldHud++;Check(c.A1==20,"Original HUD duration");});
        Bind(0x80015ADC,(c,m)=>{newHud++;Check(c.A1==20,"Source HUD duration mapping");});
        Bind(0x80017160,(c,m)=>c.V0=32767);Bind(0x8002AC5C,(c,m)=>c.V0=32767);
        old.WriteU32(obj,0x01800084);current.WriteU32(obj+4,0x60000084);
        oc.A0=obj;oc.A1=3;oc.A2=record;OriginalProjectileOracle.LAB_800359c0(oc,old);
        nc.A0=obj;nc.A1=3;nc.A2=record;Dispatcher.Call(nc,current,V8OriginalSpecials.ProjectileCallback);
        Check(oc.V0==nc.V0&&oc.V0==0,"Original non-retiring hit return");
        Check(oldFx==newFx&&oldFx==1&&oldFlash==newFlash&&oldHud==newHud,"Original hit effect sequence");
        Check(oldFree==newFree&&oldFree==1,"Collision cannot prematurely retire the beam");
        Check((old.ReadU32(obj)&0x20)==(current.ReadU32(obj+4)&0x20),"Original collision flag");
        return checks;
    }
}
