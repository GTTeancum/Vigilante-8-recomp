using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Context;

public sealed class CpuContext
{
    private uint[] _gpr = new uint[32];
    private PreciseGteVertexData[] _gteVertices =
        new PreciseGteVertexData[32];

    public uint At { get => _gpr[1];  set => Set(1, value); }
    public uint V0 { get => _gpr[2];  set => Set(2, value); }
    public uint V1 { get => _gpr[3];  set => Set(3, value); }
    public uint A0 { get => _gpr[4];  set => Set(4, value); }
    public uint A1 { get => _gpr[5];  set => Set(5, value); }
    public uint A2 { get => _gpr[6];  set => Set(6, value); }
    public uint A3 { get => _gpr[7];  set => Set(7, value); }
    public uint T0 { get => _gpr[8];  set => Set(8, value); }
    public uint T1 { get => _gpr[9];  set => Set(9, value); }
    public uint T2 { get => _gpr[10]; set => Set(10, value); }
    public uint T3 { get => _gpr[11]; set => Set(11, value); }
    public uint T4 { get => _gpr[12]; set => Set(12, value); }
    public uint T5 { get => _gpr[13]; set => Set(13, value); }
    public uint T6 { get => _gpr[14]; set => Set(14, value); }
    public uint T7 { get => _gpr[15]; set => Set(15, value); }
    public uint S0 { get => _gpr[16]; set => Set(16, value); }
    public uint S1 { get => _gpr[17]; set => Set(17, value); }
    public uint S2 { get => _gpr[18]; set => Set(18, value); }
    public uint S3 { get => _gpr[19]; set => Set(19, value); }
    public uint S4 { get => _gpr[20]; set => Set(20, value); }
    public uint S5 { get => _gpr[21]; set => Set(21, value); }
    public uint S6 { get => _gpr[22]; set => Set(22, value); }
    public uint S7 { get => _gpr[23]; set => Set(23, value); }
    public uint T8 { get => _gpr[24]; set => Set(24, value); }
    public uint T9 { get => _gpr[25]; set => Set(25, value); }
    public uint K0 { get => _gpr[26]; set => Set(26, value); }
    public uint K1 { get => _gpr[27]; set => Set(27, value); }
    public uint GP { get => _gpr[28]; set => Set(28, value); }
    public uint SP { get => _gpr[29]; set => Set(29, value); }
    public uint FP { get => _gpr[30]; set => Set(30, value); }
    public uint RA { get => _gpr[31]; set => Set(31, value); }

    public uint HI;
    public uint LO;
    
    public uint SR; 
    public uint Cause; 
    public uint EPC;
    public uint BadVAddr; 
    public uint PRId; 
    
    public uint this[int index]
    {
        get => index == 0 ? 0u : _gpr[index];
        set { if (index != 0) Set(index, value); }
    }

    void Set(int index, uint value)
    {
        _gpr[index] = value;
        _gteVertices[index] = default;
    }

    public void CopyRegister(int destination, int source)
    {
        if (destination == 0) return;
        _gpr[destination] = source == 0 ? 0 : _gpr[source];
        _gteVertices[destination] =
            source == 0 ? default : _gteVertices[source];
    }

    public void LoadWord(int destination, IMemory memory, uint address)
    {
        if (destination == 0) return;
        uint value = memory.ReadU32(address);
        _gpr[destination] = value;
        if (memory.TryGetPreciseGteVertex(
                address, value, out PreciseGteVertexData vertex))
            _gteVertices[destination] = vertex;
        else
            _gteVertices[destination] = default;
    }

    public void LoadGteWord(int destination, int register)
    {
        if (destination == 0) return;
        bool precise = Gte.TryGetStoreVertex(register, out var vertex);
        _gpr[destination] =
            precise ? vertex.PackedScreenPosition : Gte.Read(register);
        _gteVertices[destination] = precise ? vertex : default;
    }

    public void StoreWord(int source, IMemory memory, uint address)
    {
        uint value = source == 0 ? 0 : _gpr[source];
        if (source != 0 && _gteVertices[source].Valid)
            memory.WritePreciseGteVertex(address, _gteVertices[source]);
        else
            memory.WriteU32(address, value);
    }

    public (
        uint[] gpr, PreciseGteVertexData[] gteVertices,
        uint hi, uint lo) Snapshot() =>
        ((uint[])_gpr.Clone(),
         (PreciseGteVertexData[])_gteVertices.Clone(),
         HI, LO);

    public void Restore((
        uint[] gpr, PreciseGteVertexData[] gteVertices,
        uint hi, uint lo) s)
    {
        Array.Copy(s.gpr, _gpr, 32);
        Array.Copy(s.gteVertices, _gteVertices, 32);
        HI = s.hi;
        LO = s.lo;
    }
}
