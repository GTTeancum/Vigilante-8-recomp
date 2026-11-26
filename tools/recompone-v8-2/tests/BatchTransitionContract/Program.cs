using System.Diagnostics;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using State = RecompOne.Runtime.Enhanced.BatchTransitionTrace.State;

// Compare against the original renderer's text-key counter. Do not use the
// new formatter for the oracle: labels, tie order, and truncation matter.
static void LegacyRecord(Dictionary<string, long> counts, State from, State to)
{
    string label =
        $"{(from.Transparent ? 'T' : 'O')}:{from.Material}:" +
        $"{(from.DepthTest ? 1 : 0)}{(from.DepthWrite ? 1 : 0)}" +
        $"->{(to.Transparent ? 'T' : 'O')}:{to.Material}:" +
        $"{(to.DepthTest ? 1 : 0)}{(to.DepthWrite ? 1 : 0)}";
    counts[label] = counts.GetValueOrDefault(label) + 1;
}

static string LegacySummary(Dictionary<string, long> counts) => string.Join(
    '|', counts.OrderByDescending(pair => pair.Value)
        .ThenBy(pair => pair.Key, StringComparer.Ordinal)
        .Take(6).Select(pair => $"{pair.Key}={pair.Value}"));

var legacy = new Dictionary<string, long>();
var trace = new BatchTransitionTrace();
void Record(State from, State to)
{
    LegacyRecord(legacy, from, to);
    trace.Record(from, to);
}

void CheckSummary()
{
    if (trace.FormatTopTransitions() != LegacySummary(legacy))
        throw new Exception("Transition log text, counts, or ordering changed.");
    trace.Clear();
    legacy.Clear();
    if (trace.FormatTopTransitions() != "")
        throw new Exception("Transition data leaked into the next window.");
}

var states = (from material in Enum.GetValues<HleMaterialKind>().Append((HleMaterialKind)255)
              from flags in Enumerable.Range(0, 8)
              select new State((flags & 4) != 0, material,
                  (flags & 2) != 0, (flags & 1) != 0)).ToArray();

// More than six equally frequent keys, recorded in reverse order. A stable
// insertion-order tie-break would truncate away the wrong transitions.
foreach (var state in states.Reverse())
    Record(state, states[0]);
CheckSummary();

var rng = new Random(69114);
for (int window = 0; window < 10; window++)
{
    for (int i = 0; i < 3000; i++)
    {
        State from = states[rng.Next(states.Length)];
        State to = states[rng.Next(states.Length)];
        Record(from, to);
        if (i % 7 == 0) Record(to, from);
        if (i % 11 == 0) Record(from, to);
    }
    CheckSummary();
}

// Warm the same repeated keys (including dictionary capacity and JIT code)
// before measuring per-break allocations; formatting is outside the loop.
const int samples = 100_000;
void CountLegacy(int count)
{
    for (int i = 0; i < count; i++)
        LegacyRecord(legacy, states[i & 7], states[7 - (i & 7)]);
}
void CountTyped(int count)
{
    for (int i = 0; i < count; i++)
        trace.Record(states[i & 7], states[7 - (i & 7)]);
}
CountLegacy(samples);
CountTyped(samples);
CheckSummary();

long before = GC.GetAllocatedBytesForCurrentThread();
long start = Stopwatch.GetTimestamp();
CountLegacy(samples);
TimeSpan legacyTime = Stopwatch.GetElapsedTime(start);
long legacyBytes = GC.GetAllocatedBytesForCurrentThread() - before;
before = GC.GetAllocatedBytesForCurrentThread();
start = Stopwatch.GetTimestamp();
CountTyped(samples);
TimeSpan typedTime = Stopwatch.GetElapsedTime(start);
long typedBytes = GC.GetAllocatedBytesForCurrentThread() - before;
CheckSummary();
if (legacyBytes < samples * 32L || typedBytes != 0)
    throw new Exception($"Unexpected hot-path allocations: old={legacyBytes}, new={typedBytes}.");
Console.WriteLine($"PASS matching summaries, ordinal ties, and reset. {samples:N0} repeated transitions: " +
    $"{legacyBytes:N0} to {typedBytes:N0} allocated bytes, " +
    $"{legacyTime.TotalMilliseconds:F2} to {typedTime.TotalMilliseconds:F2} ms (synthetic).");
