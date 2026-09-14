namespace RecompOne.Runtime.Hle;

public struct HleVertex
{
    public uint SourceAddress;
    public float X, Y; 
    public float Z; 
    public float PerspectiveW;
    public float ViewX, ViewY, ViewZ;
    public float ProjectionCenterX, ProjectionCenterY, ProjectionScale;
    public float AuthoredMinU, AuthoredMinV, AuthoredMaxU, AuthoredMaxV;
    public byte R, G, B;
    public byte TerrainOffsetR, TerrainOffsetG, TerrainOffsetB;
    public short U, V;
    public bool HasGteZ;
    public bool HasCoherentGteZ;
    public bool HasProjectiveW;
    public bool HasViewSpace;
    public bool ReconstructedViewSpace;
    public bool HasAuthoredUvBounds;
    public bool DreamcastTerrainColor;
}

public enum HleMaterialKind : byte
{
    Auto,
    Opaque,
    AlphaTest,
    Glass,
    Particle,
    Additive,
    Subtractive,
    Ui,
    ScreenEffect,
    TerrainRoute,
    VehicleReflection,
    WaterBase,
    WaterSurface,
}

public struct PrimFlags
{
    public bool Textured, SemiTrans, RawTexture, Gouraud, Vehicle;
    public bool WorldObject, TerrainRoute, N64RouteColor;
    public bool N64RouteDepthCompare;
    public bool NativeRoad;
    public int NativeRoadPrioritySz;
    public bool DreamcastTerrainDepth;
    public HleMaterialKind Material;
    public ushort TPage; 
    public ushort Clut; 
    public int OtIndex;
    public uint PacketAddress;
    public uint WaterBaseGroup;
    public GpuHle.DreamcastWaterBaseQuad? WaterBaseQuad;
    public uint WaterSurfaceGroup;
    public GpuHle.DreamcastWaterSurfaceMesh? WaterSurfaceMesh;

    public readonly int BlendMode => (TPage >> 5) & 3;
}

public struct HleRect
{
    public float X, Y;
    public int W, H;
    public short U, V;
    public byte R, G, B;
    public bool FlipX, FlipY;
}
public struct HleDrawEnv { public int ClipX0, ClipY0, ClipX1, ClipY1; public int DrawOffsetX, DrawOffsetY; public int TwMaskX, TwMaskY, TwOffX, TwOffY; public bool SetMask, CheckMask, Dither; }
public struct HleDispEnv { public int X, Y, W, H; public bool Rgb24, Interlace; }
