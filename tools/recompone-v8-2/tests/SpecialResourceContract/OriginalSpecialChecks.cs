using System.Reflection;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class OriginalSpecialChecks
{
    public static int Run()
    {
        int checks=0;
        void Check(bool ok,string message) { if(!ok) throw new Exception(message); checks++; }
        V8OriginalSpecials.Register();
        var m=new PSMemory();
        var c=new CpuContext { SP=0x70000, RA=0x1234 };
        const uint owner=0x80610000, weapon=0x80610400, pivot=0x80610600,
            shot=0x80610800, effect=0x80610A00, fx=0x80610C00, target=0x80611000,
            bank=0x60000, matrix=0x61000, hitRecord=0x62000;
        var entries=(Dictionary<uint,int>)typeof(V82VehicleRegistry).GetField("ObjectEntries",BindingFlags.NonPublic|BindingFlags.Static)!.GetValue(null)!;
        m.WriteU32(owner+0x5C,bank); m.WriteU32(weapon+0x5C,bank);
        m.WriteU32(bank+0xC,0xABCD); m.WriteU32(weapon+0x10,pivot);
        m.WriteU32(owner+0xF0,target); m.WriteU32(shot+0x5C,bank);
        m.WriteU8(target+8,2); m.WriteU32(shot+0x80,owner);
        var allocations=new List<uint>(); var sounds=new List<uint>();
        int retired=0,impulses=0,shots=0,angleCalls=0;
        uint terrain=0x1000000;
        int nativeAiResult=0;
        void Bind(uint addr,Action<CpuContext,IMemory> f)=>Dispatcher.RegisterHostFunction(addr,(r,mem)=>f(r,mem));
        foreach(uint addr in new uint[]{0x8002CF74,0x800305FC,0x80030BF0,0x80030CB0,0x80015ADC,0x800308C4})
            Bind(addr,(r,mem)=>r.V0=0);
        Bind(0x80042330,(r,mem)=>r.V0=1);
        Bind(0x8001DD9C,(r,mem)=>r.V0=2);
        Bind(0x8001E14C,(r,mem)=>r.V0=0);
        Bind(0x80029F6C,(r,mem)=>r.V0=(uint)nativeAiResult);
        foreach(uint addr in new uint[]{0x8001E580,0x8001E628}) Bind(addr,(r,mem)=>{ Check(r.A1==0xABCD,"Sound uses original imported bank"); sounds.Add(r.A2); });
        Bind(0x8002C17C,(r,mem)=>{ Check(r.A0==bank,"Allocate original asset bank"); allocations.Add(r.A1); r.V0=allocations.Count==1?effect:fx; });
        Bind(0x80042408,(r,mem)=>{ Check(r.A0==owner&&r.A1==pivot,"Fire from original articulated mount"); Check(r.A3==0x98&&mem.ReadU32(r.SP+0x10)==effect,"Original projectile size and attached effect"); shots++; allocations.Add(r.A2); r.V0=shot; });
        Bind(0x800309C8,(r,mem)=>{Check(r.A0==shot,"Retire own shot"); retired++;});
        Bind(0x800309A0,(r,mem)=>{retired++;});
        Bind(0x8003066C,(r,mem)=>r.V0=123);
        Bind(0x8001B750,(r,mem)=>r.V0=terrain);
        Bind(0x8002B1FC,(r,mem)=>{Check(r.A0==target,"Impulse reaches collided vehicle");Check((int)mem.ReadU32(r.A1)==-16,"Original signed impulse scale");impulses++;});
        Bind(0x8002CDF4,(r,mem)=>r.V0=matrix);
        Bind(0x8002426C,(r,mem)=>{ Check(r.A0==matrix,"Original inverse rotation"); mem.WriteU32(r.A2,10);mem.WriteU32(r.A2+4,20);mem.WriteU32(r.A2+8,30); });
        Bind(0x8005B9AC,(r,mem)=>r.V0=(angleCalls++%2)==0?3800u:3500u);
        void Invoke(uint cb,uint obj,uint evt,uint arg=0)
        {
            c.A0=obj;c.A1=evt;c.A2=arg;
            Dispatcher.Call(c,m,cb);
            Check(c.SP==0x70000&&c.RA==0x1234,"Callback preserves caller stack and return address");
        }
        foreach(bool alien in new[]{false,true})
        {
            entries[owner]=alien?12:7;
            uint callback=alien?V8OriginalSpecials.AlienCallback:V8OriginalSpecials.HoustonCallback;
            Check(V82VehicleRegistry.SpecialCallbackForObject(m,owner,0x80049880)==callback,"Registry selects source callback");
            Invoke(callback,weapon,1,owner);
            Check(m.ReadU16(weapon+0x1C)==12,"Original ammo initialization");
            angleCalls=0;Invoke(callback,weapon,0,owner);
            Check((short)m.ReadU16(pivot+0x46)==-296,"Original signed 12-bit yaw");
            Check((short)m.ReadU16(pivot+0x44)==(alien?596:256),"Y unclamped pitch and Houston source clamp");
            allocations.Clear();sounds.Clear();
            m.WriteU32(owner+0x80,unchecked((uint)-128));
            Invoke(callback,weapon,12,owner);
            Check(c.V0==180,"Original AI fire delay");
            Check(allocations.Take(2).SequenceEqual(alien?new uint[]{42,40}:new uint[]{60,58}),"Exact original model kinds; no tow assets");
            Check(m.ReadU16(shot+0x1C)==(alien?100:50),"Original damage");
            Check((int)m.ReadU32(shot+0x88)==-1,"MIPS projectile inheritance rounds down");
            Check(m.ReadU32(shot)==V8OriginalSpecials.ProjectileCallback,"Original projectile behavior");
            Check(m.ReadU16(weapon+0x1C)==11,"Exactly one ammo consumed");
            Check(sounds.SequenceEqual(new uint[]{alien?2u:0u}),"Original firing sample");
            m.WriteU32(owner+0x8C,0); m.WriteU32(0x8006B874,0);
            nativeAiResult=0x12BFFF;Invoke(callback,weapon,13,owner);Check(c.V0==1,"Source AI inclusive range");
            nativeAiResult=0x12C000;Invoke(callback,weapon,13,owner);Check(c.V0==0,"Source AI rejects beyond range");
        }
        Check(shots==2,"Both source specials exercise firing");
        // Original Object_GeneralTick fixture. Input handling must not integrate;
        // the physics dispatch integrates once with MIPS rounding and source lift.
        entries[owner]=12;
        for(uint i=0;i<0x18;i+=2) m.WriteU16(owner+0x20+i,0);
        m.WriteU16(owner+0x20,4096);m.WriteU16(owner+0x28,4096);m.WriteU16(owner+0x30,4096);
        for(uint i=0;i<12;i+=4) { m.WriteU32(owner+0x34+i,0); m.WriteU32(owner+0x80+i,0); m.WriteU32(owner+0x90+i,0); }
        m.WriteU32(owner+0x80,128);m.WriteU32(owner+0x90,512);m.WriteU32(owner+0x98,512);
        m.WriteU16(owner+0xAA,0);m.WriteU16(owner+0xA8,0);
        Bind(0x8002CFBC,(r,mem)=>{Check(r.A0==owner&&r.A1==owner+0x34,"Source terrain AND obstacle probe");r.V0=65536;});
        Bind(0x80029E84,(r,mem)=>{Check(r.A0==owner+0x80,"Source length input");r.V0=128;});
        Bind(0x80059A0C,(r,mem)=>{Check(r.A0==owner+0x20&&r.A1==r.A0,"Normalize source rotation");});
        c.A0=owner;
        V82VehicleRegistry.BeginControllerPhysics(c,m);
        Check(m.ReadU32(owner+0x34)==0&&m.ReadU32(owner+0x38)==0,"Input hook cannot double-integrate flight");
        Check(V82VehicleRegistry.ApplyOriginalControllerPhysics(c,m,owner),"Original flight replaces native movement");
        Check(m.ReadU32(owner+0x34)==1&&m.ReadU32(owner+0x38)==38,"Original position increment");
        Check(m.ReadU32(owner+0x80)==126&&m.ReadU32(owner+0x84)==4851,"Original lift and linear damping");
        Check(m.ReadU32(owner+0x5C)==bank,"Flight cannot overwrite the native asset pointer");
        entries.Remove(owner);
        Check(V82VehicleRegistry.SpecialCallbackForObject(m,owner,0x12345678)==0x12345678,"Stock special callback remains untouched");
        return checks;
    }
}
