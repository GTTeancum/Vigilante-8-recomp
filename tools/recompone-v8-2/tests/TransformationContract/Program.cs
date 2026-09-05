using System.Text.Json;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hle;

namespace Recompiled;

public static partial class Vigilante82PC
{
    static int checks;
    static void Check(bool value, string name)
    {
        if (!value) throw new Exception(name);
        checks++;
    }

    // Only the GPU fill call is stubbed; row routing/table creation below is
    // the same source compiled into the game. Native visual QA is separate.
    static void func_8001ADF8(CpuContext c, IMemory m) { }

    public static void Main()
    {
        Check(new GameConfig().V82Transformations == V82TransformationMode.All,
            "unchanged native policy is default");
        Check(JsonSerializer.Deserialize<GameConfig>("{}")!.V82Transformations == V82TransformationMode.All,
            "legacy config retains original behavior");
        foreach (var mode in Enum.GetValues<V82TransformationMode>())
        {
            var config = new GameConfig { V82Transformations = mode };
            var restored = JsonSerializer.Deserialize<GameConfig>(JsonSerializer.Serialize(config))!;
            Check(restored.V82Transformations == mode, "config round trip " + mode);
            Check(V82TransformationSettings.Cycle(V82TransformationSettings.Cycle(mode, 1), -1) == mode,
                "bidirectional cycling " + mode);
            Check(V82TransformationSettings.Cycle(mode, 0) == mode, "zero input " + mode);
            Check(V82TransformationSettings.AllowsPowerups(mode) == (mode == V82TransformationMode.All),
                "powerup policy " + mode);
        }
        Check(V82TransformationSettings.Normalize((V82TransformationMode)99) == V82TransformationMode.All,
            "invalid settings use native default");
        Check(V82TransformationSettings.Label(V82TransformationMode.AutoWaterski) == "Auto Waterski",
            "requested visible label");

        var m = new PSMemory();
        var c = new CpuContext { SP = 0x801F0000 };
        const uint original = 0x80115E60;
        for (uint i = 0; i < 7; i++) m.WriteU32(original + i * 4, 0x80110000 + i * 16);
        uint[] expected = [0x80110000, 0x80110010, 0x80110020, 0x80110030, 0x80110040,
            0x8011B230, 0x8011B240, 0x80110050, 0x80110060];
        for (uint selected = 0; selected < 9; selected++)
        {
            c.S4 = selected;
            V82NativeVideoOption.AdjustLayout(c, m);
            uint top = selected > 6 ? selected - 6 : 0;
            Check(c.S4 == selected - top, "visible highlight " + selected);
            for (uint row = 0; row < 7; row++)
                Check(m.ReadU32(c.S2 + row * 4) == expected[top + row],
                    $"scroll window selected={selected} row={row}");
        }
        for (uint index = 0; index < 9; index++)
        {
            c.S1 = index; c.V0 = 1;
            V82NativeVideoOption.ExtendCursorRange(c, m);
            // Emulate the unmodified retail clamp immediately after the hook.
            if (c.S1 < 6) c.S1++;
            Check(c.S1 == Math.Min(index + 1, 8u), "down advances exactly once " + index);
            c.S1 = index; c.S3 = 0x80101180;
            V82NativeVideoOption.WidenDispatch(c, m);
            Check(c.V0 == 1, "all nine rows dispatch " + index);
            Check(c.S3 == (index > 6 ? 0x80101178u : 0x80101180u),
                "retail page remap " + index);
        }
        // A new shell may have cleared scratch memory while the host persists.
        m.WriteU32(0x8011B214, 0);
        c.S4 = 5;
        V82NativeVideoOption.AdjustLayout(c, m);
        Check(m.ReadU32(0x8011B214) == 0x8011B230, "table restored after shell reload");

        const uint vehicle = 0x80020000, pickup = 0x80021000, collision = 0x80022000;
        m.WriteU32(collision, vehicle);
        m.WriteU8(vehicle + 0xDC, 0); // Stock vehicle, not a guest special case.
        foreach (var mode in Enum.GetValues<V82TransformationMode>())
        {
            ConfigManager.Game.V82Transformations = mode;
            foreach (ushort kind in new ushort[] { 0, 6, 7, 8, 9, 10, 12 })
            {
                c.A0 = pickup; c.A1 = 3; c.A2 = collision; c.V0 = 99;
                m.WriteU16(pickup + 0x1A, kind);
                bool allowed = V82VehicleRegistry.RejectUnsupportedTransformationPickup(c, m);
                bool expectedAllowed = mode == V82TransformationMode.All || kind is < 7 or > 9;
                Check(allowed == expectedAllowed, $"pickup kind={kind} policy={mode}");
                Check(m.ReadU16(pickup + 0x1A) == kind && m.ReadU32(collision) == vehicle,
                    "rejected pickup does not mutate either participant");
                Check(allowed || c.V0 == 0, "native not-collected result");
            }
            foreach (uint nativeMode in new uint[] { 0, 1, 2, 3 })
            {
                c.A0 = vehicle; c.A1 = nativeMode;
                Check(V82VehicleRegistry.RejectUnsupportedTransformationActivation(c, m) ==
                    (nativeMode == 0 || mode == V82TransformationMode.All),
                    $"direct activation native={nativeMode} policy={mode}");
            }
        }
        int ground = 0x120000, calls = 0;
        bool narrowCrossing = false;
        Dispatcher.RegisterHostFunction(0x8001B750, (ctx, mem) => ctx.V0 = narrowCrossing
            ? (ctx.A0 >= 0x8000 && ctx.A0 <= 0x10000 ? 0x120000u : 0xE0000u)
            : unchecked((uint)ground));
        Dispatcher.RegisterHostFunction(0x8003E32C, (ctx, mem) =>
        {
            Check(V82VehicleRegistry.RejectUnsupportedTransformationActivation(ctx, mem),
                "controller request passes real policy/capability gate");
            Check(ctx.A2 == 0, "auto grants no pickup damage shield");
            Check(!V82TransformationPolicy.AllowsActivation(ctx.A0 + 0x1000, 2),
                "automatic authority cannot transform another vehicle");
            calls++;
            mem.WriteU8(ctx.A0 + 0xAC, (byte)ctx.A1);
            mem.WriteU16(ctx.A0 + 0xB4, 32);
        });
        c.GP = 0x80070000; c.A0 = vehicle; c.A1 = 0x1234; c.A2 = collision;
        m.WriteU8(vehicle + 8, 2); m.WriteU16(vehicle + 0x1C, 500);
        m.WriteU32(vehicle + 0x38, 0xF8000); m.WriteU32(vehicle + 0x3C, 0x10000);
        m.WriteU32(c.GP + 0xDB0, 0x100000); m.WriteU32(c.GP + 0xDA0, 0x20000);
        ConfigManager.Game.V82Transformations = V82TransformationMode.AutoWaterski;
        GpuHle.GameplayActive = true;
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == 1 && m.ReadU8(vehicle + 0xAC) == 2, "water approach equips native float mode");
        Check(c.A0 == vehicle && c.A1 == 0x1234 && c.A2 == collision, "native calls preserve caller registers");
        Check(!V82TransformationPolicy.AllowsActivation(vehicle, 2), "authority ends with request scope");
        for (int i = 0; i < 40; i++) V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == 1, "busy animation never restarts");
        m.WriteU16(vehicle + 0xB4, 0);
        ground = 0xE0000; m.WriteU32(vehicle + 0x38, 0xD8000);
        for (int i = 0; i < 19; i++) V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == 1, "shoreline dry debounce");
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == 2 && m.ReadU8(vehicle + 0xAC) == 0, "sustained land retracts");
        m.WriteU16(vehicle + 0xB4, 0);
        m.WriteU8(vehicle + 0xAC, 3);
        ConfigManager.Game.V82Transformations = V82TransformationMode.None;
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == 3 && m.ReadU8(vehicle + 0xAC) == 0, "None removes active transformations");
        m.WriteU16(vehicle + 0xB4, 0);
        V82AutoWaterski.Reset();
        ConfigManager.Game.V82Transformations = V82TransformationMode.All;
        m.WriteU8(vehicle + 0xAC, 3);
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == 3 && m.ReadU8(vehicle + 0xAC) == 3, "All preserves native transformation");

        // Reallocation/respawn must not inherit the previous owner's dry
        // debounce or ownership. These exercise the real lifecycle APIs.
        ConfigManager.Game.V82Transformations = V82TransformationMode.AutoWaterski;
        m.WriteU8(vehicle + 0xAC, 0);
        ground = 0x120000; m.WriteU32(vehicle + 0x38, 0xF8000);
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        m.WriteU16(vehicle + 0xB4, 0);
        ground = 0xE0000; m.WriteU32(vehicle + 0x38, 0xD8000);
        for (int i = 0; i < 19; i++) V82AutoWaterski.BeforeVehiclePhysics(c, m);
        int lifecycleCalls = calls;
        V82AutoWaterski.Forget(vehicle);
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls, "new object never inherits prior dry debounce");
        for (int i = 0; i < 19; i++) V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls + 1 && m.ReadU8(vehicle + 0xAC) == 0,
            "fresh dry interval retracts restored transformation");
        m.WriteU16(vehicle + 0xB4, 0);
        ground = 0x120000; m.WriteU32(vehicle + 0x38, 0xF8000);
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        lifecycleCalls = calls;
        m.WriteU16(vehicle + 0xB4, 0);
        m.WriteU16(vehicle + 0x1C, 0);
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls, "dead vehicle never gets automatic requests");
        m.WriteU16(vehicle + 0x1C, 500);
        ConfigManager.Game.V82Transformations = V82TransformationMode.All;
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls, "dead owner cannot leak cleanup into native All policy");
        V82AutoWaterski.Reset();
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls, "match reset leaves native All behavior untouched");

        Check(!V82AutoWaterski.SurfaceRisk(0, 100, 0, 999, 0, 999, true), "inactive plane");
        Check(!V82AutoWaterski.SurfaceRisk(0x100000, 0, 0, 0x100000, 1, 0x120000, false),
            "no hazard outside native Z boundary");
        Check(V82AutoWaterski.SurfaceRisk(0x100000, 0, 0, 0x100000, 1, 0x120000, true),
            "converted region hazard independent of native boundary");
        Check(!V82AutoWaterski.SurfaceRisk(0x100000, 0, 0, 0x80000, -1, 0x120000, false),
            "bridge/high airborne vehicle not classified at surface");
        Check(!V82AutoWaterski.SurfaceRisk(0x100000, 0, 0, 0x100000, -1, 0xF0000, false),
            "land above water plane");

        // Actual controller look-ahead must find water crossed between the
        // starting point and end of the animation, not merely at its endpoint.
        V82AutoWaterski.Reset();
        ConfigManager.Game.V82Transformations = V82TransformationMode.AutoWaterski;
        m.WriteU8(vehicle + 0xAC, 0); m.WriteU16(vehicle + 0xB4, 0);
        m.WriteU32(vehicle + 0x34, 0); m.WriteU32(vehicle + 0x38, 0xF8000);
        m.WriteU32(vehicle + 0x80, 0x80000); // 0x1000 displacement per physics tick
        narrowCrossing = true;
        lifecycleCalls = calls;
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls + 1 && m.ReadU8(vehicle + 0xAC) == 2,
            "approach detects narrow hazardous crossing before native transition");
        Check(m.ReadU32(vehicle + 0x34) == 0 && m.ReadU32(vehicle + 0x80) == 0x80000,
            "look-ahead never moves or accelerates vehicle");

        // Converted arenas share RECT hazards even outside the native Z test.
        V82AutoWaterski.Reset();
        m.WriteU8(vehicle + 0xAC, 0); m.WriteU16(vehicle + 0xB4, 0);
        m.WriteU32(vehicle + 0x80, 0); m.WriteU32(c.GP + 0xDA0, 0);
        const uint regionNode = 0x80024000, regionEnd = 0x80024020;
        m.WriteU32(c.GP + 0x10D8, regionNode); m.WriteU32(regionNode, regionEnd);
        m.WriteU16(regionNode + 8, 0x43); m.WriteU16(regionNode + 0xA, 0xFFFF);
        m.WriteU16(regionNode + 0xC, 0); m.WriteU16(regionNode + 0xE, 0);
        m.WriteU16(regionNode + 0x10, 2); m.WriteU16(regionNode + 0x12, 2);
        narrowCrossing = false; ground = 0x120000;
        lifecycleCalls = calls;
        V82AutoWaterski.BeforeVehiclePhysics(c, m);
        Check(calls == lifecycleCalls + 1 && m.ReadU8(vehicle + 0xAC) == 2,
            "real RECT traversal activates outside native boundary");
        Check(V82AutoWaterski.HasWaterMovement(m, vehicle), "native water mode exempts imported drowning");
        CheckWaterFit();
        Console.WriteLine($"PASS: {checks} transformation settings / OPTIONS / water-fit assertions");
    }
}
