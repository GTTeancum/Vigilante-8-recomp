namespace RecompOne.Runtime.Hle;

/// <summary>
/// GPU BACKEND
/// </summary>
public interface IGpuBackend
{
    // false in headless or if gl init failed
    bool Ready { get; }

    // submit
    void SetDrawEnv(in HleDrawEnv env);
    void DrawTri(in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f);
    void DrawRect(in HleRect r, in PrimFlags f);
    void DrawLine(in HleVertex a, in HleVertex b, in PrimFlags f);
    void FillRect(int x, int y, int w, int h, ushort color15);
    void CopyVram(int sx, int sy, int dx, int dy, int w, int h);
    void WriteVram(int x, int y, int w, int h, ReadOnlySpan<ushort> px);
    void ReadVram(int x, int y, int w, int h, Span<ushort> px);

    // these touch gl
    void Flush();
    void Present(in HleDispEnv disp);

    // Drop submission state whose meaning is scoped to one gameplay scene.
    // Native VRAM and display targets deliberately survive menu transitions.
    void ResetTransientState() { }

    // Forget atmosphere inferred from the previous gameplay scene without
    // disturbing packet ownership or other renderer state still used while
    // the shell hands control to the next arena.
    void ResetAtmosphereState() { }
}
