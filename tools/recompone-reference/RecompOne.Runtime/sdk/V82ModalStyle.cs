using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Hle;

namespace RecompOne.Runtime.Sdk;

public static class V82ModalStyle
{
    public static IDisposable InsetChoice(CpuContext c, IMemory m) => new ChoiceInset(c, m);
    sealed class ChoiceInset : IDisposable
    {
        readonly IMemory memory;
        readonly uint address;
        readonly ushort x, width;
        public ChoiceInset(CpuContext c, IMemory m)
        {
            memory = m; address = c.A2;
            x = m.ReadU16(address); width = m.ReadU16(address + 4);
            // Both native carousel rows share their text box with < and >.
            // Inset only the centered value while it is measured/emitted;
            // restore before the native arrow calls use the original box.
            if (width > 24)
            {
                m.WriteU16(address, (ushort)(x + 12));
                m.WriteU16(address + 4, (ushort)(width - 24));
            }
        }
        public void Dispose()
        {
            memory.WriteU16(address, x);
            memory.WriteU16(address + 4, width);
        }
    }
    // The common native modal builder (80011BB8) emits a black tile followed
    // by a nine-slice frame. Replace presentation only: retain its rectangle,
    // ordering-table link and packet allocator. Pause/confirmation/objective
    // layout, text, controls and transitions remain in the original code.
    public static bool DrawPanel(CpuContext c, IMemory m)
    {
        if (!GpuHle.Active || !GpuHle.GameplayActive) return true;
        uint packet = m.ReadU32(c.GP + 0x610u);
        uint previous = m.ReadU32(c.A1);
        m.WriteU32(packet, 0x04000000u | (previous & 0x00FFFFFFu));
        m.WriteU32(packet + 4, 0xE1000600u);
        m.WriteU32(packet + 8, 0x62000000u);
        for (uint offset = 0; offset < 8; offset += 2)
            m.WriteU16(packet + 12 + offset, m.ReadU16(c.A0 + offset));
        m.WriteU32(c.A1, packet & 0x00FFFFFFu);
        m.WriteU32(c.GP + 0x610u, packet + 20);
        GpuHle.RegisterNativeModalPanel(packet);
        GpuHle.SignalNativeModal();
        return false;
    }
}
