using System.Reflection;
using System.Runtime.Loader;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace Recompiled;

// The normal host's tiny boundary stays strongly typed. Once Entry.Run starts,
// the private guest calls its original generated functions directly.
internal static class PairedModule
{
    private static readonly Lazy<Assembly> Guest = new(Load);

    private static string Tree(Assembly assembly)
    {
        string version = assembly.GetCustomAttribute<AssemblyInformationalVersionAttribute>()?.InformationalVersion ?? "";
        int plus = version.LastIndexOf('+');
        string id = plus < 0 ? "" : version[(plus + 1)..];
        return id.Length == 40 && id.All(char.IsAsciiHexDigit) ? id.ToLowerInvariant()
            : throw new InvalidDataException($"{assembly.GetName().Name} has no paired source-tree stamp.");
    }

    private static Assembly Load()
    {
        string path = Path.Combine(Runtime.ExecutableDirectory, "Vigilante82PC.dll");
        if (!File.Exists(path)) throw new FileNotFoundException("Private game module Vigilante82PC.dll is missing. Extract the complete matched test ZIP.");
        var assembly = AssemblyLoadContext.Default.LoadFromAssemblyPath(path);
        if (assembly.GetName().Name != "Vigilante82PC" || Tree(assembly) != Tree(typeof(PairedModule).Assembly))
            throw new InvalidDataException("The Windows program and game module are from different builds. Extract both from the same ZIP.");
        bool fixture = assembly.GetCustomAttributes<AssemblyMetadataAttribute>().Any(x => x.Key == "V8QualificationFixture");
        if (fixture && Environment.GetEnvironmentVariable("V8_TEST_SYNTHETIC_MODULE") != "1")
            throw new InvalidDataException("A CI fixture is not a playable game module.");
        return assembly;
    }

    internal static Type Type(string name) => Guest.Value.GetType("Recompiled." + name, throwOnError: true)!;
    internal static T Bind<T>(string type, string name) where T : Delegate =>
        (Type(type).GetMethod(name, BindingFlags.Public | BindingFlags.Static) ?? throw new MissingMethodException(type, name)).CreateDelegate<T>();
    internal static IOverlay Overlay(string name) => (IOverlay)Activator.CreateInstance(Type(name))!;
    internal static void Check()
    {
        _ = Guest.Value; // Surface missing file/mismatched tree before static field initialization.
        _ = Entry.Start;
        _ = Vigilante82PC.func_8005BC0C;
        _ = Vigilante82PC.func_8002091C;
        _ = Vigilante82PC.func_800209C8;
        _ = Vigilante82PC.func_801109FC;
        foreach (string name in new[] { "MainDispatchTable", "SHELL_SHELLDispatchTable" })
            if (!typeof(IOverlay).IsAssignableFrom(Type(name))) throw new InvalidDataException("Unexpected overlay ABI: " + name);
    }
}

internal static class Entry
{
    internal static readonly Action<IMemory, string?, string?, string?> Start = PairedModule.Bind<Action<IMemory, string?, string?, string?>>("Entry", "Run");
    internal static void Run(IMemory m, string? cue, string? loose) => Start(m, cue, loose, null);
}

// The selector-portrait probe is the only caller of these four leaves.
internal static class Vigilante82PC
{
    private static Action<CpuContext, IMemory> Bind(string name) => PairedModule.Bind<Action<CpuContext, IMemory>>("Vigilante82PC", name);
    internal static readonly Action<CpuContext, IMemory> func_8005BC0C = Bind(nameof(func_8005BC0C));
    internal static readonly Action<CpuContext, IMemory> func_8002091C = Bind(nameof(func_8002091C));
    internal static readonly Action<CpuContext, IMemory> func_800209C8 = Bind(nameof(func_800209C8));
    internal static readonly Action<CpuContext, IMemory> func_801109FC = Bind(nameof(func_801109FC));
}

internal abstract class OverlayProxy(string typeName) : IOverlay
{
    private readonly IOverlay target = PairedModule.Overlay(typeName);
    public string Name => target.Name;
    public int LbaStart => target.LbaStart;
    public uint Base => target.Base;
    public uint Size => target.Size;
    public uint ImageSize => target.ImageSize;
    public IReadOnlyDictionary<uint, Action<CpuContext, IMemory>> Functions => target.Functions;
}
internal sealed class MainDispatchTable() : OverlayProxy(nameof(MainDispatchTable));
internal sealed class SHELL_SHELLDispatchTable() : OverlayProxy(nameof(SHELL_SHELLDispatchTable));
