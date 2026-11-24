using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
namespace Recompiled;

// Never publish this synthetic qualification DLL.
public static class Entry
{
    public static void Run(IMemory memory, string? cue = null, string? loose = null, string? over = null)
    {
        if (memory is not PSMemory || cue != null || over != null || loose == null || !File.Exists(Path.Combine(loose, "qualification-sentinel")))
            throw new InvalidOperationException("The ordinary host entry did not forward the expected loose path.");
        File.WriteAllText(Path.Combine(loose, "qualification-entered.txt"), "entered\n");
    }
}
public static class Vigilante82PC
{
    public static void func_8005BC0C(CpuContext c, IMemory m) => throw new NotSupportedException();
    public static void func_8002091C(CpuContext c, IMemory m) => throw new NotSupportedException();
    public static void func_800209C8(CpuContext c, IMemory m) => throw new NotSupportedException();
    public static void func_801109FC(CpuContext c, IMemory m) => throw new NotSupportedException();
}
public sealed class MainDispatchTable : IOverlay
{
    public string Name => "main";
    public IReadOnlyDictionary<uint, Action<CpuContext, IMemory>> Functions { get; } = new Dictionary<uint, Action<CpuContext, IMemory>>();
}
public sealed class SHELL_SHELLDispatchTable : IOverlay
{
    public string Name => "SHELL_SHELL";
    public IReadOnlyDictionary<uint, Action<CpuContext, IMemory>> Functions { get; } = new Dictionary<uint, Action<CpuContext, IMemory>>();
}
