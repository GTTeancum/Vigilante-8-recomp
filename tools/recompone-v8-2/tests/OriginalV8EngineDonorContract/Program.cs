using System.Buffers.Binary;
using System.Globalization;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using RecompOne.Runtime.Cdrom;

int checks = 0;
void Check(bool condition, string name)
{
    if (!condition) throw new Exception(name);
    checks++;
}

void Reject<T>(Action action, string name) where T : Exception
{
    try { action(); }
    catch (T) { checks++; return; }
    throw new Exception("Expected rejection: " + name);
}

if (args.Length is 3 or 4 && args[0] == "--compare-original")
{
    // Read-only: no extracted data or encoded sidecar is written to disk.
    var pack = OriginalV8EngineDonor.ExtractFromCue(args[1]);
    var revisions = OriginalV8EngineDonor.SupportedRevisions;
    byte[] encoded = OriginalV8EngineSidecar.Encode(pack, revisions);
    var actual = OriginalV8EngineSidecar.Decode(encoded, revisions);
    using var expectedDoc = JsonDocument.Parse(File.ReadAllBytes(args[2]));
    var expected = expectedDoc.RootElement;
    Check(actual.Revision.Id == expected.GetProperty("revision").GetInt32(), "retail revision");
    Check(actual.ArmsLba == expected.GetProperty("arms_lba").GetInt32(), "ARMS.EXP extent LBA");
    Check(actual.Revision.ArmsLength == expected.GetProperty("arms_length").GetInt32(), "ARMS.EXP length");
    Check(actual.Revision.BootLength == expected.GetProperty("boot_length").GetInt32(), "retail boot length");
    MatchHash("boot_sha256", actual.Revision.BootSha256);
    MatchHash("arms_sha256", actual.Revision.ArmsSha256);
    MatchHash("entries_sha256", actual.Revision.EntriesSha256);
    var entries = expected.GetProperty("entries");
    Check(actual.Loops.Count == entries.GetArrayLength(), "donor loop count");
    for (int n = 0; n < actual.Loops.Count; n++)
    {
        var engine = actual.Loops[n];
        var record = entries[n];
        Check(engine.StableId == record.GetProperty("stable_id").GetString(), "retail stable ID order");
        Check(engine.DonorOffset == record.GetProperty("offset").GetInt32(), "ADPCM extent offset");
        Check(engine.Adpcm.Length == record.GetProperty("length").GetInt32(), "ADPCM length");
        Check(engine.Pitch == record.GetProperty("pitch").GetInt32(), "native pitch");
        Check(Hash(engine.Adpcm.Span).Equals(record.GetProperty("sha256").GetString(),
            StringComparison.OrdinalIgnoreCase), "exact ADPCM digest");
    }
    if (args.Length == 4)
    {
        byte[] a = File.ReadAllBytes(Path.Combine(args[3], "CUSTOM.EXP"));
        byte[] r = File.ReadAllBytes(Path.Combine(args[3], "VEHICLES.V8R"));
        Check(OriginalV8EngineRouting.IsCuratedPair(Hash(a), r), "exact curated shipped pair is allowed");
        a[^1] ^= 1;
        Check(!OriginalV8EngineRouting.IsCuratedPair(Hash(a), r), "modified archive blocked even with official registry");
        a[^1] ^= 1; r[^1] ^= 1;
        Check(!OriginalV8EngineRouting.IsCuratedPair(Hash(a), r), "modified registry blocked even with official archive");
    }
    Console.WriteLine($"[OriginalV8EngineDonorContract] PASS real/local checks={checks}, " +
        $"drivers={actual.Loops.Count}, ADPCM bytes={actual.Loops.Sum(x => x.Adpcm.Length)}, " +
        $"sidecar bytes={encoded.Length}, media files written=0");
    return;

    void MatchHash(string name, string value) => Check(value.Equals(expected.GetProperty(name).GetString(),
        StringComparison.OrdinalIgnoreCase), name);
}
if (args.Length != 0) throw new ArgumentException("Pass no arguments for synthetic checks.");

string temporary = Directory.CreateTempSubdirectory("v8-engine-donor-contract-").FullName;
try
{
    byte[] arms = FixtureArms(13, 1), boot = FixtureBoot(1);
    var sampleSet = OriginalV8EngineDonor.DecodeArms(arms);
    var first = SyntheticRevision(100, boot, arms, sampleSet);
    byte[] secondArms = FixtureArms(13, 2), secondBoot = FixtureBoot(2);
    var second = SyntheticRevision(101, secondBoot, secondArms, OriginalV8EngineDonor.DecodeArms(secondArms));
    OriginalV8EngineRevision[] supported = [first, second];
    string cue = FixtureDisc("original", arms, boot);
    string cue2 = FixtureDisc("alternate", secondArms, secondBoot);
    var pack = OriginalV8EngineDonor.ExtractFromCue(cue, supported);
    var pack2 = OriginalV8EngineDonor.ExtractFromCue(cue2, supported);
    Check(pack.Revision.Id == first.Id && pack2.Revision.Id == second.Id, "identify trusted boot plus bank");
    Check(sampleSet.Count == 13 && pack.Loops.Count == 13, "exactly 13 loops");
    Check(pack.ArmsLba == 24, "read exact HUD/ARMS.EXP extent");
    Check(pack.Loops.Select(x => x.StableId).Distinct().Count() == 13, "stable IDs distinct");
    Check(pack.Loops.Select(x => x.Pitch).SequenceEqual(Enumerable.Range(1024, 13).Select(x => (ushort)x)),
        "unaltered native pitches");
    for (int i = 0; i < pack.Loops.Count; i++)
    {
        string id = pack.Loops[i].StableId;
        Check(OriginalV8EngineRouting.MatchesLegacyEntry(i, id, 0, 0), "correct original stable ID and slot");
        Check(!OriginalV8EngineRouting.MatchesLegacyEntry(i, pack.Loops[(i + 1) % 13].StableId, 0, 0), "wrong stable ID blocked");
        Check(!OriginalV8EngineRouting.MatchesLegacyEntry(i, id, 1, 0), "non-ground transformation blocked");
        Check(!OriginalV8EngineRouting.MatchesLegacyEntry(i, id, 0, 2), "already-mapped donor blocked");
    }
    Check(!OriginalV8EngineRouting.MatchesLegacyEntry(-1, pack.Loops[0].StableId, 0, 0) &&
        !OriginalV8EngineRouting.MatchesLegacyEntry(13, pack.Loops[0].StableId, 0, 0), "native/foreign IDs blocked");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(cue),
        "unverified revision rejected by production defaults");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("sequel", arms, boot, bootStatement: "BOOT = cdrom:\\SLUS_008.68;1"), supported), "sequel BOOT");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("duplicate boot", arms, boot, bootStatement: "BOOT = cdrom:\\SLUS_005.10;1\nBOOT=cdrom:\\SLUS_005.10;1"), supported),
        "duplicate BOOT");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("wrong checksum same title", arms, secondBoot), [first]), "same-title wrong executable bytes");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("wrong bank for revision", secondArms, boot), supported), "unapproved ARMS bytes");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("basename trap", arms, boot, armName: "NOTARMS.EXP;1"), supported),
        "don't read root ARMS.EXP fallback");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("extent past raw file", arms, boot, armLba: 200), supported), "extent past Track 01");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("relocated known bytes", arms, boot, armLba: 25), supported), "known bytes from unverified donor extent");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("big endian mismatch", arms, boot, mismatchEndian: true), supported), "ISO mirror mismatch");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("wrong sector format", arms, boot, armForm2: true), supported), "not Form 1");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("mismatched subheaders", arms, boot, mismatchSubheader: true), supported),
        "Mode 2 subheader copies disagree");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(
        FixtureDisc("combined early audio", arms, boot, audioStart: 24), supported), "extent crosses CDDA boundary");
    Check(OriginalV8EngineDonor.ExtractFromCue(FixtureDisc("combined legal", arms, boot, 26), supported)
        .Loops.Count == 13, "combined image uses Track 01 only");
    string badCombined = FixtureDisc("combined disordered", arms, boot, 26);
    File.AppendAllText(badCombined, " TRACK 03 AUDIO\n  INDEX 01 00:00:25\n");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(badCombined, supported),
        "combined image may not hide an earlier audio boundary in a later record");
    Check(OriginalV8EngineDonor.ExtractFromCue(FixtureDisc("nonzero track index", arms, boot, dataStart: 5), supported)
        .Loops.Count == 13, "nonzero INDEX 01 translates relative LBAs");
    string traversal = Path.Combine(Path.GetDirectoryName(cue)!, "traversal.cue");
    File.WriteAllText(traversal, "FILE \"../alternate/fixture.bin\" BINARY\n TRACK 01 MODE2/2352\n  INDEX 01 00:00:00\n");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(traversal, supported),
        "escaped CUE filename");
    string shortCue = FixtureDisc("truncated raw track", arms, boot);
    using (var raw = File.OpenWrite(Path.Combine(Path.GetDirectoryName(shortCue)!, "fixture.bin")))
        raw.SetLength(raw.Length - 1);
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ExtractFromCue(shortCue, supported), "raw partial sector");

    Reject<InvalidDataException>(() => OriginalV8EngineDonor.DecodeArms(FixtureArms(12, 1)), "missing driver form");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.DecodeArms(FixtureArms(14, 1)), "extra driver form");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.DecodeArms(arms[..^1]), "truncated IFF pad/form");
    var badIff = (byte[])arms.Clone();
    badIff.AsSpan(4, 4).Fill(0xFF);
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.DecodeArms(badIff), "unbounded chunk length");
    byte[] badLoop = sampleSet[0].Adpcm.ToArray();
    badLoop[1] = 2; // Strip the only loop start.
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ValidateLoop(badLoop, 1024), "missing loop point");
    badLoop[1] = 6; badLoop[^15] = 2;
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ValidateLoop(badLoop, 1024), "missing loop end");
    badLoop[^15] = 3; badLoop[0] = 0x5D;
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ValidateLoop(badLoop, 1024), "invalid predictor/shift");
    Reject<InvalidDataException>(() => OriginalV8EngineDonor.ValidateLoop(sampleSet[0].Adpcm.Span, 0), "invalid pitch");

    byte[] expected = OriginalV8EngineSidecar.Encode(pack, supported);
    byte[] expected2 = OriginalV8EngineSidecar.Encode(pack2, supported);
    Check(expected.SequenceEqual(OriginalV8EngineSidecar.Encode(
        OriginalV8EngineSidecar.Decode(expected, supported), supported)), "deterministic round trip");
    var truncated = expected[..^1];
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.Decode(truncated, supported), "truncated sidecar");
    var forged = (byte[])expected.Clone();
    forged[84 + 12 + 2] ^= 7; // Valid new ADPCM nibble, not a malformed header.
    SHA256.HashData(forged.AsSpan(0, forged.Length - 32), forged.AsSpan(forged.Length - 32));
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.Decode(forged, supported),
        "forged payload rejected even with recomputed footer");
    forged = (byte[])expected.Clone();
    forged[6] = 0xEE;
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.Decode(forged, supported), "unknown cache revision");
    forged = (byte[])expected.Clone();
    forged[17] ^= 0x60;
    SHA256.HashData(forged.AsSpan(0, forged.Length - 32), forged.AsSpan(forged.Length - 32));
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.Decode(forged, supported), "false source metadata");
    forged = (byte[])expected.Clone();
    forged[12] += 1;
    SHA256.HashData(forged.AsSpan(0, forged.Length - 32), forged.AsSpan(forged.Length - 32));
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.Decode(forged, supported), "forged donor LBA");

    var destination = Directory.CreateDirectory(Path.Combine(temporary, "selected cache")).FullName;
    var sidecar = Path.Combine(destination, OriginalV8EngineSidecar.FileName);
    Check(!File.Exists(sidecar), "extraction alone creates no cache");
    Check(OriginalV8EngineSidecar.WriteAtomic(destination, pack, supported) == OriginalV8EngineCacheResult.Created,
        "explicit first save");
    Check(File.ReadAllBytes(sidecar).AsSpan().SequenceEqual(expected), "first committed bytes are complete");
    Check(OriginalV8EngineSidecar.WriteAtomic(destination, pack, supported) == OriginalV8EngineCacheResult.Unchanged,
        "identical valid cache is a no-op");
    Reject<IOException>(() => OriginalV8EngineSidecar.WriteAtomic(destination, pack2, supported),
        "replacement is explicit");
    using var canceled = new CancellationTokenSource();
    canceled.Cancel();
    Reject<OperationCanceledException>(() => OriginalV8EngineSidecar.WriteAtomic(destination, pack2, supported,
        replaceExisting: true, cancellation: canceled.Token), "canceled replacement");
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.WriteAtomic(destination, pack2 with
        { Loops = pack2.Loops.Take(12).ToArray() }, supported, replaceExisting: true), "invalid replacement");
    Check(File.ReadAllBytes(sidecar).AsSpan().SequenceEqual(expected), "failed replacements leave old cache untouched");
    Check(OriginalV8EngineSidecar.WriteAtomic(destination, pack2, supported, replaceExisting: true) ==
        OriginalV8EngineCacheResult.Replaced, "explicit valid replacement");
    Check(File.ReadAllBytes(sidecar).AsSpan().SequenceEqual(expected2), "all replacement bytes are present");
    byte[] unrelated = Encoding.UTF8.GetBytes("A file this code does not own.");
    File.WriteAllBytes(sidecar, unrelated);
    Reject<InvalidDataException>(() => OriginalV8EngineSidecar.WriteAtomic(destination, pack, supported,
        replaceExisting: true), "unknown destination never clobbered");
    Check(File.ReadAllBytes(sidecar).AsSpan().SequenceEqual(unrelated), "unowned file unchanged");
    Check(Directory.GetFileSystemEntries(destination).Length == 1, "temporary files do not leak");
    if (OperatingSystem.IsLinux())
    {
        string linkedDirectory = Directory.CreateDirectory(Path.Combine(temporary, "linked cache")).FullName;
        File.CreateSymbolicLink(Path.Combine(linkedDirectory, OriginalV8EngineSidecar.FileName), sidecar);
        Reject<IOException>(() => OriginalV8EngineSidecar.WriteAtomic(linkedDirectory, pack, supported,
            replaceExisting: true), "cache link cannot be replaced");
        Reject<IOException>(() => OriginalV8EngineSidecar.Read(linkedDirectory), "cache link cannot be read");
        Check(File.ReadAllBytes(sidecar).AsSpan().SequenceEqual(unrelated), "linked destination unaffected");
    }

    var notices = new List<string>();
    string setupRoot = Path.Combine(temporary, "game_data");
    string installCache = OriginalV8EngineInstallation.StagedDirectory(setupRoot);
    Check(OriginalV8EngineInstallation.TryRead(setupRoot, supported, notices.Add) == null &&
        !Directory.Exists(installCache) && notices.Count == 0, "without original input reads nothing and creates nothing");
    Check(!OriginalV8EngineInstallation.TryImport(cue, setupRoot, default, notices.Add) &&
        !Directory.Exists(installCache) && notices.Count == 1, "unverified original source skips cache with one diagnostic");
    notices.Clear();
    string invalidText = Path.Combine(temporary, "invalid-text.cue");
    File.WriteAllBytes(invalidText, [0x46, 0xC0, 0xFF, 0x0A]);
    Check(!OriginalV8EngineInstallation.TryImport(invalidText, setupRoot, supported, default, notices.Add) &&
        !Directory.Exists(installCache) && notices.Count == 1, "invalid UTF-8 does not fail optional setup");
    notices.Clear();
    Reject<OperationCanceledException>(() => OriginalV8EngineInstallation.TryImport(cue, setupRoot, supported,
        canceled.Token, notices.Add), "canceled setup import does not commit");
    Check(!Directory.Exists(installCache) && notices.Count == 0, "cancel before cache creation");
    Check(OriginalV8EngineInstallation.TryImport(cue, setupRoot, supported, default, notices.Add) &&
        !Directory.Exists(setupRoot) && notices.Count == 1, "original first: complete cache without partial sequel data");
    string cacheFile = Path.Combine(installCache, OriginalV8EngineSidecar.FileName);
    Directory.CreateDirectory(setupRoot);
    string sentinel = Path.Combine(setupRoot, "unchanged.txt");
    File.WriteAllBytes(sentinel, unrelated);
    Check(OriginalV8EngineInstallation.TryRead(setupRoot, supported, notices.Add)?.Revision.Id == first.Id,
        "sequel subsequently sees same cache");
    byte[] stagedBytes = File.ReadAllBytes(cacheFile);
    OriginalV8EngineInstallation.AdoptStaged(setupRoot, supported, notices.Add);
    cacheFile = Path.Combine(setupRoot, OriginalV8EngineSidecar.FileName);
    Check(File.ReadAllBytes(cacheFile).SequenceEqual(stagedBytes) && !Directory.Exists(installCache),
        "original first: verified cache adopted byte-for-byte and empty staging directory removed");
    Check(OriginalV8EngineInstallation.TryImport(cue, setupRoot, supported, default, notices.Add, sequelInstalled: true) &&
        notices.Last().Contains("unchanged") && File.ReadAllBytes(sentinel).SequenceEqual(unrelated),
        "installed sequel: identical reimport no-op; installed data untouched");
    string direct = Directory.CreateDirectory(Path.Combine(temporary, "direct")).FullName;
    Check(OriginalV8EngineInstallation.TryImport(cue, direct, supported, default, notices.Add, sequelInstalled: true) &&
        File.ReadAllBytes(Path.Combine(direct, OriginalV8EngineSidecar.FileName)).SequenceEqual(stagedBytes) &&
        !Directory.Exists(OriginalV8EngineInstallation.StagedDirectory(direct)), "sequel first: cache written directly to game_data");
    if (OperatingSystem.IsLinux())
        Check((File.GetUnixFileMode(cacheFile) & (UnixFileMode.GroupRead | UnixFileMode.OtherRead)) == 0,
            "local cache isn't readable by other Unix accounts");
    File.WriteAllBytes(cacheFile, unrelated);
    notices.Clear();
    Check(OriginalV8EngineInstallation.TryRead(setupRoot, supported, notices.Add) == null && notices.Count == 1,
        "malformed cache is ignored once, not fatal");
    Check(!OriginalV8EngineInstallation.TryImport(cue, setupRoot, supported, default, notices.Add, sequelInstalled: true) &&
        File.ReadAllBytes(cacheFile).SequenceEqual(unrelated), "setup never clobbers unowned cache data");
    Directory.CreateDirectory(installCache);
    string oldCache = Path.Combine(installCache, OriginalV8EngineSidecar.FileName);
    File.WriteAllBytes(oldCache, stagedBytes);
    string kept = Path.Combine(installCache, "keep.txt");
    File.WriteAllBytes(kept, unrelated);
    OriginalV8EngineInstallation.AdoptStaged(setupRoot, supported, notices.Add);
    Check(File.ReadAllBytes(cacheFile).SequenceEqual(unrelated) &&
        File.ReadAllBytes(oldCache).SequenceEqual(stagedBytes) &&
        OriginalV8EngineInstallation.TryRead(setupRoot, supported, notices.Add)?.Revision.Id == first.Id,
        "existing unrelated target survives and legacy data remains readable");
    File.Delete(cacheFile);
    OriginalV8EngineInstallation.AdoptStaged(setupRoot, supported, notices.Add);
    Check(!File.Exists(oldCache) && File.ReadAllBytes(cacheFile).SequenceEqual(stagedBytes) &&
        File.ReadAllBytes(kept).SequenceEqual(unrelated), "migration leaves unrelated staging files intact");
    if (OperatingSystem.IsLinux())
    {
        string anotherRoot = Path.Combine(temporary, "redirected_game");
        Directory.CreateSymbolicLink(OriginalV8EngineInstallation.StagedDirectory(anotherRoot), installCache);
        Check(!OriginalV8EngineInstallation.TryImport(cue, anotherRoot, supported, default, notices.Add) &&
            !File.Exists(oldCache) && File.ReadAllBytes(kept).SequenceEqual(unrelated), "setup refuses a linked cache directory");
    }
    Console.WriteLine($"[OriginalV8EngineDonorContract] PASS synthetic checks={checks}");
}
finally { Directory.Delete(temporary, recursive: true); }

string FixtureDisc(string name, byte[] armBytes, byte[] executable, int? audioStart = null, int dataStart = 0,
    int armLba = 24, string armName = "ARMS.EXP;1", bool mismatchEndian = false, bool armForm2 = false,
    bool mismatchSubheader = false, string bootStatement = "BOOT = cdrom:\\SLUS_005.10;1")
{
    string root = Directory.CreateDirectory(Path.Combine(temporary, name)).FullName;
    const int sector = 2352, logical = 2048;
    byte[] dat = new byte[(dataStart + 29) * sector];
    for (int n = 0; n < dat.Length; n += sector)
    {
        dat.AsSpan(n + 1, 10).Fill(255);
        dat[n + 15] = 2;
    }
    Span<byte> Payload(int lba) => dat.AsSpan((dataStart + lba) * sector + 24, logical);
    Span<byte> pvd = Payload(16);
    pvd[0] = 1; "CD001"u8.CopyTo(pvd[1..]); pvd[6] = 1;
    Both32(pvd[80..], 149); // Valid volume can include later CDDA records.
    Both16(pvd[128..], logical);
    PutRecord(pvd[156..], [0], 20, logical, true);
    byte[] cnf = Encoding.ASCII.GetBytes(bootStatement);
    Span<byte> records = Payload(20);
    int pos = PutRecord(records, "SYSTEM.CNF;1"u8, 21, cnf.Length);
    pos += PutRecord(records[pos..], "SLUS_005.10;1"u8, 22, executable.Length);
    pos += PutRecord(records[pos..], "HUD"u8, 23, logical, true);
    _ = PutRecord(records[pos..], "ARMS.EXP;1"u8, 24, armBytes.Length);
    cnf.CopyTo(Payload(21)); executable.CopyTo(Payload(22)); armBytes.CopyTo(Payload(24));
    if (armLba is > 24 and < 29) armBytes.CopyTo(Payload(armLba));
    _ = PutRecord(Payload(23), Encoding.ASCII.GetBytes(armName), armLba, armBytes.Length);
    if (mismatchEndian) Payload(23)[6] ^= 1;
    if (armForm2) dat[(dataStart + 24) * sector + 18] = dat[(dataStart + 24) * sector + 22] = 0x20;
    if (mismatchSubheader) dat[(dataStart + 24) * sector + 23] = 1;
    File.WriteAllBytes(Path.Combine(root, "fixture.bin"), dat);
    string dataIndex = dataStart.ToString("00", CultureInfo.InvariantCulture);
    string sheet = $"FILE \"fixture.bin\" BINARY\n TRACK 01 MODE2/2352\n  INDEX 01 00:00:{dataIndex}\n";
    sheet += audioStart.HasValue
        ? $" TRACK 02 AUDIO\n  INDEX 01 00:00:{audioStart.Value:00}\n"
        : "FILE \"intentionally unavailable audio.bin\" BINARY\n TRACK 02 AUDIO\n  INDEX 01 00:02:00\n";
    string cueFile = Path.Combine(root, "fixture.cue");
    File.WriteAllText(cueFile, sheet);
    return cueFile;
}

static byte[] FixtureArms(int drivers, int seed)
{
    var children = new List<byte[]> { Chunk("HEAD"u8, [0x01]) };
    for (int i = 0; i < drivers; i++)
    {
        var snd = new byte[8 + 32];
        BinaryPrimitives.WriteUInt16LittleEndian(snd, 1);
        BinaryPrimitives.WriteUInt16LittleEndian(snd.AsSpan(2), 4);
        BinaryPrimitives.WriteUInt16LittleEndian(snd.AsSpan(6), (ushort)(1024 + i));
        snd[9] = 6; snd[10] = (byte)(seed + i); snd[25] = 3;
        children.Add(Form("XOBF"u8, [Chunk("BIN "u8, [1, 2, 3]), Chunk("SND "u8, snd)]));
    }
    children.Add(Chunk("PLTX"u8, []));
    return Form("TERR"u8, children);
}

static byte[] FixtureBoot(int seed)
{
    byte[] bytes = new byte[64];
    "PS-X EXE"u8.CopyTo(bytes);
    bytes[55] = (byte)seed;
    return bytes;
}

static OriginalV8EngineRevision SyntheticRevision(ushort id, byte[] boot, byte[] arms,
    IReadOnlyList<OriginalV8EngineLoop> loops)
{
    using var memory = new MemoryStream();
    using var records = new BinaryWriter(memory);
    foreach (var loop in loops)
    {
        records.Write(loop.DonorOffset); records.Write(loop.Adpcm.Length);
        records.Write(loop.Pitch); records.Write((ushort)0); records.Write(loop.Adpcm.Span);
    }
    return new(id, boot.Length, Hash(boot), 24, arms.Length, Hash(arms), Hash(memory.ToArray()));
}

static byte[] Form(ReadOnlySpan<byte> type, IEnumerable<byte[]> children)
    => Chunk("FORM"u8, type.ToArray().Concat(children.SelectMany(x => x)).ToArray());

static byte[] Chunk(ReadOnlySpan<byte> type, ReadOnlySpan<byte> payload)
{
    byte[] bytes = new byte[8 + payload.Length + (payload.Length & 1)];
    type.CopyTo(bytes);
    BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(4), payload.Length);
    payload.CopyTo(bytes.AsSpan(8));
    return bytes;
}

static int PutRecord(Span<byte> record, ReadOnlySpan<byte> name, int lba, int bytes, bool directory = false)
{
    int length = 33 + name.Length + (name.Length % 2 == 0 ? 1 : 0);
    record[0] = (byte)length;
    Both32(record[2..], lba); Both32(record[10..], bytes);
    record[25] = (byte)(directory ? 2 : 0);
    Both16(record[28..], 1);
    record[32] = (byte)name.Length;
    name.CopyTo(record[33..]);
    return length;
}

static void Both32(Span<byte> data, int value)
{
    BinaryPrimitives.WriteInt32LittleEndian(data, value);
    BinaryPrimitives.WriteInt32BigEndian(data[4..], value);
}
static void Both16(Span<byte> data, ushort value)
{
    BinaryPrimitives.WriteUInt16LittleEndian(data, value);
    BinaryPrimitives.WriteUInt16BigEndian(data[2..], value);
}
static string Hash(ReadOnlySpan<byte> bytes) => Convert.ToHexString(SHA256.HashData(bytes));
