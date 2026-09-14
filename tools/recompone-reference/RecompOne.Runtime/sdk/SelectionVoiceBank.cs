using System.Buffers.Binary;

namespace RecompOne.Runtime.Sdk;

// Immutable nonresident SND data. Only the accepted sample is uploaded to
// the selector's already allocated per-player SPU slot.
internal sealed class SelectionVoiceBank
{
    readonly byte[] _data;
    readonly int _headerBytes;
    readonly int[] _offsets;
    readonly ushort[] _pitches;
    public int Count => _pitches.Length;
    public int MaximumSampleBytes { get; }

    public SelectionVoiceBank(byte[] data)
    {
        if (data.Length < 4) throw new InvalidDataException("truncated voice SND");
        int count = BinaryPrimitives.ReadUInt16LittleEndian(data);
        int bytes = BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(2)) * 8;
        _headerBytes = 4 + count * 4;
        if (count == 0 || data.Length != _headerBytes + bytes)
            throw new InvalidDataException("invalid voice SND length");
        _data = (byte[])data.Clone();
        _offsets = new int[count + 1];
        _pitches = new ushort[count];
        _offsets[count] = bytes;
        for (int i = 0; i < count; i++)
        {
            _offsets[i] = BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(4 + i * 4)) * 8;
            _pitches[i] = BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(6 + i * 4));
        }
        for (int i = 0; i < count; i++)
        {
            int length = _offsets[i + 1] - _offsets[i];
            if ((i == 0 && _offsets[i] != 0) || length <= 0 || length % 16 != 0 ||
                _offsets[i + 1] > bytes || _pitches[i] == 0 || _pitches[i] > 0x3FFF)
                throw new InvalidDataException("invalid voice SND entry");
            MaximumSampleBytes = Math.Max(MaximumSampleBytes, length);
        }
    }

    public ushort Pitch(int index) => _pitches[index];
    public byte[] Sample(int index) => _data.AsSpan(
        _headerBytes + _offsets[index], _offsets[index + 1] - _offsets[index]).ToArray();
}
