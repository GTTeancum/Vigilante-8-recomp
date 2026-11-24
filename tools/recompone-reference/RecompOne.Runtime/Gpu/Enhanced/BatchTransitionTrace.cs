using System.Runtime.InteropServices;
using RecompOne.Runtime.Hle;

namespace RecompOne.Runtime.Enhanced;

/// <summary>Counts render batch changes without creating per-draw log strings.</summary>
internal sealed class BatchTransitionTrace
{
    internal readonly record struct State(
        bool Transparent, HleMaterialKind Material, bool DepthTest, bool DepthWrite);

    readonly record struct Transition(State From, State To);
    readonly Dictionary<Transition, long> _counts = [];

    internal void Record(State from, State to)
    {
        ref long count = ref CollectionsMarshal.GetValueRefOrAddDefault(
            _counts, new Transition(from, to), out _);
        count++;
    }

    internal string FormatTopTransitions() => string.Join(
        '|',
        _counts.Select(pair => (
                Label: $"{Format(pair.Key.From)}->{Format(pair.Key.To)}",
                Count: pair.Value))
            .OrderByDescending(pair => pair.Count)
            .ThenBy(pair => pair.Label, StringComparer.Ordinal)
            .Take(6)
            .Select(pair => $"{pair.Label}={pair.Count}"));

    internal void Clear() => _counts.Clear();

    static string Format(State state) =>
        $"{(state.Transparent ? 'T' : 'O')}:{state.Material}:" +
        $"{(state.DepthTest ? 1 : 0)}{(state.DepthWrite ? 1 : 0)}";
}
