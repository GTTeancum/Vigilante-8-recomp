using System.Runtime.InteropServices;
using System.Text;
using RecompOne.Runtime.Host;

int checks = 0;
void Check(bool condition, string name)
{
    if (!condition) throw new Exception(name);
    checks++;
}

// The setup window's former declaration. Its StringBuilder field cannot be
// marshalled inside a struct, so sizing it throws: that exception escaped the
// render callback and closed the game when "Choose BIN / CUE..." was clicked.
bool legacyThrew = false;
try { _ = Marshal.SizeOf<LegacyOpenFileName>(); }
catch (ArgumentException) { legacyThrew = true; }
Check(legacyThrew, "legacy StringBuilder declaration is rejected by the marshaller");

// OPENFILENAMEW is 152 bytes on x64; comdlg32 validates lStructSize.
Check(NativeFileDialog.StructSize == 152, $"OPENFILENAMEW size {NativeFileDialog.StructSize} != 152");

// A Linux install gets uppercase CUE extensions and case-sensitive paths.
var fixture = Path.Combine(Path.GetTempPath(), "v8-disc-dialog-" + Guid.NewGuid().ToString("N"));
Directory.CreateDirectory(fixture);
try
{
    var bin = Path.Combine(fixture, "Vigilante 8 Track 01.BIN");
    var cue = Path.Combine(fixture, "Vigilante 8.CUE");
    File.WriteAllText(bin, "");
    File.WriteAllText(cue, "FILE \"Vigilante 8 Track 01.BIN\" BINARY\n");
    if (!OperatingSystem.IsWindows())
        File.WriteAllText(Path.Combine(fixture, "wrong-case.cue"),
            "FILE \"vigilante 8 Track 01.BIN\" BINARY\n");
    Check(FirstRunSetup.ResolveCue(cue) == cue, "direct uppercase CUE is accepted");
    Check(FirstRunSetup.ResolveCue(bin) == cue, "BIN resolves its own uppercase CUE");
    string ResolveTypedPath(string path) =>
        FirstRunSetup.ResolveCue(FirstRunSetup.NormalizeTypedDiscPath(path));
    Check(ResolveTypedPath($" \"{cue}\" ") == cue, "pasted Copy as path selects a CUE");
    Check(ResolveTypedPath($"\"{bin}\"") == cue, "pasted Copy as path resolves a BIN to its CUE");
    Check(string.IsNullOrWhiteSpace(FirstRunSetup.NormalizeTypedDiscPath(" \"\" ")),
        "empty quoted path is treated as empty");
    bool missingRejected = false;
    try { _ = ResolveTypedPath($"\"{Path.Combine(fixture, "missing disc.cue")}\""); }
    catch (FileNotFoundException) { missingRejected = true; }
    Check(missingRejected, "pasting a nonexistent path does not bypass validation");

    File.WriteAllText(Path.Combine(fixture, "duplicate.cue"),
        "FILE \"Vigilante 8 Track 01.BIN\" BINARY\n");
    bool ambiguousRejected = false;
    try { _ = ResolveTypedPath($"\"{bin}\""); }
    catch (InvalidDataException) { ambiguousRejected = true; }
    Check(ambiguousRejected, "ambiguous CUE requires direct selection");
}
finally { Directory.Delete(fixture, recursive: true); }

Console.WriteLine($"[FileDialogContract] PASS checks={checks}");
return 0;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
struct LegacyOpenFileName
{
    public int Size; public nint Owner, Instance; public string Filter;
    public nint CustomFilter; public int MaxCustomFilter, FilterIndex;
    public StringBuilder File; public int MaxFile; public nint FileTitle;
    public int MaxFileTitle; public string? InitialDir, Title; public int Flags;
    public short FileOffset, FileExtension; public nint DefaultExtension, CustomData, Hook, TemplateName, Reserved;
    public int Reserved2, FlagsEx;
}
