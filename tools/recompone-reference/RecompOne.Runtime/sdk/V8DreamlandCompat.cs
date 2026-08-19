using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// PS1 implementations of the three platform-dependent services used by the
/// original N64 Dreamland overlay. All gameplay callbacks themselves execute
/// as recompiled native overlay code.
/// </summary>
public static class V8DreamlandCompat
{
    public const uint VehicleHazardDamageAddress = 0x8FFF0100u;
    public const uint ScreenFeedbackAddress = 0x8FFF0104u;
    public const uint ContactEffectAddress = 0x8FFF0108u;
    public const uint FindNearestObjectAddress = 0x8FFF010Cu;
    public const uint PlaySpatialSoundAddress = 0x8FFF0110u;
    public const uint PlaySpatialSoundAltAddress = 0x8FFF0114u;
    public const uint StartSpatialSoundAddress = 0x8FFF0118u;
    public const uint ContactEffectUpdateAddress = 0x8FFF011Cu;
    [ThreadStatic]
    private static int _animationAbortResult;

    public static void RegisterHostFunctions()
    {
        Dispatcher.RegisterHostFunction(
            VehicleHazardDamageAddress, VehicleHazardDamage);
        Dispatcher.RegisterHostFunction(
            ScreenFeedbackAddress, ScreenFeedback);
        Dispatcher.RegisterHostFunction(
            ContactEffectAddress, ContactEffect);
        Dispatcher.RegisterHostFunction(
            FindNearestObjectAddress, FindNearestObject);
        Dispatcher.RegisterHostFunction(
            PlaySpatialSoundAddress, PlaySpatialSound);
        Dispatcher.RegisterHostFunction(
            PlaySpatialSoundAltAddress, PlaySpatialSoundAlt);
        Dispatcher.RegisterHostFunction(
            StartSpatialSoundAddress, StartSpatialSound);
        Dispatcher.RegisterHostFunction(
            ContactEffectUpdateAddress, ContactEffectUpdate);
    }

    /// <summary>
    /// Back-port the N64 engine's extended animation-tree return contract for
    /// Dreamland. A callback result of -1 skips only the current object's
    /// children; a result below -1 aborts the complete current sibling walk.
    ///
    /// Dreamland returns -2 after replacing an actor hierarchy from inside an
    /// animated child callback. The retail PS1 traversal caches the old
    /// sibling before invoking that callback and would otherwise continue
    /// through the freed hierarchy. This is the exact behavior implemented by
    /// N64 main-code function 0x8013A330.
    /// </summary>
    public static bool UpdateAnimationSiblingChain(CpuContext c, IMemory m)
    {
        if (V8ArenaRegistry.SelectedStableId != "n64.super_dreamland_64")
            return true;

        uint savedReturnAddress = c.RA;
        UpdateAnimationSiblingChainCore(c, m, c.A0, (ushort)c.A1);
        c.RA = savedReturnAddress;
        return false;
    }

    public static void BeginAnimationObject()
    {
        if (V8ArenaRegistry.SelectedStableId == "n64.super_dreamland_64")
            _animationAbortResult = 0;
    }

    public static void CaptureAnimationCallbackReturn(CpuContext c)
    {
        if (V8ArenaRegistry.SelectedStableId != "n64.super_dreamland_64" ||
            c.RA != 0x8001FA74u)
            return;
        int result = unchecked((int)c.V0);
        if (result < -1)
            _animationAbortResult = result;
    }

    /// <summary>
    /// Native PS1 back-port of N64 main-code function 0x8013C1D0. Dreamland
    /// uses its exact/family-filtered nearest-object query for mushroom
    /// projectiles. Retail PS1 only has the simpler 0x80021DB0 query.
    /// </summary>
    private static void FindNearestObject(CpuContext c, IMemory m)
    {
        uint savedReturnAddress = c.RA;
        uint queryPosition = c.A0;
        int requestedId = unchecked((int)c.A1);
        int requestedFamily = -1;
        if (requestedId < 0)
        {
            uint familyIndex = unchecked((uint)~requestedId);
            requestedFamily = unchecked(
                (sbyte)m.ReadU8(0x80065674u + familyIndex));
        }

        uint exactObject = 0u;
        uint familyObject = 0u;
        uint exactDistance = uint.MaxValue;
        uint familyDistance = uint.MaxValue;
        uint listRoot = m.ReadU32(0x80065A18u);
        for (uint node = m.ReadU32(listRoot);
             node != 0u;
             node = m.ReadU32(node))
        {
            uint obj = m.ReadU32(node + 8u);
            if (obj == 0u ||
                m.ReadU8(obj + 4u) != 2u ||
                m.ReadU16(obj + 0x0Cu) == 0u)
                continue;

            c.A0 = queryPosition;
            c.A1 = obj + 0x48u;
            c.RA = FindNearestObjectAddress;
            Dispatcher.Call(c, m, 0x80016AACu);
            uint distance = c.V0;
            int objectId = unchecked((short)m.ReadU16(obj + 6u));
            if (objectId == requestedId)
            {
                if (distance < exactDistance)
                {
                    exactDistance = distance;
                    exactObject = obj;
                }
                continue;
            }

            if (objectId > 0)
                continue;
            uint familyIndex = unchecked((uint)~objectId);
            int objectFamily = unchecked(
                (sbyte)m.ReadU8(0x80065674u + familyIndex));
            if (objectFamily == requestedFamily &&
                distance < familyDistance)
            {
                familyDistance = distance;
                familyObject = obj;
            }
        }

        c.V0 = exactObject != 0u ? exactObject : familyObject;
        c.RA = savedReturnAddress;
    }

    public static void EndAnimationObject(CpuContext c)
    {
        if (_animationAbortResult >= -1)
            return;
        c.V0 = unchecked((uint)_animationAbortResult);
        _animationAbortResult = 0;
    }

    private static void UpdateAnimationSiblingChainCore(
        CpuContext c, IMemory m, uint first, ushort frame)
    {
        uint previous = 0u;
        int previousResult = 0;
        for (uint current = first; current != 0u;)
        {
            if (current < 0x80000000u || current >= 0x80200000u)
                throw new InvalidOperationException(
                    $"Dreamland animation hierarchy escaped RAM: " +
                    $"first=0x{first:X8} current=0x{current:X8} " +
                    $"previous=0x{previous:X8} " +
                    $"previousResult={previousResult}");
            uint next = m.ReadU32(current + 0x34u);
            int result = 0;
            if ((m.ReadU32(current) & 4u) == 0u)
            {
                uint animation = m.ReadU32(current + 0x60u);
                if (animation >= 0x00200000u &&
                    (animation < 0x80000000u || animation >= 0x80200000u))
                    throw new InvalidOperationException(
                        $"Dreamland animation object is corrupt: " +
                        $"first=0x{first:X8} current=0x{current:X8} " +
                        $"previous=0x{previous:X8} id={m.ReadU16(current + 0x0Au)} " +
                        $"parent=0x{m.ReadU32(current + 0x3Cu):X8} " +
                        $"next=0x{next:X8} animation=0x{animation:X8} " +
                        $"callback=0x{m.ReadU32(current + 0x64u):X8} " +
                        $"previousResult={previousResult}");
                c.A0 = current;
                c.A1 = frame;
                c.RA = 0x8001FC70u;
                Dispatcher.Call(c, m, 0x8001F9CCu);
                result = unchecked((int)c.V0);
                if (result < -1)
                    return;
                if (result >= 0)
                {
                    uint child = m.ReadU32(current + 0x38u);
                    if (child != 0u)
                        UpdateAnimationSiblingChainCore(c, m, child, frame);
                }
            }
            previous = current;
            previousResult = result;
            current = next;
        }
    }

    /// <summary>
    /// N64's two-argument terrain-hazard damage wrapper. PS1 exposes the same
    /// damage path with an explicit impulse vector and sign convention.
    /// </summary>
    private static void VehicleHazardDamage(CpuContext c, IMemory m)
    {
        uint vehicle = c.A0;
        int amount = unchecked((int)c.A1);
        uint impulse = c.SP - 0x10u;
        m.WriteU32(impulse, 0u);
        m.WriteU32(impulse + 4u, 0u);
        m.WriteU32(impulse + 8u, 0u);
        c.A0 = vehicle;
        c.A1 = unchecked((uint)-Math.Abs(amount));
        c.A2 = impulse;
        c.A3 = 1u;
        Dispatcher.Call(c, m, 0x8002C958u);
    }

    /// <summary>
    /// Translate Dreamland's N64 global SFX indices to native PS1 MAIN.SND
    /// entries. IDs 0x42-0x4B are append-only SPU ADPCM conversions in the
    /// Dreamland loose bank. The four retail-bank mappings are waveform
    /// matches with identical trimmed lengths and 0.992-0.999 spectra.
    /// </summary>
    private static uint TranslateGlobalSoundId(uint n64Id)
    {
        return n64Id switch
        {
            0x45u => 0x42u,
            0x46u => 0x43u,
            0x48u => 0x44u,
            0x49u => 0x45u,
            0x4Au => 0x46u,
            0x4Bu => 0x47u,
            0x4Cu => 0x48u,
            0x4Eu => 0x49u,
            0x4Fu => 0x4Au,
            0x50u => 0x4Bu,
            0x5Fu => 0x4Cu,
            0x8Du => 0x30u,
            0x8Eu => 0x27u,
            0x97u => 0x35u,
            0xA6u => 0x39u,
            _ => throw new InvalidOperationException(
                $"Dreamland requested unmapped N64 global SFX " +
                $"0x{n64Id:X2}."),
        };
    }

    private static void PlaySpatialSound(CpuContext c, IMemory m)
    {
        uint savedReturnAddress = c.RA;
        c.A1 = m.ReadU32(c.GP + 0x5F8u);
        c.A2 = TranslateGlobalSoundId(c.A2);
        c.RA = PlaySpatialSoundAddress;
        Dispatcher.Call(c, m, 0x800447E8u);
        c.RA = savedReturnAddress;
    }

    private static void StartSpatialSound(CpuContext c, IMemory m)
    {
        uint savedReturnAddress = c.RA;
        c.A1 = m.ReadU32(c.GP + 0x5F8u);
        c.A2 = TranslateGlobalSoundId(c.A2);
        c.RA = StartSpatialSoundAddress;
        Dispatcher.Call(c, m, 0x800443C8u);
        c.RA = savedReturnAddress;
    }

    private static void PlaySpatialSoundAlt(CpuContext c, IMemory m)
    {
        uint savedReturnAddress = c.RA;
        c.A1 = m.ReadU32(c.GP + 0x5F8u);
        c.A2 = TranslateGlobalSoundId(c.A2);
        c.RA = PlaySpatialSoundAltAddress;
        Dispatcher.Call(c, m, 0x8004483Cu);
        c.RA = savedReturnAddress;
    }

    /// <summary>
    /// Dreamland calls N64 main-code function 0x80159900 once per frame to
    /// draw its RECT water surface. The arena converter now emits that surface
    /// as a native PS1 XOBF model and ANM texture cycle, so the translated
    /// overlay call has no remaining runtime work.
    /// </summary>
    private static void ScreenFeedback(CpuContext c, IMemory m)
    {
        _ = m;
        c.V0 = 0u;
    }

    /// <summary>
    /// Native PS1 back-port of N64 main-code function 0x80159E54. This is the
    /// same emitter implemented by Hoover Dam's PS1 overlay at 0x80100B40,
    /// extended with the N64 helper's explicit sound argument.
    /// </summary>
    private static void ContactEffect(CpuContext c, IMemory m)
    {
        uint savedReturnAddress = c.RA;
        uint position = c.A0;
        uint resource = c.A1;
        uint primaryModel = c.A2 & 0xFFFFu;
        uint secondaryModel = c.A3 & 0xFFFFu;
        uint soundId = m.ReadU32(c.SP + 0x10u) & 0xFFFFu;
        uint interval = m.ReadU32(c.SP + 0x14u);
        uint lifetime = m.ReadU32(c.SP + 0x18u);

        c.A0 = resource;
        c.A1 = primaryModel;
        c.A2 = 0x9Cu;
        c.A3 = 8u;
        c.RA = ContactEffectAddress;
        Dispatcher.Call(c, m, 0x8001AC44u);
        uint effect = c.V0;

        c.A0 = effect + 0x10u;
        c.RA = ContactEffectAddress;
        Dispatcher.Call(c, m, 0x80016DA8u);
        m.WriteU32(effect + 0x24u, m.ReadU32(position));
        m.WriteU32(effect + 0x28u, m.ReadU32(position + 4u));
        m.WriteU32(effect + 0x2Cu, m.ReadU32(position + 8u));
        m.WriteU32(effect + 0x64u, ContactEffectUpdateAddress);
        m.WriteU32(effect + 0x54u, 0x8000u);
        m.WriteU32(effect + 0x98u, resource);
        m.WriteU16(effect + 0x96u, (ushort)secondaryModel);
        m.WriteU32(effect, m.ReadU32(effect) | 0xA4u);
        m.WriteU16(effect + 0x82u, (ushort)interval);

        c.A0 = effect;
        c.RA = ContactEffectAddress;
        Dispatcher.Call(c, m, 0x800202F4u);
        c.A0 = effect;
        c.A1 = lifetime;
        c.RA = ContactEffectAddress;
        Dispatcher.Call(c, m, 0x80020890u);

        c.RA = ContactEffectAddress;
        Dispatcher.Call(c, m, 0x8004410Cu);
        c.A0 = c.V0;
        c.A1 = m.ReadU32(c.GP + 0x5F8u);
        c.A2 = TranslateGlobalSoundId(soundId);
        c.A3 = position;
        c.RA = ContactEffectAddress;
        Dispatcher.Call(c, m, 0x800447E8u);

        c.V0 = effect;
        c.RA = savedReturnAddress;
    }

    /// <summary>
    /// Native PS1 back-port of Hoover Dam callback 0x80100A18 / N64 main-code
    /// callback 0x80159D24. It emits the secondary authored model at the exact
    /// interval until the scheduled parent effect expires.
    /// </summary>
    private static void ContactEffectUpdate(CpuContext c, IMemory m)
    {
        uint savedReturnAddress = c.RA;
        uint effect = c.A0;
        uint eventCode = c.A1;
        if (eventCode == 2u)
        {
            m.WriteU16(effect + 0x80u, 0xFFFF);
            c.V0 = 0u;
            c.RA = savedReturnAddress;
            return;
        }
        if (eventCode == 5u)
        {
            m.WriteU32(effect + 0x60u, 0u);
            m.WriteU16(effect + 0x80u, 0xFFFF);
            c.V0 = 0u;
            c.RA = savedReturnAddress;
            return;
        }
        if (eventCode != 0u)
        {
            c.V0 = 0u;
            c.RA = savedReturnAddress;
            return;
        }

        ushort counter = unchecked(
            (ushort)(m.ReadU16(effect + 0x80u) - 1));
        m.WriteU16(effect + 0x80u, counter);
        if (counter == 0xFFFF)
        {
            c.A0 = m.ReadU32(effect + 0x98u);
            c.A1 = m.ReadU16(effect + 0x96u);
            c.A2 = 0x80u;
            c.A3 = 8u;
            c.RA = ContactEffectUpdateAddress;
            Dispatcher.Call(c, m, 0x8001AC44u);
            uint particle = c.V0;
            int radius = unchecked((int)m.ReadU32(effect + 0x54u));

            c.RA = ContactEffectUpdateAddress;
            Dispatcher.Call(c, m, 0x80017160u);
            int randomX = unchecked((int)c.V0);
            m.WriteU32(
                particle + 0x24u,
                unchecked((uint)(((randomX * 2L * radius) >> 15) - radius)));
            m.WriteU32(particle + 0x28u, 0u);

            c.RA = ContactEffectUpdateAddress;
            Dispatcher.Call(c, m, 0x80017160u);
            int randomZ = unchecked((int)c.V0);
            m.WriteU32(
                particle + 0x2Cu,
                unchecked((uint)(((randomZ * 2L * radius) >> 15) - radius)));
            m.WriteU32(particle + 0x64u, 0x8003E80Cu);

            c.A0 = effect;
            c.A1 = particle;
            c.RA = ContactEffectUpdateAddress;
            Dispatcher.Call(c, m, 0x8001D4F0u);
            m.WriteU16(effect + 0x80u, m.ReadU16(effect + 0x82u));
        }

        if (m.ReadU32(effect + 0x38u) != 0u)
        {
            c.V0 = 0u;
            c.RA = savedReturnAddress;
            return;
        }
        c.A0 = effect;
        c.RA = ContactEffectUpdateAddress;
        Dispatcher.Call(c, m, 0x800205F8u);
        // Hoover Dam's native PS1 callback continues through 0x80100B18:
        // clear animation, disarm the interval, and return zero even after
        // the object frees itself. Returning -1 here changed the animation
        // sibling-walk contract and left Dreamland effect teardown divergent.
        m.WriteU32(effect + 0x60u, 0u);
        m.WriteU16(effect + 0x80u, 0xFFFF);
        c.V0 = 0u;
        c.RA = savedReturnAddress;
    }

}
