using System.Buffers.Binary;

namespace RecompOne.Runtime;

public static class CddaAudio
{
    const int Capacity = 1 << 17;
    const int Mask = Capacity - 1;
    const int PrimeFrames = 1176;

    static readonly int[] _ring = new int[Capacity];
    static readonly object _gate = new();
    static readonly bool _trace =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_AUDIO") == "1";

    static int _writeIdx;
    static int _readIdx;
    static int _count;
    static bool _playing;
    static long _sectorsQueued;

    public static void Reset()
    {
        lock (_gate)
        {
            _writeIdx = _readIdx = _count = 0;
            _playing = false;
            _sectorsQueued = 0;
        }
    }

    public static void QueueSector(ReadOnlySpan<byte> sector, int trackNumber, int lba)
    {
        if (sector.Length < 2352)
            throw new InvalidDataException("CD-DA sector is shorter than 2352 bytes");

        lock (_gate)
        {
            int sectorPeak = 0;
            for (int offset = 0; offset < 2352; offset += 4)
            {
                short left = BinaryPrimitives.ReadInt16LittleEndian(sector.Slice(offset, 2));
                short right = BinaryPrimitives.ReadInt16LittleEndian(sector.Slice(offset + 2, 2));
                sectorPeak = Math.Max(sectorPeak, Math.Abs((int)left));
                sectorPeak = Math.Max(sectorPeak, Math.Abs((int)right));
                _ring[_writeIdx] = (ushort)left | (right << 16);
                _writeIdx = (_writeIdx + 1) & Mask;
                if (_count < Capacity) _count++;
                else _readIdx = (_readIdx + 1) & Mask;
            }
            long queued = ++_sectorsQueued;
            if (!_playing && _count >= PrimeFrames) _playing = true;
            if (_trace && (queued == 1 || queued % 75 == 0))
            {
                Console.Error.WriteLine(
                    $"[CDDA] track={trackNumber} lba={lba} sectors={queued} " +
                    $"buffered={_count} peak={sectorPeak}");
            }
        }
    }

    public static int BufferedFrames { get { lock (_gate) return _count; } }
    public static bool Playing { get { lock (_gate) return _playing; } }

    public static bool Next(out short left, out short right)
    {
        lock (_gate)
        {
            if (!_playing || _count == 0)
            {
                _playing = false;
                left = right = 0;
                return false;
            }

            int packed = _ring[_readIdx];
            _readIdx = (_readIdx + 1) & Mask;
            _count--;
            left = (short)(packed & 0xFFFF);
            right = (short)(packed >> 16);
            return true;
        }
    }
}
