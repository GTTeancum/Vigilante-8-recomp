using System.Reflection;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class DisplayCallbackChecks
{
    public static int Run()
    {
        var method = typeof(V82Compat).GetMethod("DrainPendingDisplayFlip",
            BindingFlags.Static | BindingFlags.NonPublic)!;
        var m = new PSMemory();
        const uint slot = 0x80065470;
        void Drain(Action present) => method.Invoke(null, new object[] { m, present });
        int frames = 0;
        // Unrelated installed VSync callbacks do not require a display wait.
        m.WriteU32(slot, 0x80015104);
        Drain(() => frames++);
        if (frames != 0) throw new Exception("Unrelated callback must not add a frame");
        m.WriteU32(slot, 0x80014BE4);
        Drain(() => { frames++; m.WriteU32(slot, 0x80015104); });
        if (frames != 1 || m.ReadU32(slot) != 0x80015104)
            throw new Exception("Pending flip must restore predecessor before next completion");
        m.WriteU32(slot, 0);
        Drain(() => frames++);
        if (frames != 1) throw new Exception("Absent callback must not wait");
        m.WriteU32(slot, 0x80014BE4);
        try { Drain(() => frames++); }
        catch (TargetInvocationException e) when (e.InnerException is InvalidOperationException)
        {
            if (frames != 17) throw new Exception("Stuck flip must have a bounded wait");
            return 4;
        }
        throw new Exception("Unfinished flip must not silently report completion");
    }
}
