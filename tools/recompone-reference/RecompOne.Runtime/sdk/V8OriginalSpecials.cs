using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Original SLUS_005.10 specials, ported from src/gameplay/vehicle_init_joints.c:
/// Houston 80035CF8, Y 8003607C, shared projectile 800359C0.
/// Only object layout, event numbers and engine-service calls use the V8:2 ABI.
/// No retail special overlay is borrowed. Fixed-point arithmetic wraps as on MIPS.
/// </summary>
public static class V8OriginalSpecials
{
    public const uint HoustonCallback = 0x807FF020;
    public const uint AlienCallback = 0x807FF024;
    public const uint ProjectileCallback = 0x807FF028;
    public const uint ImpactCallback = 0x807FF030;
    public const uint EffectCallback = 0x807FF02C;
    static bool Trace => Environment.GetEnvironmentVariable("RECOMPONE_TRACE_V8_SPECIAL") == "1";

    public static void Register()
    {
        Dispatcher.RegisterHostFunction(HoustonCallback, (c,m) => Special(c,m,false));
        Dispatcher.RegisterHostFunction(AlienCallback, (c,m) => Special(c,m,true));
        Dispatcher.RegisterHostFunction(ProjectileCallback, Projectile);
        Dispatcher.RegisterHostFunction(EffectCallback, Effect);
        Dispatcher.RegisterHostFunction(ImpactCallback, ImpactEffect);
    }

    internal static uint Call(CpuContext c, IMemory m, uint address,
        uint a0=0, uint a1=0, uint a2=0, uint a3=0, uint a4=0)
    {
        var saved=c.Snapshot();
        try
        {
            c.SP-=0x40; c.A0=a0; c.A1=a1; c.A2=a2; c.A3=a3;
            m.WriteU32(c.SP+0x10,a4);
            Dispatcher.Call(c,m,address);
            return c.V0;
        }
        finally { c.Restore(saved); }
    }
    static int I(IMemory m,uint p)=>unchecked((int)m.ReadU32(p));
    static int S(IMemory m,uint p)=>(short)m.ReadU16(p);
    static void W(IMemory m,uint p,int v)=>m.WriteU32(p,unchecked((uint)v));
    static int Angle(int v)=>unchecked(v<<20)>>20;
    static uint Voice(CpuContext c,IMemory m)=>Call(c,m,0x8001DD9C);
    static uint BankSound(IMemory m,uint obj)=>m.ReadU32(m.ReadU32(obj+0x5C)+0xC);

    static void Special(CpuContext c,IMemory m,bool alien)
    {
        uint obj=c.A0, owner=c.A2, result=0;
        // V8:2 inserted event 10; original equip/fire/AI/query become 11..15.
        switch(c.A1)
        {
            case 0:
                if(owner!=0 && Call(c,m,0x80042330,obj,owner)!=0)
                    Aim(c,m,obj,owner,alien);
                break;
            case 1:
                m.WriteU16(obj+0x1C,12);
                // Original weapon category/slot 3/6 becomes native category/slot 3/7.
                m.WriteU8(obj+8,3);
                m.WriteU32(obj+4,m.ReadU32(obj+4)|0x4000);
                break;
            case 2:
                if((m.ReadU32(obj+4)&0x20000)==0)
                {
                    m.WriteU32(obj+4,m.ReadU32(obj+4)|0x20000);
                    Call(c,m,0x8001E14C,Voice(c,m),BankSound(m,obj),alien?0u:1u);
                }
                if(m.ReadU16(obj+0x1C)==0) Call(c,m,0x8003A368,obj);
                break;
            case 11:
                m.WriteU32(obj+4,m.ReadU32(obj+4)&~0x20000u);
                uint matrix=Call(c,m,0x8002CDF4,obj);
                Call(c,m,alien?0x8001E628u:0x8001E580u,Voice(c,m),BankSound(m,obj),4,matrix+0x14);
                break;
            case 12:
                if(owner!=0) result=Fire(c,m,obj,owner,alien);
                break;
            case 13:
                if(owner!=0 && m.ReadU32(0x8006B874)==0 && I(m,owner+0x8C)<0x11E1)
                {
                    uint target=m.ReadU32(owner+0xF0);
                    if(target!=0)
                        result=Call(c,m,0x80029F6C,owner+0x4C,target+0x4C)<=0x12BFFF?1u:0u;
                }
                break;
            case 14: result=6; break;
            case 15: result=0x801F; break;
        }
        c.V0=result;
    }

    internal static void Aim(CpuContext c,IMemory m,uint obj,uint owner,bool alien)
    {
        uint target=m.ReadU32(owner+0xF0), child=m.ReadU32(obj+0x10);
        if(target==0 || child==0) return;
        uint matrix=Call(c,m,0x8002CDF4,obj), v=c.SP-0x20;
        for(uint i=0;i<12;i+=4) W(m,v+i,unchecked(I(m,target+0x4C+i)-I(m,matrix+0x14+i)));
        Call(c,m,0x8002426C,matrix,v,v);
        int yaw=Angle((int)Call(c,m,0x8005B9AC,m.ReadU32(v),m.ReadU32(v+8)));
        int pitch=Angle(unchecked(-(int)Call(c,m,0x8005B9AC,m.ReadU32(v+4),m.ReadU32(v+8))));
        if(!alien) pitch=Math.Clamp(pitch,-128,256);
        m.WriteU16(child+0x46,unchecked((ushort)yaw));
        m.WriteU16(child+0x44,unchecked((ushort)pitch));
        Call(c,m,0x8002CF74,child);
        if(Trace) Console.Error.WriteLine($"[V8SourceSpecial] aim source={(alien?12:7)} yaw={yaw} pitch={pitch}");
    }

    static uint Fire(CpuContext c,IMemory m,uint obj,uint owner,bool alien)
    {
        uint bank=m.ReadU32(obj+0x5C);
        uint effect=Call(c,m,0x8002C17C,bank,alien?0x2Au:0x3Cu,0x80,8);
        if(effect==0) return 0;
        // Original FUN_80031300 -> native 80042408: mount transform, owner,
        // collision identity and authored muzzle socket; fifth arg attaches effect.
        uint shot=Call(c,m,0x80042408,owner,m.ReadU32(obj+0x10),alien?0x28u:0x3Au,0x98,effect);
        if(shot==0) { Call(c,m,0x800309A0,effect); return 0; }
        m.WriteU16(shot+0x1C,alien?(ushort)100:(ushort)50);
        m.WriteU32(shot,ProjectileCallback);
        m.WriteU16(shot+0x96,2); m.WriteU16(shot+0x94,8);
        // V8 flags 0x01800084: persistent physics position and collision layer.
        // V8:2 moved the corresponding high bits to 0x60000000.
        m.WriteU32(shot+4,0x60000084);
        for(uint i=0;i<12;i+=4) W(m,shot+0x88+i,unchecked(I(m,owner+0x80+i)<<5)>>12);
        Call(c,m,0x800305FC,shot);
        m.WriteU32(effect+4,0x10); m.WriteU32(effect,EffectCallback);
        if((m.ReadU32(owner+4)&4)==0) Call(c,m,0x80030BF0,effect);
        Call(c,m,0x8001E580,Voice(c,m),BankSound(m,obj),alien?2u:0u,shot+0x34);
        Call(c,m,0x80030CB0,obj,25);
        m.WriteU16(obj+0x1C,unchecked((ushort)(m.ReadU16(obj+0x1C)-1)));
        if(Trace) Console.Error.WriteLine($"[V8SourceSpecial] fire source={(alien?12:7)} shot=0x{shot:X8} damage={m.ReadU16(shot+0x1C)} generations=8 callback=0x{ProjectileCallback:X8}");
        return S(m,owner+0xA)>=0?180u:30u;
    }

    // Original LAB_800359C0 (generated MIPS): a chain of animated segments.
    // +94 is remaining generations, +96 is the propagation countdown.
    // Animation event 5 retires a segment; neither field is a lifetime timer.
    static void Projectile(CpuContext c,IMemory m)
    {
        uint obj=c.A0, mode=c.A1, collision=c.A2;
        c.V0=0;
        if(mode==0)
        {
            for(uint i=0;i<12;i+=4)
            {
                W(m,obj+0x34+i,unchecked(I(m,obj+0x34+i)+I(m,obj+0x88+i)));
                m.WriteU32(obj+0x4C+i,m.ReadU32(obj+0x34+i));
            }
            ushort delay=unchecked((ushort)(m.ReadU16(obj+0x96)-1));
            m.WriteU16(obj+0x96,delay);
            if(delay!=0 || S(m,obj+0x94)==0) return;
            uint next=Call(c,m,0x8002C17C,m.ReadU32(obj+0x5C),m.ReadU16(obj+0x1A),0x98,8);
            if(next==0) return;
            uint socket=Call(c,m,0x8002C5F4,obj,0x8000);
            if(socket==0) throw new InvalidOperationException("Original beam segment has no source propagation socket");
            m.WriteU8(next+8,8);m.WriteU32(next+4,0x60000084);
            m.WriteU16(next+0xA,m.ReadU16(obj+0xA));
            m.WriteU16(next+0x1C,m.ReadU16(obj+0x1C));
            m.WriteU32(next,ProjectileCallback);
            Call(c,m,0x8003066C,next);
            m.WriteU16(next+0x96,3);
            m.WriteU16(next+0x94,unchecked((ushort)(m.ReadU16(obj+0x94)-1)));
            for(uint i=0;i<12;i+=4) m.WriteU32(next+0x88+i,m.ReadU32(obj+0x88+i));
            m.WriteU32(next+0x80,m.ReadU32(obj+0x80));
            for(uint i=0;i<32;i+=4) m.WriteU32(next+0x20+i,m.ReadU32(obj+0x20+i));
            Call(c,m,0x80024148,obj+0x20,socket+4,next+0x34);
            if(Trace) Console.Error.WriteLine($"[V8SourceSpecial] propagate parent=0x{obj:X8} child=0x{next:X8} remaining={m.ReadU16(next+0x94)}");
        }
        else if(mode==3)
        {
            Impact(c,m,obj);
            m.WriteU32(obj+4,m.ReadU32(obj+4)|0x20);
            Call(c,m,0x8001E580,Voice(c,m),BankSound(m,obj),3,obj+0x34);
            // Original 8003FEA8 visibility-gated flash; same native flash service.
            if(Call(c,m,0x8002E22C,obj+0x34,0)!=0)
                Call(c,m,0x8004E338,0x08C0C000);
            uint hit=collision==0?0:m.ReadU32(collision);
            if(hit!=0 && m.ReadU8(hit+8)==2)
            {
                if(S(m,hit+0xA)<0) Call(c,m,0x80015ADC,unchecked((uint)~S(m,hit+0xA)),20);
                if((Call(c,m,0x8002AC5C)*10>>15)==0)
                {
                    uint v=c.SP-0x20;
                    for(uint i=0;i<3;i++)
                    {
                        uint timer=hit+0x12E+i*2;
                        if(m.ReadU16(timer)==0) continue;
                        W(m,v,unchecked((int)(Call(c,m,0x8002AC5C)*3051>>15))-1525);
                        W(m,v+4,-4577);
                        W(m,v+8,unchecked((int)(Call(c,m,0x8002AC5C)*3051>>15))-1525);
                        // Original 8003CE24 selects among twelve source entries.
                        // Retain its RNG consumption, then translate the selected
                        // upgrade to the sequel's common pickup asset table.
                        while((Call(c,m,0x8002AC5C)*12>>15)!=i) { }
                        uint nativeKind=m.ReadU16(0x80063FA4+i*2);
                        Call(c,m,0x8004AA24,nativeKind,hit+0x34,v);
                        m.WriteU16(timer,0);
                    }
                }
            }
            if(Trace) Console.Error.WriteLine($"[V8SourceSpecial] hit shot=0x{obj:X8} target=0x{hit:X8}");
        }
        else if(mode==5)
        {
            Call(c,m,0x800309A0,obj);c.V0=uint.MaxValue;
            if(Trace) Console.Error.WriteLine($"[V8SourceSpecial] animation-end shot=0x{obj:X8}");
        }
    }

    static void Impact(CpuContext c,IMemory m,uint obj)
    {
        // Original FUN_80040234: common root 0x16, with eight root-0x15 sparks.
        int kind=V82VehicleRegistry.OriginalImpactKindForBank(m.ReadU32(obj+0x5C));
        if(kind<0) throw new InvalidOperationException("original special lost source effect metadata");
        uint fx=Call(c,m,0x8002C17C,m.ReadU32(obj+0x5C),(uint)kind,0x80,8);
        if(fx==0) return;
        m.WriteU8(fx+8,7);m.WriteU32(fx+4,0xB4);m.WriteU32(fx,ImpactCallback);
        for(uint i=0;i<12;i+=4) m.WriteU32(fx+0x4C+i,m.ReadU32(obj+0x4C+i));
        Call(c,m,0x8003066C,fx);
    }

    static void ImpactEffect(CpuContext c,IMemory m)
    {
        uint obj=c.A0,mode=c.A1;c.V0=0;
        if(mode==1)
        {
            for(int n=0;n<8;n++)
            {
                uint spark=Call(c,m,0x8002C17C,m.ReadU32(obj+0x5C),(uint)(m.ReadU16(obj+0x1A)-1),0x94,8);
                if(spark==0) continue;
                for(uint i=0;i<12;i+=4) m.WriteU32(spark+0x34+i,0);
                W(m,spark+0x88,unchecked((int)(Call(c,m,0x8002AC5C)<<11))>>15);
                W(m,spark+0x88,I(m,spark+0x88)-1024);
                W(m,spark+0x8C,-(unchecked((int)(Call(c,m,0x8002AC5C)<<12))>>15));
                W(m,spark+0x90,(unchecked((int)(Call(c,m,0x8002AC5C)<<11))>>15)-1024);
                m.WriteU32(spark+4,0x10);
                m.WriteU8(spark+0x86,(byte)((Call(c,m,0x8002AC5C)<<3>>15)+24));
                Call(c,m,0x8002CC48,obj,spark);
            }
        }
        else if(mode==0)
        {
            for(uint child=m.ReadU32(obj+0x10);child!=0;)
            {
                uint next=m.ReadU32(child+0xC);
                byte life=unchecked((byte)(m.ReadU8(child+0x86)-1));m.WriteU8(child+0x86,life);
                if(life==0) { Call(c,m,0x8002CCBC,child);Call(c,m,0x8002C4B4,child); }
                else
                {
                    for(uint i=0;i<12;i+=4) W(m,child+0x34+i,unchecked(I(m,child+0x34+i)+I(m,child+0x88+i)));
                    W(m,child+0x8C,unchecked(I(m,child+0x8C)+56));
                }
                child=next;
            }
        }
        else if(mode==5) {Call(c,m,0x800309A0,obj);c.V0=uint.MaxValue;}
    }

    static void Effect(CpuContext c,IMemory m)
    {
        uint obj=c.A0;
        c.V0=0;
        if(c.A1!=5) return;
        if(m.ReadU32(obj+0x14)!=0)
            Call(c,m,0x800308C4,Call(c,m,0x8002CCBC,obj));
        else Call(c,m,0x800309A0,obj);
        c.V0=uint.MaxValue;
    }
}
