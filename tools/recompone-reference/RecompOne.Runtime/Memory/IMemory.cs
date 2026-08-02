namespace RecompOne.Runtime.Memory;

/// <summary>
/// Exact vertex data captured before the PlayStation GTE quantizes a projected
/// coordinate into the 16-bit SXY FIFO.  The stock renderer deliberately
/// ignores this record.  The enhanced renderer uses it as its geometry seam,
/// so it does not have to reverse-engineer view-space position from a GPU
/// packet after the original rasterizer has discarded that information.
/// </summary>
public readonly record struct PreciseGteVertexData(
    uint PackedScreenPosition,
    ushort Depth,
    float PerspectiveW,
    float ViewX,
    float ViewY,
    float ViewZ,
    float PreciseX,
    float PreciseY,
    float ProjectionCenterX,
    float ProjectionCenterY,
    float ProjectionScale)
{
    // A vertex behind or on the camera plane is still valid enhanced-renderer
    // geometry.  The PS1 saturates its SZ/SXY outputs, but a modern clipper
    // needs the unsaturated view-space position to intersect the primitive
    // against the near plane correctly.  Positivity is therefore a raster
    // eligibility test, not a provenance-storage test.
    public bool Valid =>
        float.IsFinite(PerspectiveW) &&
        float.IsFinite(ViewX) &&
        float.IsFinite(ViewY) &&
        float.IsFinite(ViewZ) &&
        float.IsFinite(PreciseX) &&
        float.IsFinite(PreciseY) &&
        float.IsFinite(ProjectionCenterX) &&
        float.IsFinite(ProjectionCenterY) &&
            float.IsFinite(ProjectionScale) && ProjectionScale > 0;
}

public interface IMemory // https://psx-spx.consoledev.net/memorymap/
{
    byte ReadU8(uint address);
    ushort ReadU16(uint address);
    uint ReadU32(uint address);

    void WriteU8(uint address, byte value);
    void WriteU16(uint address, ushort value);
    void WriteU32(uint address, uint value);
    void WriteGteWord(uint address, int register);
    void WritePreciseGteVertex(
        uint address, in PreciseGteVertexData vertex);
    bool TryGetPreciseGteVertex(
        uint address, uint packedScreenPosition,
        out PreciseGteVertexData vertex);
    
    uint ReadWordLeft(uint current, uint address);
    uint ReadWordRight(uint current, uint address);
    
    void WriteWordLeft(uint address, uint value);
    void WriteWordRight(uint address, uint value);

    void LoadBytes(uint address, byte[] data);
    void ZeroRange(uint address, uint length);

    void SetCd(Cdrom.CdController cd);
}
