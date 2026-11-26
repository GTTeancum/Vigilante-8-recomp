namespace RecompOne.Runtime.Cdrom;

internal static class OriginalV8EngineRouting
{
    // The shipped, curated legacy pair, not a folder name or an arbitrary
    // third-party package using the same stable IDs. Already-mapped packages
    // retain the existing authored engine selection without this overlay.
    internal const string ArchiveSha256 = "8CFB3224A9DF8BF47DE319776086A477A752EF67AA592E79360CFDAD7F27B766";
    internal const string RegistrySha256 = "B64295BD322503FDE5FC0FCFD2C92716C5AE537D9F8A4E9AC25CCD7C93696846";

    internal static bool IsCuratedPair(string archiveSha256, ReadOnlySpan<byte> registry) =>
        archiveSha256.Equals(ArchiveSha256, StringComparison.OrdinalIgnoreCase) &&
        OriginalV8EngineDonor.MatchesHash(registry, RegistrySha256);

    internal static bool MatchesLegacyEntry(int ordinal, string stableId, uint mode, uint sample) =>
        mode == 0 && sample == 0 && (uint)ordinal < OriginalV8EngineDonor.SampleCount &&
        string.Equals(stableId, OriginalV8EngineDonor.StableId(ordinal), StringComparison.Ordinal);
}
