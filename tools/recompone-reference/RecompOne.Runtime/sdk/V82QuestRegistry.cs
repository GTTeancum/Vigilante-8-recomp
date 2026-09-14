using System.Buffers.Binary;
using System.Text.Json;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Host;

namespace RecompOne.Runtime.Sdk;

public static class V82QuestRegistry
{
    static QuestCatalog? catalog;
    static NativeQuestImage? image;
    static uint imageBase;
    static uint progressBase;
    static uint resultTextBase;
    static byte[]? nativeHeader;
    static readonly Dictionary<string, byte[]> saved = new(StringComparer.Ordinal);
    static string? savePath;
    const uint NativeProgress = 0x8006B9E8u;
    const int NativeCount = 18;
    const int RecordBytes = 10;
    const int ProgressSlots = 3;
    const int TypeLimit = 128;
    static readonly string? ProbeResult = Environment.GetEnvironmentVariable("RECOMPONE_QUEST_TEST_RESULT");
    static bool probeTriggered;

    public static void ProbeNativeProgress(IMemory m)
    {
        if (Environment.GetEnvironmentVariable("RECOMPONE_QUEST_TEST_NATIVE_PROGRESS") != "1") return;
        // Process-local completed-save fixture. Exercise the native campaign
        // tier logic itself, rather than bypassing the carousel eligibility mask.
        for (uint slot = 0; slot < 2; slot++)
            for (uint type = 0; type < NativeCount; type++)
                m.WriteU16(NativeProgress + slot * 180 + type * RecordBytes, 0x1FF);
    }

    // Explicit process-local state-machine fixture, never active in normal play.
    // It tests results/progression; it does not stand in for combat/objective tests.
    public static void ProbeGameplay(CpuContext c, IMemory m)
    {
        if (IsQuest(c, m) && m.ReadU32(c.GP + 0xC54u) == 0
            && Environment.GetEnvironmentVariable("RECOMPONE_TRACE_QUEST") == "1")
        {
            uint player = m.ReadU32(0x8006B7E8u);
            Console.WriteLine($"[QuestPlayer] type={CurrentType(m)} id={(short)m.ReadU16(player + 0xAu)}");
        }
        if (ProbeResult is not ("win" or "lose" or "objective-fail")
            || !IsQuest(c, m) || m.ReadU32(c.GP + 0xC54u) < 600) return;
        if (ProbeResult is "win" or "objective-fail")
            V82Compat.CompleteQuestCombatFixture(c, m);
        if (probeTriggered) return;
        probeTriggered = true;
        // Rendering fixtures need the whole native victory transition:
        // 800356AC sets the camera's 0x02000000 flag as well as this timer.
        // Let the native combat/objective check do both after the fixture has
        // defeated the opponents. Normal progression fixtures keep their
        // existing direct result entry, including the explicit lose case.
        bool nativeVictory = ProbeResult is "win" or "objective-fail" &&
            Environment.GetEnvironmentVariable("RECOMPONE_QUEST_TEST_NATIVE_RESULT_TRANSITION") == "1";
        if (!nativeVictory) m.WriteU32(c.GP + 0xC74u, 1);
        Console.WriteLine($"[QuestFixture] result={ProbeResult} type={CurrentType(m)} mission={m.ReadU8(c.GP + 0xC3Cu)}");
    }

    public static void ProbeResultEntry(CpuContext c, IMemory m)
    {
        if (IsQuest(c, m)) InputManager.SignalScriptStage("quest_result");
        if (probeTriggered) m.WriteU8(c.GP + 0x33u, (byte)(ProbeResult == "lose" ? 0 : 1));
    }

    public static void ProbeObjectives(CpuContext c)
    {
        if (probeTriggered) c.V0 = ProbeResult == "win" ? 3u : 1u;
    }

    public static void ResultText(CpuContext c, IMemory m)
    {
        if (resultTextBase == 0 || image?.Campaigns.ContainsKey(CurrentType(m)) != true) return;
        c.A1 = resultTextBase;
        // Native row B (bit 1) is scenery; row A (bit 0) is collection.
        c.A2 = c.A3;
        // Original quests have scenery plus enemies, with no collection goal.
        // This native branch is entered only after winning the combat objective.
        c.A3 = c.GP + 0x32Cu;
    }

    public static bool IsQuest(CpuContext c, IMemory m) => m.ReadU8(c.GP + 0x31u) is 0 or 7;
    public static void BeginRoute() => InputManager.SignalScriptStage("quest_route");

    static void EnsureCatalog()
    {
        if (catalog != null) return;
        string? package = V82VehicleRegistry.LoadedPackageRoot;
        if (package == null) return;
        string path = Path.Combine(package, "quests.json");
        if (!File.Exists(path)) return;
        catalog = QuestCatalog.Read(File.ReadAllText(path));
        string root = Runtime.ResolveLoosePath() ?? Runtime.ExecutableDirectory;
        savePath = Environment.GetEnvironmentVariable("RECOMPONE_V82_QUEST_SAVE_PATH")
            ?? Path.Combine(root, "quest-progress.json");
        if (File.Exists(savePath))
        {
            var loaded = JsonSerializer.Deserialize<Dictionary<string, byte[]>>(File.ReadAllText(savePath))
                ?? throw new InvalidDataException("Invalid quest progress file");
            foreach (var (key, value) in loaded)
            {
                if (value.Length != ProgressSlots * RecordBytes) throw new InvalidDataException("Invalid quest progress record");
                saved[key] = value;
            }
        }
    }

    public static bool HasCampaign(int type)
    {
        EnsureCatalog();
        if (image != null && type >= 0 && type < BinaryPrimitives.ReadInt32LittleEndian(image.Data))
            return BinaryPrimitives.ReadUInt32LittleEndian(image.Data.AsSpan(16 + type * 4, 4)) != 0;
        if (type is >= 0 and < NativeCount)
        {
            string? root = Runtime.ResolveLoosePath();
            if (root == null) return true; // Retain the native disc-mode mask.
            nativeHeader ??= File.ReadAllBytes(Path.Combine(root, "QUEST.BIN"));
            return BinaryPrimitives.ReadUInt32LittleEndian(nativeHeader.AsSpan(16 + type * 4, 4)) != 0;
        }
        string? id = V82VehicleRegistry.StableIdForType(type);
        return id != null && catalog?.Campaigns.Any(c => c.Vehicle == id && c.Missions.Length > 0) == true;
    }

    public static bool Load(CpuContext c, IMemory m)
    {
        EnsureCatalog();
        if (catalog == null) return false;
        string? root = Runtime.ResolveLoosePath();
        if (root == null) return false;
        var state = c.Snapshot();
        try
        {
            byte[] native = File.ReadAllBytes(Path.Combine(root, "QUEST.BIN"));
            byte[] teams = new byte[NativeCount];
            for (uint t = 0; t < teams.Length; t++) teams[t] = m.ReadU8(0x8006B08Cu + t);
            image = NativeQuestImage.Build(native, teams, catalog, V82VehicleRegistry.TypeForStableId);
            c.A0 = checked((uint)image.Data.Length);
            V82Compat.PcMalloc(c, m);
            imageBase = c.V0;
            if (imageBase == 0) throw new OutOfMemoryException("Quest image allocation failed");
            for (uint i = 0; i < image.Data.Length; i++) m.WriteU8(imageBase + i, image.Data[i]);
            foreach (int offset in image.Relocations)
            {
                uint p = imageBase + checked((uint)offset);
                m.WriteU32(p, m.ReadU32(p) + imageBase);
            }
            m.WriteU32(c.GP + 0x1034u, imageBase);
            probeTriggered = false;
            if (progressBase == 0)
            {
                c.A0 = checked((uint)(TypeLimit * ProgressSlots * RecordBytes
                    + image.Data.Length - image.ResultTextOffset));
                V82Compat.PcMalloc(c, m);
                progressBase = c.V0;
                if (progressBase == 0) throw new OutOfMemoryException("Quest progress allocation failed");
                resultTextBase = progressBase + TypeLimit * ProgressSlots * RecordBytes;
                for (int i = image.ResultTextOffset; i < image.Data.Length; i++)
                    m.WriteU8(resultTextBase + (uint)(i - image.ResultTextOffset), image.Data[i]);
                for (uint i = 0; i < TypeLimit * ProgressSlots * RecordBytes; i++) m.WriteU8(progressBase + i, 0);
                foreach (var (type, campaign) in image.Campaigns)
                {
                    byte[] bytes = saved.GetValueOrDefault(campaign.Vehicle) ?? new byte[ProgressSlots * RecordBytes];
                    for (int slot = 0; slot < ProgressSlots; slot++)
                    {
                        if (bytes[slot * RecordBytes] == 0 && bytes[slot * RecordBytes + 1] == 0)
                            bytes[slot * RecordBytes] = 1;
                        uint address = ProgressAddress(type, slot, NativeProgress);
                        for (uint i = 0; i < RecordBytes; i++) m.WriteU8(address + i, bytes[slot * RecordBytes + i]);
                    }
                }
            }
            Console.WriteLine($"[V82Quests] loaded {image.Campaigns.Count} imported campaigns; native data preserved");
            return true;
        }
        finally { c.Restore(state); }
    }

    public static uint RouteAddress(int type, uint nativeAddress)
    {
        uint address = imageBase != 0 && image?.Routes.TryGetValue(type, out int offset) == true
            ? imageBase + checked((uint)offset) : nativeAddress;
        if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_QUEST") == "1")
            Console.WriteLine($"[QuestRoute] type={type} base={imageBase:X8} route={address:X8}");
        return address;
    }

    public static int CurrentType(IMemory m) => m.ReadU8(0x8006B8F4u);

    public static void BindPlayerActor(IMemory m, uint actor, uint vehicle)
    {
        if (vehicle == 0) return;
        int identity = (sbyte)m.ReadU8(actor);
        if (identity is not (-1 or -2)) return;
        // Original LOAD explicitly assigns the player sentinel after resolving
        // an arbitrary authored spawn node. Its negative node ID is not a pad ID.
        m.WriteU16(vehicle + 0xAu, unchecked((ushort)identity));
        for (uint offset = 0xF8; offset <= 0xFC; offset += 4)
        {
            uint child = m.ReadU32(vehicle + offset);
            if (child != 0) m.WriteU16(child + 0xAu, unchecked((ushort)identity));
        }
    }

    public static IEnumerable<int> MissionVehicleTypes(IMemory m)
    {
        if (m.ReadU8(0x8006A821u) is not (0 or 7)) yield break;
        EnsureCatalog();
        string? id = V82VehicleRegistry.StableIdForType(CurrentType(m));
        var campaign = catalog?.Campaigns.FirstOrDefault(q => q.Vehicle == id);
        int mission = m.ReadU8(0x8006B42Cu);
        if (campaign == null || mission >= campaign.Missions.Length) yield break;
        foreach (var actor in campaign.Missions[mission].Actors)
            if (actor.Sentinel == 0)
                yield return V82VehicleRegistry.TypeForStableId(actor.Vehicle);
    }

    public static bool PrepareEnding(CpuContext c, IMemory m)
    {
        if (image?.Campaigns.TryGetValue((int)c.A0, out var campaign) != true) return false;
        InputManager.SignalScriptStage("quest_ending");
        byte[] path = System.Text.Encoding.ASCII.GetBytes(campaign.EndingMovie.Replace('/', '\\'));
        if (path.Length is 0 or >= 128) throw new InvalidDataException("Invalid campaign ending path");
        // Keep the MIPS caller's four argument home slots below the string.
        // The movie player saves registers into those slots on entry.
        c.SP -= 144;
        for (uint i = 0; i < path.Length; i++) m.WriteU8(c.SP + 16 + i, path[i]);
        m.WriteU8(c.SP + 16 + (uint)path.Length, 0);
        c.A0 = c.SP + 16;
        c.A1 = 1;
        c.A2 = 0x08400000;
        return true;
    }
    public static uint SafeTeam(int type, uint nativeTeam) =>
        image?.Campaigns.ContainsKey(type) == true ? 0u : nativeTeam;

    public static void LoadingCard(CpuContext c, IMemory m)
    {
        int type = CurrentType(m), mission = m.ReadU8(c.GP + 0xC3Cu);
        if (IsQuest(c, m) && image?.Campaigns.TryGetValue(type, out var campaign) == true
            && mission < campaign.Missions.Length)
            c.A2 = checked((uint)campaign.Missions[mission].NativeLoadingCardVariant);
    }

    public static uint ProgressAddress(int type, int slot, uint nativeAddress) =>
        progressBase != 0 && image?.Campaigns.ContainsKey(type) == true && slot is >= 0 and < ProgressSlots
            ? progressBase + checked((uint)((type * ProgressSlots + slot) * RecordBytes)) : nativeAddress;

    public static uint ProgressBase(int type, uint nativeBase) =>
        progressBase != 0 && image?.Campaigns.ContainsKey(type) == true
            ? ProgressAddress(type, 0, nativeBase) - checked((uint)(type * RecordBytes)) : nativeBase;

    public static void SaveProgress(IMemory m)
    {
        if (progressBase == 0 || image == null || savePath == null) return;
        bool changed = false;
        foreach (var (type, campaign) in image.Campaigns)
        {
            var bytes = new byte[ProgressSlots * RecordBytes];
            for (int slot = 0; slot < ProgressSlots; slot++)
            {
                uint address = ProgressAddress(type, slot, NativeProgress);
                for (uint i = 0; i < RecordBytes; i++) bytes[slot * RecordBytes + i] = m.ReadU8(address + i);
            }
            if (!saved.TryGetValue(campaign.Vehicle, out byte[]? old) || !old.SequenceEqual(bytes))
            {
                saved[campaign.Vehicle] = bytes;
                changed = true;
            }
        }
        if (!changed) return;
        string temporary = savePath + ".tmp";
        File.WriteAllText(temporary, JsonSerializer.Serialize(saved, new JsonSerializerOptions { WriteIndented = true }));
        File.Move(temporary, savePath, true);
    }

    public static void BeforeFree(CpuContext c, IMemory m)
    {
        SaveProgress(m);
        // Native QUEST.BIN is discarded once actors/objectives are loaded,
        // before gameplay and earned-upgrade/result processing. Like native
        // fixed progress RAM, our single small progress allocation survives
        // mission teardown and is reused for the lifetime of this process.
        imageBase = 0;
    }
}
