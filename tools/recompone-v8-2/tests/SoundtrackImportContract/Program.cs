using NVorbis;
using RecompOne.Runtime.Cdrom;

// Usage: SoundtrackImportContract <Vigilante 8 (USA).cue> <Second Offense (USA).cue>
// Needs the user's own discs, so it is run by hand rather than in CI.
int checks = 0;
void Check(bool condition, string name)
{
    if (!condition) throw new Exception(name);
    checks++;
}
string v8 = Path.GetFullPath(args[0]), v82 = Path.GetFullPath(args[1]);
string scratch = Path.Combine(Path.GetTempPath(), $"soundtrack-contract-{Environment.ProcessId}");
Directory.CreateDirectory(scratch);
try
{
    // Disc identity: only the first game's disc takes the soundtrack path,
    // whichever way its title ID is written.
    Check(V8SoundtrackImporter.BootExecutable(v8) == "SLUS_005.10", "first game boots SLUS_005.10");
    Check(V8SoundtrackImporter.IsFirstGameDisc(v8), "first game recognised");
    Check(!V8SoundtrackImporter.IsFirstGameDisc(v82), "Second Offense is not the first game");
    Check(!V8SoundtrackImporter.IsFirstGameDisc(Path.Combine(scratch, "missing.cue")), "unreadable disc is not the first game");
    foreach (string id in new[] { "SLUS_005.10", "SLUS_00510", "SLUS-00510", "slus00510" })
        Check(V8SoundtrackImporter.IsFirstGameTitle(id), $"title spelling accepted: {id}");
    Check(!V8SoundtrackImporter.IsFirstGameTitle("SLUS_008.68"), "Second Offense title rejected");

    // Not installed yet: the album is staged, game_data is not created (the
    // Second Offense import refuses an existing half-filled one).
    string fresh = Path.Combine(scratch, "fresh", "game_data");
    var phases = new List<string>();
    Check(!V8SoundtrackImporter.Import(v8, fresh, p => phases.Add(p.Phase)), "fresh install stages the album");
    Check(!Directory.Exists(fresh), "game_data not created before Second Offense");
    string staged = Path.Combine(V8SoundtrackImporter.StagingRoot(fresh), "music");
    string[] expected = ["BEEZEWAX", "BOOGIE", "CHASSEY", "CONVOY", "DAVE", "HOUSTON3", "JOHNT", "LOKI", "MOLO", "SHELIA", "SID", "SLICK"];
    Check(Directory.GetFiles(staged, "*.ogg").Select(Path.GetFileNameWithoutExtension).Order()
        .SequenceEqual(expected), "all 12 album tracks staged under the manifest names");
    Check(phases.Count(p => p == "soundtrack") == 12, "progress reported per track");
    Check(!Directory.Exists(fresh + ".v8-soundtrack.partial"), "scratch folder removed");

    // Each OGG plays at the CD's rate and is exactly as long as its CD track.
    using (var disc = CueBin.Open(v8))
    {
        for (int track = 2; track <= 13; track++)
        {
            disc.TryGetTrackStartLba(track, out int start);
            disc.TryReadAudioSector(start, out _, out _, out int end);
            var file = Directory.GetFiles(staged, "*.ogg").Single(f =>
            {
                using var r = new VorbisReader(f);
                return r.Tags.Title == $"Vigilante 8 Track {track:00}";
            });
            using var reader = new VorbisReader(file);
            Check(reader.SampleRate == 44100 && reader.Channels == 2, $"track {track:00} is 44.1 kHz stereo");
            long cdFrames = (long)(end - start) * 588;
            Check(Math.Abs(reader.TotalSamples - cdFrames) <= 1024, $"track {track:00} length {reader.TotalSamples} vs CD {cdFrames}");
        }
    }

    // The Second Offense import adopts the staged album into the new install.
    string partial = Path.Combine(scratch, "fresh", "game_data.partial");
    Directory.CreateDirectory(Path.Combine(partial, "music"));
    File.WriteAllBytes(Path.Combine(partial, "music", "track02.ogg"), [1]);
    V8SoundtrackImporter.AdoptStaged(fresh, partial);
    Check(Directory.GetFiles(Path.Combine(partial, "music"), "*.ogg").Length == 13, "album adopted beside Second Offense music");
    Check(File.ReadAllBytes(Path.Combine(partial, "music", "track02.ogg")).Length == 1, "Second Offense music untouched");
    Check(!Directory.Exists(V8SoundtrackImporter.StagingRoot(fresh)), "staging removed after adoption");

    // Already installed: the album goes straight into game_data\music.
    string installed = Path.Combine(scratch, "installed", "game_data");
    Directory.CreateDirectory(Path.Combine(installed, "music"));
    foreach (string marker in new[] { ".recompone-import-complete", "SYSTEM.CNF", "SLUS_008.68" })
        File.WriteAllText(Path.Combine(installed, marker), "");
    File.WriteAllBytes(Path.Combine(installed, "music", "track02.ogg"), [2]);
    Check(V8SoundtrackImporter.Import(v8, installed), "installed game receives the album directly");
    Check(Directory.GetFiles(Path.Combine(installed, "music"), "*.ogg").Length == 13 &&
        File.ReadAllBytes(Path.Combine(installed, "music", "track02.ogg")).Length == 1,
        "album added beside the existing music");
    Check(!Directory.Exists(V8SoundtrackImporter.StagingRoot(installed)), "nothing staged for an installed game");

    // Selecting the Second Offense disc never takes the soundtrack path.
    bool rejected = false;
    try { V8SoundtrackImporter.Import(v82, Path.Combine(scratch, "wrong", "game_data")); }
    catch (InvalidDataException ex) { rejected = ex.Message.Contains("SLUS-00510"); }
    Check(rejected, "soundtrack import rejects other discs");

    Console.WriteLine($"[SoundtrackImportContract] PASS checks={checks}");
    return 0;
}
finally
{
    Directory.Delete(scratch, true);
}
