using System.Diagnostics;

namespace RecompOne.Runtime.Host;

internal static class FrameClock
{
    const double FramesPerSecond = 60.0;
    const double SpinReserveMs = 1.5;
    const double ResyncThresholdMs = 100.0;

    static readonly double TicksPerFrame =
        Stopwatch.Frequency / FramesPerSecond;
    static readonly double TicksPerMillisecond =
        Stopwatch.Frequency / 1000.0;
    static readonly bool Unthrottled =
        Environment.GetEnvironmentVariable("RECOMPONE_UNTHROTTLED") == "1";
    static double _nextFrameTicks;
    static long _requestedSleepTicks;
    static long _actualSleepTicks;
    static long _spinTicks;
    static long _maximumSleepOvershootTicks;
    static long _maximumDeadlineMissTicks;
    static int _deadlineMisses;
    static int _resyncs;

    public readonly record struct Metrics(
        double RequestedSleepMs,
        double ActualSleepMs,
        double SpinMs,
        double MaximumSleepOvershootMs,
        double MaximumDeadlineMissMs,
        int DeadlineMisses,
        int Resyncs);

    public static void Throttle()
    {
        if (Unthrottled)
            return;

        long now = Stopwatch.GetTimestamp();
        if (_nextFrameTicks == 0)
            _nextFrameTicks = now;
        _nextFrameTicks += TicksPerFrame;

        double remainingTicks = _nextFrameTicks - now;
        if (remainingTicks < -ResyncThresholdMs * TicksPerMillisecond)
        {
            // A debugger stop, loading transition, or host suspension should
            // not make the game race through seconds of missed presents.
            _nextFrameTicks = now;
            _resyncs++;
            return;
        }

        double remainingMs = remainingTicks / TicksPerMillisecond;
        int sleepMs = (int)Math.Floor(remainingMs - SpinReserveMs);
        if (sleepMs > 0)
        {
            long sleepStarted = Stopwatch.GetTimestamp();
            Thread.Sleep(sleepMs);
            long sleepFinished = Stopwatch.GetTimestamp();
            long actualSleepTicks = sleepFinished - sleepStarted;
            long requestedSleepTicks = (long)(sleepMs * TicksPerMillisecond);
            _requestedSleepTicks += requestedSleepTicks;
            _actualSleepTicks += actualSleepTicks;
            _maximumSleepOvershootTicks = Math.Max(
                _maximumSleepOvershootTicks,
                Math.Max(0, actualSleepTicks - requestedSleepTicks));
            now = sleepFinished;
        }

        if (now < _nextFrameTicks)
        {
            long spinStarted = now;
            do
            {
                // Reserve the short tail for a monotonic busy wait. The old
                // integer Thread.Sleep(wait) routinely overshot Windows timer
                // quanta and turned a frame with CPU headroom into 30-50 FPS.
                Thread.SpinWait(32);
                now = Stopwatch.GetTimestamp();
            }
            while (now < _nextFrameTicks);
            _spinTicks += now - spinStarted;
        }

        long deadlineMissTicks = Math.Max(
            0,
            now - (long)_nextFrameTicks);
        if (deadlineMissTicks > TicksPerMillisecond)
        {
            _deadlineMisses++;
            _maximumDeadlineMissTicks = Math.Max(
                _maximumDeadlineMissTicks,
                deadlineMissTicks);
        }
    }

    public static Metrics ConsumeMetrics()
    {
        var metrics = new Metrics(
            _requestedSleepTicks / TicksPerMillisecond,
            _actualSleepTicks / TicksPerMillisecond,
            _spinTicks / TicksPerMillisecond,
            _maximumSleepOvershootTicks / TicksPerMillisecond,
            _maximumDeadlineMissTicks / TicksPerMillisecond,
            _deadlineMisses,
            _resyncs);
        _requestedSleepTicks = 0;
        _actualSleepTicks = 0;
        _spinTicks = 0;
        _maximumSleepOvershootTicks = 0;
        _maximumDeadlineMissTicks = 0;
        _deadlineMisses = 0;
        _resyncs = 0;
        return metrics;
    }
}
