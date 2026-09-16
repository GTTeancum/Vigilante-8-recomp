namespace RecompOne.Runtime.Sdk;

/// <summary>Lifetime tracking for identical, immutable native bank index images.</summary>
public sealed class NativeImageSharing
{
    sealed class Entry(string key, uint descriptor, uint x, uint y, uint words)
    {
        public readonly string Key = key;
        public readonly uint Descriptor = descriptor, X = x, Y = y, Words = words;
        public int References = 1;
    }
    readonly Dictionary<string, Entry> _images = new();
    readonly Dictionary<(uint X, uint Y), Entry> _coordinates = new();
    public int Hits { get; private set; }
    public ulong SavedWords { get; private set; }

    public bool TryAcquire(string key, out uint descriptor)
    {
        descriptor = 0;
        if (!_images.TryGetValue(key, out var entry)) return false;
        entry.References++;
        descriptor = entry.Descriptor;
        Hits++;
        SavedWords += entry.Words;
        return true;
    }

    public void Record(string key, uint descriptor, uint x, uint y, uint words)
    {
        // Post-allocation also runs after TryAcquire; count that owner once.
        if (_images.ContainsKey(key)) return;
        var entry = new Entry(key, descriptor, x, y, words);
        _images.Add(key, entry);
        _coordinates.Add((x, y), entry);
    }

    // True lets the original allocator free the backing rectangle. Unknown
    // coordinates belong to ordinary allocations, outside this cache.
    public bool Release(uint x, uint y)
    {
        if (!_coordinates.TryGetValue((x, y), out var entry)) return true;
        if (--entry.References != 0) return false;
        _coordinates.Remove((x, y));
        _images.Remove(entry.Key);
        return true;
    }

    public void Clear()
    {
        _images.Clear();
        _coordinates.Clear();
        Hits = 0;
        SavedWords = 0;
    }
}
