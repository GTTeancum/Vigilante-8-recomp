// Generated from original SLUS_005.10, not from the port under test.
// Source body SHA256: f2ff73e174b6e76346f447e5bf1a7bb4586ab68191d932798f8a3a3b33f8dae4
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
static class OriginalProjectileOracle
{
    public static void LAB_800359c0(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x38u;
        c.StoreWord(18, m, (c.SP + 0x28u));
        c.CopyRegister(18, 4);
        c.StoreWord(17, m, (c.SP + 0x24u));
        c.CopyRegister(17, 6);
        c.StoreWord(19, m, (c.SP + 0x2Cu));
        c.S3 = 0x00000003u;
        c.StoreWord(31, m, (c.SP + 0x30u));
        if (c.A1 == c.S3) {
            c.StoreWord(16, m, (c.SP + 0x20u));
            goto L80035B78;
        }
        c.StoreWord(16, m, (c.SP + 0x20u));
        c.V0 = c.A1 < 0x00000004u ? 1u : 0u;
        if (c.V0 == 0u) {
            c.V0 = 0x00000005u;
            goto L80035A04;
        }
        c.V0 = 0x00000005u;
        if (c.A1 == 0u) {
            c.CopyRegister(2, 0);
            goto L80035A14;
        }
        c.CopyRegister(2, 0);
        goto L80035CDC;
        L80035A04: ;
        if (c.A1 == c.V0) {
            c.CopyRegister(2, 0);
            goto L80035CC8;
        }
        c.CopyRegister(2, 0);
        goto L80035CDC;
        L80035A14: ;
        c.LoadWord(2, m, (c.S2 + 0x24u));
        c.LoadWord(3, m, (c.S2 + 0x88u));
        c.A0 = c.S2 + 0x24u;
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S2 + 0x24u));
        c.LoadWord(2, m, (c.A0 + 0x4u));
        c.A2 = c.S2 + 0x88u;
        c.LoadWord(5, m, (c.A2 + 0x4u));
        c.LoadWord(3, m, (c.A0 + 0x8u));
        c.V0 = c.V0 + c.A1;
        c.StoreWord(2, m, (c.A0 + 0x4u));
        c.LoadWord(2, m, (c.A2 + 0x8u));
        c.V1 = c.V1 + c.V0;
        c.StoreWord(3, m, (c.A0 + 0x8u));
        c.LoadWord(8, m, (c.S2 + 0x24u));
        c.LoadWord(9, m, (c.S2 + 0x28u));
        c.LoadWord(10, m, (c.S2 + 0x2Cu));
        c.StoreWord(8, m, (c.S2 + 0x48u));
        c.StoreWord(9, m, (c.S2 + 0x4Cu));
        c.StoreWord(10, m, (c.S2 + 0x50u));
        c.V0 = m.ReadU16((c.S2 + 0x96u));
        c.V0 = c.V0 - 0x1u;
        m.WriteU16((c.S2 + 0x96u), (ushort)c.V0);
        c.V0 = c.V0 << 16;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L80035CDC;
        }
        c.CopyRegister(2, 0);
        c.V0 = (uint)(short)m.ReadU16((c.S2 + 0x94u));
        if (c.V0 == 0u) {
            c.A2 = 0x00000098u;
            goto L80035CD8;
        }
        c.A2 = 0x00000098u;
        c.LoadWord(4, m, (c.S2 + 0x58u));
        c.A1 = m.ReadU16((c.S2 + 0xAu));
        c.A3 = 0x00000008u;
        c.RA = 0x80035AA4u;
        Dispatcher.Call(c, m, 0x8001ac44u);
        c.CopyRegister(4, 18);
        c.A1 = 0u | 0x8000u;
        c.CopyRegister(16, 2);
        c.RA = 0x80035AB4u;
        Dispatcher.Call(c, m, 0x8001b038u);
        c.A1 = m.ReadU16((c.S2 + 0x6u));
        c.A0 = 0x01800000u;
        c.A0 = c.A0 | 0x0084u;
        c.V1 = 0x00000007u;
        m.WriteU8((c.S0 + 0x4u), (byte)c.V1);
        c.StoreWord(4, m, c.S0);
        m.WriteU16((c.S0 + 0x6u), (ushort)c.A1);
        c.A1 = m.ReadU16((c.S2 + 0xCu));
        c.CopyRegister(4, 16);
        c.V1 = 0x80030000u;
        c.V1 = c.V1 + 0x59C0u;
        c.CopyRegister(17, 2);
        c.StoreWord(3, m, (c.S0 + 0x64u));
        m.WriteU16((c.S0 + 0xCu), (ushort)c.A1);
        c.RA = 0x80035AF0u;
        Dispatcher.Call(c, m, 0x8002036cu);
        c.V0 = m.ReadU16((c.S2 + 0x94u));
        m.WriteU16((c.S0 + 0x96u), (ushort)c.S3);
        c.V0 = c.V0 - 0x1u;
        m.WriteU16((c.S0 + 0x94u), (ushort)c.V0);
        c.LoadWord(8, m, (c.S2 + 0x88u));
        c.LoadWord(9, m, (c.S2 + 0x8Cu));
        c.LoadWord(10, m, (c.S2 + 0x90u));
        c.StoreWord(8, m, (c.S0 + 0x88u));
        c.StoreWord(9, m, (c.S0 + 0x8Cu));
        c.StoreWord(10, m, (c.S0 + 0x90u));
        c.LoadWord(2, m, (c.S2 + 0x80u));
        c.A0 = c.S2 + 0x10u;
        c.A1 = c.S1 + 0x4u;
        c.StoreWord(2, m, (c.S0 + 0x80u));
        c.LoadWord(8, m, (c.S2 + 0x10u));
        c.LoadWord(9, m, (c.S2 + 0x14u));
        c.LoadWord(10, m, (c.S2 + 0x18u));
        c.LoadWord(11, m, (c.S2 + 0x1Cu));
        c.StoreWord(8, m, (c.S0 + 0x10u));
        c.StoreWord(9, m, (c.S0 + 0x14u));
        c.StoreWord(10, m, (c.S0 + 0x18u));
        c.StoreWord(11, m, (c.S0 + 0x1Cu));
        c.LoadWord(8, m, (c.S2 + 0x20u));
        c.LoadWord(9, m, (c.S2 + 0x24u));
        c.LoadWord(10, m, (c.S2 + 0x28u));
        c.LoadWord(11, m, (c.S2 + 0x2Cu));
        c.StoreWord(8, m, (c.S0 + 0x20u));
        c.StoreWord(9, m, (c.S0 + 0x24u));
        c.StoreWord(10, m, (c.S0 + 0x28u));
        c.StoreWord(11, m, (c.S0 + 0x2Cu));
        c.A2 = c.S0 + 0x24u;
        c.RA = 0x80035B70u;
        Dispatcher.Call(c, m, 0x80043408u);
        c.CopyRegister(2, 0);
        goto L80035CDC;
        L80035B78: ;
        c.A0 = c.S2 + 0x48u;
        c.RA = 0x80035B80u;
        Dispatcher.Call(c, m, 0x80040234u);
        c.LoadWord(2, m, c.S2);
        c.V0 = c.V0 | 0x0020u;
        c.StoreWord(2, m, c.S2);
        c.RA = 0x80035B94u;
        Dispatcher.Call(c, m, 0x8004410cu);
        c.LoadWord(5, m, (c.GP + 0x5F8u));
        c.CopyRegister(4, 2);
        c.A2 = 0x00000041u;
        c.S0 = c.S2 + 0x24u;
        c.CopyRegister(7, 16);
        c.RA = 0x80035BACu;
        Dispatcher.Call(c, m, 0x8004483cu);
        c.CopyRegister(4, 16);
        c.A1 = 0x08C00000u;
        c.A1 = c.A1 | 0xC000u;
        c.RA = 0x80035BBCu;
        Dispatcher.Call(c, m, 0x8003fea8u);
        c.LoadWord(5, m, c.S1);
        c.V1 = m.ReadU8((c.A1 + 0x4u));
        c.V0 = 0x00000002u;
        if (c.V1 != c.V0) {
            c.CopyRegister(2, 0);
            goto L80035CDC;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(19, 5);
        c.A0 = (uint)(short)m.ReadU16((c.S3 + 0x6u));
        if ((int)c.A0 >= 0) {
            c.A0 = ~(0u | c.A0);
            goto L80035BF0;
        }
        c.A0 = ~(0u | c.A0);
        c.A1 = 0x00000014u;
        c.RA = 0x80035BF0u;
        Dispatcher.Call(c, m, 0x80012050u);
        L80035BF0: ;
        c.RA = 0x80035BF8u;
        Dispatcher.Call(c, m, 0x80017160u);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 1;
        c.V1 = (uint)((int)c.V1 >> 15);
        if (c.V1 != 0u) {
            c.CopyRegister(2, 0);
            goto L80035CDC;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(17, 0);
        c.S2 = c.SP + 0x10u;
        c.CopyRegister(16, 19);
        L80035C1C: ;
        c.V0 = m.ReadU16((c.S0 + 0x11Cu));
        if (c.V0 == 0u) {
            goto L80035CB0;
        }
        c.RA = 0x80035C34u;
        Dispatcher.Call(c, m, 0x80017160u);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 6;
        c.V1 = c.V1 - c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 - c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 - c.V0;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.V1 - 0x5F5u;
        c.V0 = 0xFFFFEE1Fu;
        c.StoreWord(3, m, (c.SP + 0x10u));
        c.StoreWord(2, m, (c.S2 + 0x4u));
        c.RA = 0x80035C6Cu;
        Dispatcher.Call(c, m, 0x80017160u);
        c.A0 = 0x00080000u;
        c.A0 = c.A0 << (int)(c.S1 & 31u);
        c.A1 = c.S3 + 0x24u;
        c.CopyRegister(6, 18);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 6;
        c.V1 = c.V1 - c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 - c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 - c.V0;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.V1 - 0x5F5u;
        c.StoreWord(3, m, (c.S2 + 0x8u));
        c.RA = 0x80035CACu;
        Dispatcher.Call(c, m, 0x8003cee0u);
        m.WriteU16((c.S0 + 0x11Cu), (ushort)0u);
        L80035CB0: ;
        c.S1 = c.S1 + 0x1u;
        c.V0 = (int)c.S1 < 3 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.S0 = c.S0 + 0x2u;
            goto L80035C1C;
        }
        c.S0 = c.S0 + 0x2u;
        c.CopyRegister(2, 0);
        goto L80035CDC;
        L80035CC8: ;
        c.CopyRegister(4, 18);
        c.RA = 0x80035CD0u;
        Dispatcher.Call(c, m, 0x800205f8u);
        c.V0 = 0xFFFFFFFFu;
        goto L80035CDC;
        L80035CD8: ;
        c.CopyRegister(2, 0);
        L80035CDC: ;
        c.LoadWord(31, m, (c.SP + 0x30u));
        c.LoadWord(19, m, (c.SP + 0x2Cu));
        c.LoadWord(18, m, (c.SP + 0x28u));
        c.LoadWord(17, m, (c.SP + 0x24u));
        c.LoadWord(16, m, (c.SP + 0x20u));
        c.SP = c.SP + 0x38u;
        return;
    }
}
