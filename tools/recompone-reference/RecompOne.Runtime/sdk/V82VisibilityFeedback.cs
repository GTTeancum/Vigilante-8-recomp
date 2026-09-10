using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static partial class V82Compat
{
    static readonly bool PipelinedFeedback = Environment.GetEnvironmentVariable("RECOMPONE_PIPELINED_FEEDBACK") != "0";
    static readonly bool VerifyPipelinedFeedback = Environment.GetEnvironmentVariable("RECOMPONE_VERIFY_PIPELINED_FEEDBACK") == "1";
    sealed record FeedbackView(uint[] Records, uint CenterX, uint CenterY, int Start, ushort[]? Oracle);
    static readonly List<FeedbackView> FeedbackViews = [];
    static readonly ushort[] FeedbackPixels = new ushort[4096];
    static bool FeedbackRead;
    static long FeedbackVerified;

    public static bool BeginPipelinedFeedback()
    {
        FeedbackViews.Clear(); FeedbackRead = false;
        if (!PipelinedFeedback || !DirectSceneRendering ||
            GpuHle.Backend is not Enhanced.EnhancedGlBackend { Ready:true } backend) return false;
        backend.BeginVisibilityProbes();
        return true;
    }

    public static void QueueCurrentFeedback(CpuContext c, IMemory m)
    {
        FeedbackRead = false;
        m = Dispatcher.UnwrapMemory(m);
        uint pool = ExpandedEdgePoolBase(m.ReadU32(c.GP+0x20)&1);
        int count = checked((int)m.ReadU32(pool));
        if (count < 0 || count > ExpandedEdgePoolCapacity) throw new InvalidOperationException("feedback pool count");
        var words = new uint[count*4];
        var points = new int[count*2];
        for (int i=0; i<count; i++)
        {
            uint record = pool+4+(uint)i*16;
            for (int j=0; j<4; j++) words[i*4+j] = m.ReadU32(record+(uint)j*4);
            points[i*2] = (int)(m.ReadU16(record+12)+c.A0)&1023;
            points[i*2+1] = (int)(m.ReadU16(record+14)+c.A1)&511;
        }
        var backend = (Enhanced.EnhancedGlBackend)GpuHle.Backend!;
        int start = backend.QueueVisibilityProbes(points);
        ushort[]? oracle = null;
        if (VerifyPipelinedFeedback)
        {
            oracle = new ushort[count];
            for (int i=0; i<count; i++) backend.ReadVram(points[i*2],points[i*2+1],1,1,oracle.AsSpan(i,1));
        }
        FeedbackViews.Add(new(words,Gte.ReadControl(24),Gte.ReadControl(25),start,oracle));
        m.WriteU32(pool,0);
    }

    public static void CompleteFeedbackView(CpuContext c, IMemory m, int player, uint hud)
    {
        var backend = (Enhanced.EnhancedGlBackend)GpuHle.Backend!;
        if (!FeedbackRead) { backend.ReadVisibilityProbes(FeedbackPixels); FeedbackRead=true; }
        var view = FeedbackViews[player];
        var state = c.Snapshot();
        uint centerX=Gte.ReadControl(24), centerY=Gte.ReadControl(25);
        uint scratch=c.SP-0x80;
        Gte.WriteControl(24,view.CenterX); Gte.WriteControl(25,view.CenterY);
        try
        {
            for (int i=0; i<view.Records.Length/4; i++)
            {
                ushort pixel=FeedbackPixels[view.Start+i];
                if (view.Oracle is { } oracle)
                {
                    if (pixel!=oracle[i]) throw new InvalidOperationException($"Pipelined feedback mismatch player={player} index={i}: {pixel:X4}!={oracle[i]:X4}");
                    FeedbackVerified++;
                }
                if ((pixel&0x7FFF)!=0x7FFF) continue;
                for (int j=0; j<4; j++) m.WriteU32(scratch+(uint)j*4,view.Records[i*4+j]);
                c.Restore(state); c.SP=scratch-0x38; c.A0=scratch; c.A1=hud; c.RA=0x8001D534;
                Dispatcher.Call(c,m,0x8001D00C);
            }
        }
        finally { c.Restore(state); Gte.WriteControl(24,centerX); Gte.WriteControl(25,centerY); }
        if (player==3 && VerifyPipelinedFeedback && GpuHle.DebugGameplayTick%120==0)
            Console.Error.WriteLine($"[PipelinedFeedbackVerified] samples={FeedbackVerified}");
    }
}
