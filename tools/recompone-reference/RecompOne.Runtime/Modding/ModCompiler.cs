namespace RecompOne.Runtime.Modding;

public static class ModCompiler
{
    public static byte[]? Compile(
        string modId, IReadOnlyList<(string Path, string Text)> sources)
    {
        Console.Error.WriteLine(
            $"[Mods] {modId}: source-code mods are disabled in the standalone runtime; " +
            "runtime hooks are hard-coded into the executable.");
        return null;
    }
}
