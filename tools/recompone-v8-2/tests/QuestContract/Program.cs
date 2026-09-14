using System.Buffers.Binary;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Context;
using System.Reflection;
using System.Text.Json;

void Check(bool condition, string description)
{
    if (!condition) throw new Exception(description);
    Console.WriteLine("PASS " + description);
}
string root = args[0];
byte[] native = File.ReadAllBytes(Path.Combine(root, "V8_2_LOOSE/QUEST.BIN"));
byte[] exe = File.ReadAllBytes(Path.Combine(root, "V8_2_LOOSE/SLUS_008.68"));
byte[] teams = exe.AsSpan(0x5B88C, 18).ToArray();
var catalog = QuestCatalog.Read(File.ReadAllText(Path.Combine(root, "artifacts/original-quests-20260910/quests.json")));
var names = new[] { "chassey_blue", "slick_clyde", "sheila", "john_torque", "dave", "convoy",
    "loki", "houston_3", "boogie", "beezwax", "molo", "sid_burn", "y_the_alien" };
int Resolve(string name) { int n = Array.IndexOf(names, name.Replace("guest.v8.", "")); return n < 0 ? -1 : n + 64; }
var image = NativeQuestImage.Build(native, teams, catalog, Resolve);
int Read(byte[] b, int offset) => BinaryPrimitives.ReadInt32LittleEndian(b.AsSpan(offset));
Check(image.Campaigns.Count == 12 && image.Campaigns.Values.Sum(c => c.Missions.Length) == 48, "all twelve original campaigns and 48 missions");
Check(!image.Campaigns.ContainsKey(76), "Y has no campaign; original Boogie remains eligible");
Check(image.Campaigns.ContainsKey(72), "original Boogie has a campaign");
int shift = (Read(image.Data, 0) - Read(native, 0)) * 4;
var relocated = image.Relocations.ToHashSet();
for (int i = 88; i < native.Length; i++)
{
    int at = i + shift;
    bool pointer = Enumerable.Range(0, 4).Any(n => relocated.Contains(at - n));
    if (!pointer && image.Data[at] != native[i]) throw new Exception($"Native payload changed at {i:X}");
}
Check(true, "native payload preserved byte for byte except relocated pointers");
foreach (int pointer in image.Relocations)
    Check(Read(image.Data, pointer) >= 0 && Read(image.Data, pointer) < image.Data.Length, $"pointer {pointer:X} stays inside image");
Check(new[] { 5, 11, 17 }.All(t => Read(image.Data, 16 + t * 4) == 0), "native no-quest entries remain null");
foreach (var (type, campaign) in image.Campaigns)
{
    int route = image.Routes[type];
    int records = Read(image.Data, 16 + type * 4);
    Check(Read(image.Data, route) == 4, $"type {type} owns its four-stage route");
    for (int i = 0; i < 4; i++)
    {
        var mission = campaign.Missions[i];
        int r = route + 4 + i * 20;
        Check(image.Data[r] == mission.Arena && image.Data[r + 2] == (mission.Destroy ? 128 : 0), $"type {type} mission {i} arena and protect/destroy polarity");
        Check(BinaryPrimitives.ReadUInt16LittleEndian(image.Data.AsSpan(r + 6)) == mission.TargetFirst &&
              BinaryPrimitives.ReadUInt16LittleEndian(image.Data.AsSpan(r + 8)) == mission.TargetLast, "source target-ID range preserved");
        Check(Read(image.Data, records + i * 12) == mission.Actors.Length, "actor count preserved");
    }
}
Console.WriteLine("Quest image contract complete.");

// Supply the image and arena-independent RAM for the actual registry accessors.
void Field(string name, object value) => typeof(V82QuestRegistry).GetField(name,
    BindingFlags.Static | BindingFlags.NonPublic)!.SetValue(null, value);
Field("catalog", catalog); Field("image", image); Field("imageBase", 0x801A0000u);
Field("progressBase", 0x801D0000u);
Field("resultTextBase", 0x801D0F00u);
string save = Path.Combine(root, "artifacts/original-quests-20260910/progress-contract.json");
Field("savePath", save);
var memory = new PSMemory();
var uiCpu = new CpuContext { GP = 0x8006A7F0, A1 = 123, A2 = 99, A3 = 456 };
memory.WriteU8(0x8006B8F4, 64);
V82QuestRegistry.LoadingCard(uiCpu, memory);
Check(uiCpu.A2 == 0, "Ski Resort uses its available card instead of missing original variant 1");
V82QuestRegistry.BeforeFree(uiCpu, memory);
Check(V82QuestRegistry.ProgressAddress(64, 0, 123) != 123, "custom upgrade RAM survives native quest-file teardown before gameplay");
V82QuestRegistry.ResultText(uiCpu, memory);
Check(uiCpu.A1 == 0x801D0F00u, "result format survives native quest-file teardown before gameplay");
Check(uiCpu.A2 == 456, "original first objective uses native scenery result, not the empty collection objective");
string resultFormat = System.Text.Encoding.Latin1.GetString(image.Data.AsSpan(image.ResultTextOffset)).TrimEnd('\0');
Check(resultFormat.Count(ch => ch == '%') == 2 && !resultFormat.Contains("C\u0007"), "original result has two objective rows");
Check(uiCpu.A3 == uiCpu.GP + 0x32C, "combat victory populates the second original objective");
memory.WriteU8(0x8006B8F4, 0);
uiCpu.A1 = 123; uiCpu.A2 = 99; uiCpu.A3 = 456;
V82QuestRegistry.LoadingCard(uiCpu, memory);
V82QuestRegistry.ResultText(uiCpu, memory);
Check(uiCpu.A1 == 123 && uiCpu.A2 == 99 && uiCpu.A3 == 456, "native briefing and result arguments remain unchanged");
memory.WriteU32(0x801801F8, 0x80180300);
memory.WriteU32(0x801801FC, 0x80180400);
foreach (byte sentinel in new byte[] { 255, 254 })
{
    memory.WriteU8(0x80180000, sentinel);
    memory.WriteU16(0x80180002, unchecked((ushort)-11));
    memory.WriteU16(0x8018010A, unchecked((ushort)-11));
    V82QuestRegistry.BindPlayerActor(memory, 0x80180000, 0x80180100);
    Check((short)memory.ReadU16(0x8018010A) == (sbyte)sentinel &&
          (short)memory.ReadU16(0x8018030A) == (sbyte)sentinel &&
          (short)memory.ReadU16(0x8018040A) == (sbyte)sentinel,
          "authored spawn -11 binds player and children to the correct player slot");
}
for (uint i = 0; i < 540; i++) memory.WriteU8(0x8006B9E8u + i, (byte)(i * 17));
var addresses = new HashSet<uint>();
foreach (var (type, campaign) in image.Campaigns)
{
    Check(V82QuestRegistry.HasCampaign(type), $"campaign {type} is selectable");
    for (int slot = 0; slot < 3; slot++)
    {
        uint address = V82QuestRegistry.ProgressAddress(type, slot, 0x8006B9E8);
        Check(addresses.Add(address), $"type {type} slot {slot} has independent progress");
        for (uint i = 0; i < 10; i++) memory.WriteU8(address + i, (byte)(type + slot + i));
    }
    var cpu = new CpuContext { SP = 0x801FF000, A0 = (uint)type };
    Check(V82QuestRegistry.PrepareEnding(cpu, memory), "original ending dispatches without native type indexing");
    string actual = "";
    for (uint p = cpu.A0; memory.ReadU8(p) != 0; p++) actual += (char)memory.ReadU8(p);
    Check(actual == campaign.EndingMovie.Replace('/', '\\'), "ending path matches original source table");
}
Check(Enumerable.Range(0, 18).Count(V82QuestRegistry.HasCampaign) == 15, "fifteen native campaign entries remain selectable");
Check(!V82QuestRegistry.HasCampaign(76), "Y is excluded from Quest");
Check(V82QuestRegistry.ProgressAddress(0, 0, 123) == 123, "native progress address unchanged");
V82QuestRegistry.SaveProgress(memory);
var progress = JsonSerializer.Deserialize<Dictionary<string, byte[]>>(File.ReadAllText(save))!;
foreach (var (type, campaign) in image.Campaigns)
    for (int slot = 0; slot < 3; slot++)
        for (int i = 0; i < 10; i++)
            if (progress[campaign.Vehicle][slot * 10 + i] != type + slot + i) throw new Exception("Progress save lost slot data");
V82QuestRegistry.ProbeNativeProgress(memory);
for (uint i = 0; i < 540; i++)
    if (memory.ReadU8(0x8006B9E8u + i) != (byte)(i * 17)) throw new Exception("Native save record overwritten");
Check(true, "all imported slots serialize under stable identities; native save canary remains intact");

// Quest's zero selector argument disables enemy setup; it must not disable
// player portrait replacement or impersonate a return from the enemy editor.
var selectorCpu = new CpuContext { GP = 0x8006A7F0, A1 = 0 };
memory.WriteU8(selectorCpu.GP + 0x31u, 0);
V82VehicleRegistry.BeginNativeSelector(selectorCpu, memory);
object SelectorField(string name) => typeof(V82VehicleRegistry).GetField(name,
    BindingFlags.Static | BindingFlags.NonPublic)!.GetValue(null)!;
Check((uint)SelectorField("_selectorContext") == 1u &&
    !(bool)SelectorField("_selectorEnemyPhase") && selectorCpu.A1 == 0,
    "Quest selects player one while preserving native skip-enemy argument");
memory.WriteU8(selectorCpu.GP + 0x31u, 1);
V82VehicleRegistry.BeginNativeSelector(selectorCpu, memory);
Check((uint)SelectorField("_selectorContext") == 0u &&
    (bool)SelectorField("_selectorEnemyPhase"), "non-Quest enemy context remains unchanged");
