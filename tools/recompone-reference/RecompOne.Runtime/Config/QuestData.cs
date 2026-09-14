using System.Buffers.Binary;
using System.Text;
using System.Text.Json;

namespace RecompOne.Runtime.Config;

// Authored campaign data is independent of native character slots and factions.
public sealed class QuestCatalog
{
    public int Version { get; set; }
    public QuestCampaign[] Campaigns { get; set; } = [];
    public static QuestCatalog Read(string json)
    {
        var result = JsonSerializer.Deserialize<QuestCatalog>(json,
            new JsonSerializerOptions { PropertyNameCaseInsensitive = true })
            ?? throw new InvalidDataException("Empty quest catalog");
        if (result.Version != 1) throw new InvalidDataException("Unsupported quest catalog");
        var ids = new HashSet<string>(StringComparer.Ordinal);
        foreach (var campaign in result.Campaigns)
        {
            if (string.IsNullOrWhiteSpace(campaign.Vehicle) || !ids.Add(campaign.Vehicle)
                || campaign.Missions.Length is < 1 or > 15)
                throw new InvalidDataException("Invalid campaign identity or mission count");
            foreach (var mission in campaign.Missions)
            {
                if (mission.Arena is < 0 or > 127 || mission.TargetFirst < 0
                    || mission.NativeLoadingCardVariant is < 0 or > 255
                    || mission.TargetLast < mission.TargetFirst || mission.TargetLast > short.MaxValue
                    || mission.Actors.Length is < 1 or > 7
                    || string.IsNullOrWhiteSpace(mission.Briefing)
                    || string.IsNullOrWhiteSpace(mission.Objectives))
                    throw new InvalidDataException("Invalid quest mission");
                foreach (var actor in mission.Actors)
                    if (actor.Sentinel is not (0 or 254 or 255)
                        || (actor.Sentinel == 0 && string.IsNullOrWhiteSpace(actor.Vehicle))
                        || actor.Flags is < 0 or > 255 || actor.Spawn is < short.MinValue or > short.MaxValue
                        || actor.Aux is < 0 or > ushort.MaxValue)
                        throw new InvalidDataException("Invalid quest actor");
            }
        }
        return result;
    }
}

public sealed class QuestCampaign
{
    public string Vehicle { get; set; } = "";
    public string EndingMovie { get; set; } = "";
    public QuestMission[] Missions { get; set; } = [];
}
public sealed class QuestMission
{
    public int Arena { get; set; }
    public int LoadingCardVariant { get; set; }
    // Layout selector in the installed V8:2 arena. The original selector above
    // is source provenance; its numbered cards are absent from reused arenas.
    public int NativeLoadingCardVariant { get; set; }
    public int TargetFirst { get; set; }
    public int TargetLast { get; set; }
    public bool Destroy { get; set; }
    public QuestActor[] Actors { get; set; } = [];
    public string Briefing { get; set; } = "";
    public string Objectives { get; set; } = "";
}
public sealed class QuestActor
{
    public string Vehicle { get; set; } = "";
    public int Sentinel { get; set; }
    public bool Deferred { get; set; }
    public int Flags { get; set; }
    public int Spawn { get; set; }
    public int Aux { get; set; }
}

public sealed record NativeQuestImage(byte[] Data, int[] Relocations,
    IReadOnlyDictionary<int, int> Routes, IReadOnlyDictionary<int, QuestCampaign> Campaigns,
    int ResultTextOffset)
{
    public static NativeQuestImage Build(byte[] native, byte[] nativeTeams,
        QuestCatalog catalog, Func<string, int> resolveType)
    {
        int U32(int at) => checked((int)BinaryPrimitives.ReadUInt32LittleEndian(native.AsSpan(at, 4)));
        int nativeCount = U32(0);
        if (nativeCount != nativeTeams.Length) throw new InvalidDataException("Quest team table mismatch");
        var campaigns = catalog.Campaigns.ToDictionary(q => resolveType(q.Vehicle));
        if (campaigns.Keys.Any(t => t < nativeCount || t >= 128))
            throw new InvalidDataException("Campaign must own a distinct valid vehicle type");
        int total = campaigns.Count == 0 ? nativeCount : Math.Max(nativeCount, campaigns.Keys.Max() + 1);
        int oldHeader = 16 + nativeCount * 4, header = 16 + total * 4, shift = header - oldHeader;
        using var stream = new MemoryStream();
        using var writer = new BinaryWriter(stream, Encoding.Latin1, true);
        writer.Write(new byte[header]);
        writer.Write(native.AsSpan(oldHeader));
        var relocations = new List<int>();
        var routes = new Dictionary<int, int>();
        void Put(int at, int value) { long end = stream.Position; stream.Position = at; writer.Write(value); stream.Position = end; }
        void Pointer(int at, int value) { Put(at, value); relocations.Add(at); }
        int Append(byte[] bytes) { while ((stream.Position & 3) != 0) writer.Write((byte)0); int at = checked((int)stream.Position); writer.Write(bytes); return at; }
        int Text(string text) => Append(Encoding.Latin1.GetBytes(text + "\0"));
        Put(0, total);
        for (int team = 0; team < 3; team++)
        {
            int route = U32(4 + team * 4), count = U32(route);
            Pointer(4 + team * 4, route + shift);
            for (int i = 0; i < count; i++)
                foreach (int field in new[] { 12, 16 })
                {
                    int at = route + 4 + i * 20 + field;
                    Pointer(at + shift, U32(at) + shift);
                }
        }
        for (int type = 0; type < nativeCount; type++)
        {
            int route = U32(4 + nativeTeams[type] * 4);
            routes[type] = route + shift;
            int records = U32(16 + type * 4);
            if (records == 0) continue;
            Pointer(16 + type * 4, records + shift);
            for (int i = 0; i < U32(route); i++)
                foreach (int field in new[] { 4, 8 })
                {
                    int at = records + i * 12 + field;
                    Pointer(at + shift, U32(at) + shift);
                }
        }
        foreach (var (type, campaign) in campaigns)
        {
            int count = campaign.Missions.Length;
            int route = Append(new byte[4 + count * 20]);
            int records = Append(new byte[count * 12]);
            routes[type] = route;
            Put(route, count);
            Pointer(16 + type * 4, records);
            for (int i = 0; i < count; i++)
            {
                var mission = campaign.Missions[i];
                int record = records + i * 12, routeRecord = route + 4 + i * 20;
                var actors = new byte[mission.Actors.Length * 6];
                for (int a = 0; a < mission.Actors.Length; a++)
                {
                    var actor = mission.Actors[a];
                    int actorType = actor.Sentinel != 0 ? actor.Sentinel : resolveType(actor.Vehicle);
                    if (actor.Sentinel == 0 && actorType is < 0 or >= 128)
                        throw new InvalidDataException("Quest actor type is outside native identity range");
                    actors[a * 6] = checked((byte)(actorType | (actor.Deferred ? 128 : 0)));
                    actors[a * 6 + 1] = checked((byte)actor.Flags);
                    BinaryPrimitives.WriteInt16LittleEndian(actors.AsSpan(a * 6 + 2), checked((short)actor.Spawn));
                    BinaryPrimitives.WriteUInt16LittleEndian(actors.AsSpan(a * 6 + 4), checked((ushort)actor.Aux));
                }
                Put(record, mission.Actors.Length);
                Pointer(record + 4, Append(actors));
                Pointer(record + 8, Text(mission.Briefing));
                Put(routeRecord, mission.Arena | (mission.Destroy ? 128 << 16 : 0));
                Put(routeRecord + 4, mission.TargetFirst << 16);
                Put(routeRecord + 8, mission.TargetLast);
                Pointer(routeRecord + 12, Append(new byte[2])); // No collection objective.
                Pointer(routeRecord + 16, Text(mission.Objectives));
            }
        }
        int resultText = Text("Objectives Met:\tA\u0007@\u0006%s\nB\u0007@\u0006%s\n\r\u0004À");
        return new(stream.ToArray(), relocations.ToArray(), routes, campaigns, resultText);
    }
}
