using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>V8 object-layout and N64 service compatibility for Dreamland.</summary>
public static class V82DreamlandCompat
{
    public const uint VehicleHazardDamageAddress = 0x8FFF0100u;
    public const uint ScreenFeedbackAddress = 0x8FFF0104u;
    public const uint ContactEffectAddress = 0x8FFF0108u;
    public const uint FindNearestObjectAddress = 0x8FFF010Cu;
    public const uint PlaySpatialSoundAddress = 0x8FFF0110u;
    public const uint PlaySpatialSoundAltAddress = 0x8FFF0114u;
    public const uint StartSpatialSoundAddress = 0x8FFF0118u;
    public const uint ContactEffectUpdateAddress = 0x8FFF011Cu;
    public const uint LegacyModelEventAddress = 0x8FFF0120u;

    static DreamlandMemory? _memory;

    public static void RegisterHostFunctions()
    {
        Dispatcher.RegisterHostFunction(VehicleHazardDamageAddress, VehicleHazardDamage);
        Dispatcher.RegisterHostFunction(ScreenFeedbackAddress, ScreenFeedback);
        Dispatcher.RegisterHostFunction(ContactEffectAddress, ContactEffect);
        Dispatcher.RegisterHostFunction(FindNearestObjectAddress, FindNearestObject);
        Dispatcher.RegisterHostFunction(PlaySpatialSoundAddress, PlaySpatialSound);
        Dispatcher.RegisterHostFunction(PlaySpatialSoundAltAddress, PlaySpatialSoundAlt);
        Dispatcher.RegisterHostFunction(StartSpatialSoundAddress, StartSpatialSound);
        Dispatcher.RegisterHostFunction(ContactEffectUpdateAddress, ContactEffectUpdate);
        Dispatcher.RegisterHostFunction(LegacyModelEventAddress, LegacyModelEvent);
    }

    public static Action<CpuContext, IMemory> Wrap(Action<CpuContext, IMemory> function) =>
        (c, m) =>
        {
            DreamlandMemory memory = Memory(m);
            memory.RegisterObject(c.A0);
            function(c, memory);
        };

    public static void CallEngine(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        DreamlandMemory memory = Memory(m);
        c.A0 = memory.MapPointer(c.A0);
        c.A1 = memory.MapPointer(c.A1);
        c.A2 = memory.MapPointer(c.A2);
        c.A3 = memory.MapPointer(c.A3);
        function(c, memory.Inner);
    }

    public static void CallObjectFactory(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        CallEngine(c, m, function);
        Memory(m).RegisterObject(c.V0);
    }

    public static void CallObjectReallocator(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        DreamlandMemory memory = Memory(m);
        IMemory raw = memory.Inner;
        uint source = c.S2;
        uint size = c.A1;

        // V8 detaches this model node and reallocates it as a live projectile.
        // Dreamland's converted V8:2 hierarchy is shared level data, so moving
        // that node would corrupt every later spawn. Clone the native-layout
        // node instead, then detach only the clone from its sibling/parent.
        c.A0 = 0u;
        c.A1 = size;
        function(c, raw);
        uint clone = c.V0;
        if (clone == 0u || source == 0u)
            return;

        for (uint offset = 0; offset < size; offset++)
            raw.WriteU8(clone + offset, raw.ReadU8(source + offset));
        raw.WriteU32(clone + 0x0Cu, 0u);
        raw.WriteU32(clone + 0x14u, 0u);
        memory.RegisterObject(clone);
        c.V0 = clone;
    }

    public static void CallObjectModelFactory(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        _ = m;
        _ = function;
        c.V0 = c.A0;
    }

    public static void CallModelNodeFinder(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        CallEngine(c, m, function);
        Memory(m).RegisterObject(c.V0);
    }

    public static void SkipStaticModelDetach(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        _ = m;
        _ = function;
        c.V0 = 0u;
    }

    public static bool PreserveActorModel(CpuContext c, IMemory m)
    {
        _ = c;
        _ = m;
        return true;
    }

    public static bool PreserveAnimatedModel(CpuContext c, IMemory m)
    {
        _ = c;
        _ = m;
        return V82ArenaRegistry.IsDreamlandSelected;
    }

    public static void CallLegacyCooldown(
        CpuContext c,
        IMemory m,
        Action<CpuContext, IMemory> function)
    {
        _ = function;
        uint actor = Memory(m).MapPointer(c.A0);
        IMemory raw = BaseMemory(m);
        ushort timer = raw.ReadU16(actor + 0x1Cu);
        if ((raw.ReadU32(actor + 4u) & 0x8000u) != 0u || timer >= c.A1)
        {
            if (timer >= c.A1)
                raw.WriteU16(actor + 0x1Cu, unchecked((ushort)(timer - c.A1)));
            c.V0 = 0u;
            return;
        }
        raw.WriteU16(actor + 0x1Cu, raw.ReadU16(actor + 0x1Eu));
        c.V0 = 1u;
    }

    public static uint NormalizeLinkedAddress(IMemory m, uint address) =>
        Dispatcher.NormalizeLinkedAddress(Memory(m).Inner, address);

    public static void Dispatch(CpuContext c, IMemory m, uint address)
    {
        DreamlandMemory memory = Memory(m);
        uint runtimeAddress = Dispatcher.ResolveLinkedAddress(
            memory.Inner, address);
        Dispatcher.Call(c, memory.Inner, runtimeAddress);
    }

    public static void DispatchAnimationEvent(
        CpuContext c, IMemory m, uint callback)
    {
        if (V82ArenaRegistry.IsDreamlandSelected && callback < 0x80000000u)
        {
            c.V0 = 0u;
            return;
        }
        Dispatcher.Call(c, m, callback);
    }

    public static void DispatchObjectEvent(
        CpuContext c, IMemory m, uint callback)
    {
        if (!V82ArenaRegistry.IsDreamlandSelected || callback >= 0x80000000u)
        {
            Dispatcher.Call(c, m, callback);
            return;
        }

        uint legacyCallback = BaseMemory(m).ReadU32(c.A0 + 0x64u);
        if (legacyCallback >= 0x80000000u)
        {
            Dispatch(c, m, legacyCallback);
            return;
        }

        c.V0 = 0u;
    }

    static DreamlandMemory Memory(IMemory memory)
    {
        if (memory is DreamlandMemory dreamland)
            return dreamland;
        if (_memory == null || !ReferenceEquals(_memory.Inner, memory))
            _memory = new DreamlandMemory(memory);
        return _memory;
    }

    static IMemory BaseMemory(IMemory memory) =>
        memory is DreamlandMemory dreamland ? dreamland.Inner : memory;

    static void Call(CpuContext c, IMemory m, uint address)
    {
        DreamlandMemory memory = Memory(m);
        c.A0 = memory.MapPointer(c.A0);
        c.A1 = memory.MapPointer(c.A1);
        c.A2 = memory.MapPointer(c.A2);
        c.A3 = memory.MapPointer(c.A3);
        Dispatcher.Call(c, memory.Inner, address);
    }

    static void VehicleHazardDamage(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        int amount = unchecked((int)c.A1);
        uint impulse = c.SP - 0x10u;
        IMemory memory = BaseMemory(m);
        memory.WriteU32(impulse, 0u);
        memory.WriteU32(impulse + 4u, 0u);
        memory.WriteU32(impulse + 8u, 0u);
        c.A1 = unchecked((uint)-Math.Abs(amount));
        c.A2 = impulse;
        c.A3 = 1u;
        Call(c, m, 0x8003A020u);
        c.RA = savedRa;
    }

    static void ScreenFeedback(CpuContext c, IMemory m)
    {
        _ = m;
        c.V0 = 0u;
    }

    static uint TranslateGlobalSoundId(uint n64Id) => n64Id switch
    {
        0x45u => 0x42u, 0x46u => 0x43u, 0x48u => 0x44u,
        0x49u => 0x45u, 0x4Au => 0x46u, 0x4Bu => 0x47u,
        0x4Cu => 0x48u, 0x4Eu => 0x49u, 0x4Fu => 0x4Au,
        0x50u => 0x4Bu, 0x5Fu => 0x4Cu,
        0x8Du => 0x30u, 0x8Eu => 0x27u, 0x97u => 0x35u, 0xA6u => 0x39u,
        _ => throw new InvalidOperationException(
            $"Dreamland requested unmapped N64 global SFX 0x{n64Id:X2}"),
    };

    static void PlaySpatialSound(CpuContext c, IMemory m) =>
        PlaySound(c, m, 0x8001E580u);

    static void PlaySpatialSoundAlt(CpuContext c, IMemory m) =>
        PlaySound(c, m, 0x8001E628u);

    static void StartSpatialSound(CpuContext c, IMemory m) =>
        PlaySound(c, m, 0x8001E098u);

    static void PlaySound(CpuContext c, IMemory m, uint address)
    {
        uint savedRa = c.RA;
        c.A1 = BaseMemory(m).ReadU32(0x8006B41Cu);
        c.A2 = TranslateGlobalSoundId(c.A2);
        Call(c, m, address);
        c.RA = savedRa;
    }

    static void FindNearestObject(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint query = Memory(m).MapPointer(c.A0);
        int requestedId = unchecked((int)c.A1);
        int requestedFamily = -1;
        IMemory raw = BaseMemory(m);
        if (requestedId < 0)
            requestedFamily = unchecked((sbyte)raw.ReadU8(
                0x8006AD74u + unchecked((uint)~requestedId)));

        uint exact = 0u, family = 0u;
        uint exactDistance = uint.MaxValue, familyDistance = uint.MaxValue;
        uint listRoot = raw.ReadU32(0x8006B830u);
        for (uint node = raw.ReadU32(listRoot); node != 0u; node = raw.ReadU32(node))
        {
            uint obj = raw.ReadU32(node + 8u);
            if (obj == 0u || raw.ReadU8(obj + 8u) != 2u ||
                raw.ReadU16(obj + 0x1Cu) == 0u)
                continue;
            c.A0 = query;
            c.A1 = obj + 0x4Cu;
            Dispatcher.Call(c, raw, 0x80029F6Cu);
            uint distance = c.V0;
            int objectId = unchecked((short)raw.ReadU16(obj + 0x1Au));
            if (objectId == requestedId && distance < exactDistance)
            {
                exact = obj;
                exactDistance = distance;
            }
            else if (objectId <= 0)
            {
                int objectFamily = unchecked((sbyte)raw.ReadU8(
                    0x8006AD74u + unchecked((uint)~objectId)));
                if (objectFamily == requestedFamily && distance < familyDistance)
                {
                    family = obj;
                    familyDistance = distance;
                }
            }
        }
        c.V0 = exact != 0u ? exact : family;
        Memory(m).RegisterObject(c.V0);
        c.RA = savedRa;
    }

    static void ContactEffect(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        DreamlandMemory view = Memory(m);
        IMemory raw = view.Inner;
        uint position = view.MapPointer(c.A0);
        uint resource = c.A1;
        uint primary = c.A2 & 0xFFFFu;
        uint secondary = c.A3 & 0xFFFFu;
        uint sound = view.ReadU32(c.SP + 0x10u) & 0xFFFFu;
        uint interval = view.ReadU32(c.SP + 0x14u);
        uint lifetime = view.ReadU32(c.SP + 0x18u);

        c.A0 = resource; c.A1 = primary; c.A2 = 0x9Cu; c.A3 = 8u;
        Dispatcher.Call(c, raw, 0x8002C17Cu);
        uint effect = c.V0;
        view.RegisterObject(effect);
        c.A0 = effect + 0x20u;
        Dispatcher.Call(c, raw, 0x8002A39Cu);
        raw.WriteU32(effect + 0x34u, raw.ReadU32(position));
        raw.WriteU32(effect + 0x38u, raw.ReadU32(position + 4u));
        raw.WriteU32(effect + 0x3Cu, raw.ReadU32(position + 8u));
        raw.WriteU32(effect + 0x18u, ContactEffectUpdateAddress);
        raw.WriteU32(effect + 0x58u, 0x8000u);
        raw.WriteU32(effect + 0x98u, resource);
        raw.WriteU16(effect + 0x96u, (ushort)secondary);
        raw.WriteU32(effect + 4u, raw.ReadU32(effect + 4u) | 0xA4u);
        raw.WriteU16(effect + 0x82u, (ushort)interval);
        c.A0 = effect; Dispatcher.Call(c, raw, 0x800305FCu);
        c.A0 = effect; c.A1 = lifetime; Dispatcher.Call(c, raw, 0x80030CB0u);
        Dispatcher.Call(c, raw, 0x8001DD9Cu);
        c.A0 = c.V0; c.A1 = raw.ReadU32(0x8006B41Cu);
        c.A2 = TranslateGlobalSoundId(sound); c.A3 = position;
        Dispatcher.Call(c, raw, 0x8001E580u);
        c.V0 = effect;
        c.RA = savedRa;
    }

    static void ContactEffectUpdate(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        IMemory raw = BaseMemory(m);
        uint effect = c.A0;
        if (c.A1 == 2u || c.A1 == 5u)
        {
            if (c.A1 == 5u) raw.WriteU32(effect + 0x64u, 0u);
            raw.WriteU16(effect + 0x80u, 0xFFFF);
            c.V0 = 0u; c.RA = savedRa; return;
        }
        if (c.A1 != 0u) { c.V0 = 0u; c.RA = savedRa; return; }
        ushort counter = unchecked((ushort)(raw.ReadU16(effect + 0x80u) - 1));
        raw.WriteU16(effect + 0x80u, counter);
        if (counter == 0xFFFF)
        {
            c.A0 = raw.ReadU32(effect + 0x98u); c.A1 = raw.ReadU16(effect + 0x96u);
            c.A2 = 0x80u; c.A3 = 8u; Dispatcher.Call(c, raw, 0x8002C17Cu);
            uint particle = c.V0;
            int radius = unchecked((int)raw.ReadU32(effect + 0x58u));
            Dispatcher.Call(c, raw, 0x8002AC5Cu);
            raw.WriteU32(particle + 0x34u,
                unchecked((uint)(((unchecked((int)c.V0) * 2L * radius) >> 15) - radius)));
            raw.WriteU32(particle + 0x38u, 0u);
            Dispatcher.Call(c, raw, 0x8002AC5Cu);
            raw.WriteU32(particle + 0x3Cu,
                unchecked((uint)(((unchecked((int)c.V0) * 2L * radius) >> 15) - radius)));
            raw.WriteU32(particle + 0x18u, 0x8004CA10u);
            c.A0 = effect; c.A1 = particle; Dispatcher.Call(c, raw, 0x8002CC48u);
            raw.WriteU16(effect + 0x80u, raw.ReadU16(effect + 0x82u));
        }
        if (raw.ReadU32(effect + 0x10u) == 0u)
        {
            c.A0 = effect; Dispatcher.Call(c, raw, 0x800309A0u);
            raw.WriteU32(effect + 0x64u, 0u);
            raw.WriteU16(effect + 0x80u, 0xFFFF);
        }
        c.V0 = 0u;
        c.RA = savedRa;
    }

    static void LegacyModelEvent(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint eventCode = c.A1;
        uint actor = c.A2;
        IMemory raw = BaseMemory(m);
        Dispatcher.Call(c, raw, 0x8002CD78u);
        uint child = c.V0;
        uint callback = child == 0u ? 0u : raw.ReadU32(child);
        if (callback >= 0x80000000u)
        {
            c.A0 = child;
            c.A1 = eventCode;
            c.A2 = actor;
            Dispatcher.Call(c, raw, callback);
        }
        else
        {
            c.V0 = 0u;
        }
        c.RA = savedRa;
    }

    sealed class DreamlandMemory(IMemory inner) : IMemory
    {
        readonly Dictionary<uint, uint> _objectAddresses = [];
        readonly HashSet<uint> _objectBases = [];
        public IMemory Inner { get; } = inner;

        public void RegisterObject(uint address)
        {
            if (address < 0x80010000u || address >= 0x80800000u || (address & 3u) != 0u)
                return;
            _objectBases.Add(address);
            MapRange(address, 0x00, 0x08, 4);
            MapRange(address, 0x08, 0x02, 0);
            MapRange(address, 0x0A, 0x2A, 0x10);
            MapRange(address, 0x34, 0x04, -0x28);
            MapRange(address, 0x38, 0x04, -0x28);
            MapRange(address, 0x3C, 0x04, -0x28);
            MapRange(address, 0x40, 0x24, 4);
            // V8 keeps the primary actor callback at +0x64; V8:2 moved it to +0x00.
            MapRange(address, 0x64, 0x04, -0x64);
            MapRange(address, 0x68, 0x34, 0);
            MapRange(address, 0x9C, 0x18, 4);
            MapRange(address, 0xB4, 0x1E, 0);
            MapRange(address, 0xD2, 0x16, 0x0C);
        }

        void MapRange(uint address, int start, int length, int delta)
        {
            for (int offset = start; offset < start + length; offset++)
                _objectAddresses[address + (uint)offset] =
                    unchecked(address + (uint)(offset + delta));
        }

        public uint MapPointer(uint address)
        {
            if (_objectBases.Contains(address)) return address;
            if (_objectAddresses.TryGetValue(address, out uint mapped)) return mapped;
            return MapGlobal(address);
        }

        static uint MapGlobal(uint address)
        {
            if (address == 0x800659FCu || address == 0x800759FCu) return 0x8006B814u;
            if (address == 0x80065A10u) return 0x8006B828u;
            if (address == 0x80065A18u) return 0x8006B830u;
            if (address == 0x80065A50u) return 0x8006B868u;
            if (address == 0x800605F8u || address == 0x800737E8u) return 0x8006B41Cu;
            if (address >= 0x800607B4u && address < 0x800647B4u)
                return 0x80065C90u + address - 0x800607B4u;
            return address;
        }

        static bool LooksCallable(uint value) =>
            value == 0u || value >= 0x80000000u;

        bool TryLegacyCallbackField(uint address, out uint baseAddress)
        {
            for (uint offset = 0; offset < 4u; offset++)
            {
                if (address < 0x64u + offset)
                    continue;
                uint candidate = address - 0x64u - offset;
                if (_objectBases.Contains(candidate))
                {
                    baseAddress = candidate;
                    return true;
                }
            }
            baseAddress = 0u;
            return false;
        }

        bool TryLegacyCallbackWord(uint address, out uint baseAddress)
        {
            if (address >= 0x64u)
            {
                uint candidate = address - 0x64u;
                if (_objectBases.Contains(candidate))
                {
                    baseAddress = candidate;
                    return true;
                }
            }
            baseAddress = 0u;
            return false;
        }

        uint A(uint address) => _objectAddresses.TryGetValue(address, out uint mapped)
            ? mapped
            : MapGlobal(address);
        public byte ReadU8(uint address) => TryLegacyCallbackField(address, out _)
            ? Inner.ReadU8(address)
            : Inner.ReadU8(A(address));
        public ushort ReadU16(uint address) => TryLegacyCallbackField(address, out _)
            ? Inner.ReadU16(address)
            : Inner.ReadU16(A(address));
        public uint ReadU32(uint address)
        {
            uint mapped = A(address);
            if (TryLegacyCallbackWord(address, out uint baseAddress))
            {
                uint callback = Inner.ReadU32(baseAddress);
                mapped = LooksCallable(callback) ? baseAddress : address;
            }
            uint value = Inner.ReadU32(mapped);
            RegisterObject(value);
            return value;
        }
        public void WriteU8(uint address, byte value) =>
            Inner.WriteU8(TryLegacyCallbackField(address, out _) ? address : A(address), value);
        public void WriteU16(uint address, ushort value) =>
            Inner.WriteU16(TryLegacyCallbackField(address, out _) ? address : A(address), value);
        public void WriteU32(uint address, uint value)
        {
            if (TryLegacyCallbackWord(address, out uint baseAddress))
            {
                Inner.WriteU32(LooksCallable(value) ? baseAddress : address, value);
                return;
            }
            Inner.WriteU32(A(address), value);
        }
        public void WriteGteWord(uint address, int register) => Inner.WriteGteWord(A(address), register);
        public void WritePreciseGteVertex(uint address, in PreciseGteVertexData vertex) =>
            Inner.WritePreciseGteVertex(A(address), vertex);
        public bool TryGetPreciseGteVertex(uint address, uint packed,
            out PreciseGteVertexData vertex) =>
            Inner.TryGetPreciseGteVertex(A(address), packed, out vertex);
        public uint ReadWordLeft(uint current, uint address) => Inner.ReadWordLeft(current, A(address));
        public uint ReadWordRight(uint current, uint address) => Inner.ReadWordRight(current, A(address));
        public void WriteWordLeft(uint address, uint value) => Inner.WriteWordLeft(A(address), value);
        public void WriteWordRight(uint address, uint value) => Inner.WriteWordRight(A(address), value);
        public void LoadBytes(uint address, byte[] data) => Inner.LoadBytes(A(address), data);
        public void ZeroRange(uint address, uint length) => Inner.ZeroRange(A(address), length);
        public void SetCd(CdController cd) => Inner.SetCd(cd);
    }
}
