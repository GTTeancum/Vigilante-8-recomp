using System.Reflection;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class FlightOracleChecks
{
    public static int Run()
    {
        int checks=0;
        void Check(bool ok,string label){if(!ok)throw new Exception(label);checks++;}
        var services=(Dictionary<uint,Action<CpuContext,IMemory>>)typeof(Dispatcher).GetField("_hostFunctions",BindingFlags.NonPublic|BindingFlags.Static)!.GetValue(null)!;
        var entries=(Dictionary<uint,int>)typeof(V82VehicleRegistry).GetField("ObjectEntries",BindingFlags.NonPublic|BindingFlags.Static)!.GetValue(null)!;
        const uint obj=0x61000;
        entries[obj]=12;
        int terrain=0;
        services[0x8001d748]=(c,m)=>c.V0=unchecked((uint)terrain);
        services[0x8002CFBC]=(c,m)=>c.V0=unchecked((uint)terrain);
        services[0x80016a20]=(c,m)=>c.V0=19000;
        services[0x80029E84]=(c,m)=>c.V0=19000;
        services[0x80017324]=OriginalFlightOracle.FUN_80017324;
        services[0x800439b8]=OriginalFlightOracle.FUN_800439b8;
        services[0x80059A0C]=(c,m)=>{}; // Identical external normalization seam.
        var random=new Random(30534);
        for(int sample=0;sample<128;sample++)
        {
            var old=new PSMemory();var current=new PSMemory();
            var oc=new CpuContext{SP=0x78000,A0=obj};var nc=new CpuContext{SP=0x78000,A0=obj};
            terrain=random.Next(-1000000,1000000);
            for(uint i=0;i<32;i+=2)
            {
                ushort word=unchecked((ushort)random.Next(-4096,4097));
                old.WriteU16(obj+0x10+i,word);current.WriteU16(obj+0x20+i,word);
            }
            for(uint i=0;i<12;i+=4)
            {
                uint position=unchecked((uint)random.Next(-1000000,1000000));
                old.WriteU32(obj+0x24+i,position);current.WriteU32(obj+0x34+i,position);
                uint velocity=unchecked((uint)random.Next(-100000,100000));
                old.WriteU32(obj+0x80+i,velocity);current.WriteU32(obj+0x80+i,velocity);
                uint angular=unchecked((uint)random.Next(-10000,10000));
                old.WriteU32(obj+0x90+i,angular);current.WriteU32(obj+0x90+i,angular);
            }
            ushort steer=unchecked((ushort)random.Next(-128,128)),thrust=unchecked((ushort)random.Next(-128,128));
            old.WriteU16(obj+0xA4,steer);current.WriteU16(obj+0xA8,steer);
            old.WriteU16(obj+0xA6,thrust);current.WriteU16(obj+0xAA,thrust);
            old.WriteU32(obj+0xD8,0x5000);
            // Timers and follow position belong to the native common epilogue.
            old.WriteU32(obj,0x800000);
            OriginalFlightOracle.FUN_80030c08(oc,old);
            Check(V82VehicleRegistry.ApplyOriginalControllerPhysics(nc,current,obj),"Y source physics selected");
            for(uint i=0;i<32;i+=2)
                Check(old.ReadU16(obj+0x10+i)==current.ReadU16(obj+0x20+i),$"Original Y matrix/position sample={sample} offset={i:X}");
            for(uint i=0;i<28;i+=4)
                Check(old.ReadU32(obj+0x80+i)==current.ReadU32(obj+0x80+i),$"Original Y velocity sample={sample} offset={i:X}");
        }
        entries.Remove(obj);
        return checks;
    }
}
