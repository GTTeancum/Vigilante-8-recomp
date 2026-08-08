using RecompOne.Recompiler.Disasm;

namespace RecompOne.Recompiler.Analysis;

public sealed class MipsFunction
{
    public string Name = "";
    public string OverlayName = "";
    public uint Start;
    public uint End;
    public MipsInstruction[] Instructions = [];
    public string EmittedName = "";
    public bool IsStub;
    public bool IsPatch;
    public string PatchTarget = "";
    public string PreHookTarget = "";
    public string PostHookTarget = "";
    public List<JumpTable> JumpTables = [];

    /// <summary>
    /// Calls spliced into the middle of a function at a named instruction
    /// address. Function-level pre/post hooks cannot express a seam inside a
    /// long routine -- the native shell menus need exactly that -- and without
    /// this such call sites survive only as hand edits to generated output,
    /// which is regenerated from the disc and therefore loses them.
    /// </summary>
    public List<InlineHook> InlineHooks = [];
}

/// <summary>
/// A call emitted next to the instruction at <paramref name="Address"/>.
/// When <paramref name="BranchTo"/> is set the target must return bool and the
/// emitted call becomes a conditional jump to that label, which is how a hook
/// takes over the remainder of a routine.
/// </summary>
public sealed record InlineHook(
    uint Address, string Target, bool After, uint? BranchTo)
{
}
