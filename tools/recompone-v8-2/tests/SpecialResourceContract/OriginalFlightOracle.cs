// Generated original V8 instructions; MatrixNormal links to the shared native PSYQ service.
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
static class OriginalFlightOracle
{
    public static void FUN_80030c08(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        c.StoreWord(18, m, (c.SP + 0x18u));
        c.S2 = c.S0 + 0xA4u;
        c.A1 = c.S0 + 0x24u;
        c.CopyRegister(6, 0);
        c.CopyRegister(7, 6);
        c.StoreWord(31, m, (c.SP + 0x1Cu));
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.RA = 0x80030C34u;
        Dispatcher.Call(c, m, 0x8001d748u);
        c.LoadWord(3, m, (c.S0 + 0x28u));
        c.V0 = c.V0 - c.V1;
        if ((int)c.V0 >= 0) {
            c.A0 = c.S0 + 0x80u;
            goto L80030C4C;
        }
        c.A0 = c.S0 + 0x80u;
        c.V0 = c.V0 + 0xFFu;
        L80030C4C: ;
        c.S1 = (uint)((int)c.V0 >> 8);
        c.RA = 0x80030C54u;
        Dispatcher.Call(c, m, 0x80016a20u);
        c.CopyRegister(3, 2);
        if ((int)c.V1 >= 0) {
            goto L80030C64;
        }
        c.V1 = c.V1 + 0x7Fu;
        L80030C64: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0xA4u));
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x1Au));
        c.V1 = (uint)((int)c.V1 >> 7);
        c.StoreWord(3, m, (c.S0 + 0x8Cu));
        c.V0 = c.V0 << 6;
        c.StoreWord(2, m, (c.S0 + 0x94u));
        c.LoadWord(2, m, (c.S0 + 0x90u));
        if ((int)c.A0 > 0) {
            c.V1 = c.V0 + 0x200u;
            goto L80030C8C;
        }
        c.V1 = c.V0 + 0x200u;
        c.V1 = c.V0 - 0x200u;
        L80030C8C: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x16u));
        c.StoreWord(3, m, (c.S0 + 0x90u));
        c.LoadWord(3, m, (c.S0 + 0x98u));
        if ((int)c.V0 >= 0) {
            c.V0 = c.V1 - 0x200u;
            goto L80030CA4;
        }
        c.V0 = c.V1 - 0x200u;
        c.V0 = c.V1 + 0x200u;
        L80030CA4: ;
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x14u));
        c.StoreWord(2, m, (c.S0 + 0x98u));
        c.V0 = (uint)(short)m.ReadU16((c.S2 + 0x2u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.A2 = c.LO;
        if ((int)c.A2 >= 0) {
            goto L80030CC8;
        }
        c.A2 = c.A2 + 0xFu;
        L80030CC8: ;
        c.LoadWord(3, m, (c.S0 + 0x80u));
        c.LoadWord(5, m, (c.S0 + 0x84u));
        if ((int)c.S1 >= 0) {
            c.CopyRegister(2, 17);
            goto L80030CDC;
        }
        c.CopyRegister(2, 17);
        c.V0 = 0u - c.V0;
        L80030CDC: ;
        { var _r = (long)(int)c.S1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.LoadWord(4, m, (c.S0 + 0xD8u));
        c.A3 = 0x00000800u;
        c.V0 = (uint)((int)c.A2 >> 4);
        c.V1 = c.V1 + c.V0;
        c.V0 = c.A0 << 3;
        c.V0 = c.V0 - c.A0;
        c.StoreWord(3, m, (c.S0 + 0x80u));
        c.V1 = c.V0 << 10;
        c.T0 = c.LO;
        c.V0 = (int)c.A3 < (int)c.T0 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.A1 = c.A1 + 0x1C00u;
            goto L80030D14;
        }
        c.A1 = c.A1 + 0x1C00u;
        c.CopyRegister(7, 8);
        L80030D14: ;
        if (c.A3 != 0u) { if ((int)c.V1 == int.MinValue && (int)c.A3 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.V1 / (int)c.A3); c.HI = (uint)((int)c.V1 % (int)c.A3); } }
        c.V0 = c.LO;
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x20u));
        c.V0 = c.A1 - c.V0;
        c.StoreWord(2, m, (c.S0 + 0x84u));
        c.V0 = (uint)(short)m.ReadU16((c.S2 + 0x2u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        if ((int)c.V1 >= 0) {
            goto L80030D44;
        }
        c.V1 = c.V1 + 0xFu;
        L80030D44: ;
        c.LoadWord(2, m, (c.S0 + 0x88u));
        c.CopyRegister(4, 16);
        c.V1 = (uint)((int)c.V1 >> 4);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x88u));
        c.RA = 0x80030D5Cu;
        Dispatcher.Call(c, m, 0x80017324u);
        c.LoadWord(2, m, (c.S0 + 0x90u));
        c.A0 = 0x00000F80u;
        { var _r = (long)(int)c.V0 * (int)c.A0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.V0 = (uint)((int)c.V1 >> 12);
        c.V1 = c.S0 + 0x90u;
        c.StoreWord(2, m, (c.S0 + 0x90u));
        c.LoadWord(2, m, (c.V1 + 0x4u));
        { var _r = (long)(int)c.V0 * (int)c.A0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.LoadWord(2, m, (c.V1 + 0x8u));
        c.T0 = c.LO;
        { var _r = (long)(int)c.V0 * (int)c.A0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = (uint)((int)c.T0 >> 12);
        c.StoreWord(2, m, (c.V1 + 0x4u));
        c.A0 = c.LO;
        c.V0 = (uint)((int)c.A0 >> 12);
        c.StoreWord(2, m, (c.V1 + 0x8u));
        c.LoadWord(3, m, (c.S0 + 0x80u));
        if ((int)c.V1 >= 0) {
            c.CopyRegister(2, 3);
            goto L80030DC0;
        }
        c.CopyRegister(2, 3);
        c.V0 = c.V1 + 0x3Fu;
        L80030DC0: ;
        c.LoadWord(4, m, (c.S0 + 0x84u));
        c.V0 = (uint)((int)c.V0 >> 6);
        c.V0 = c.V1 - c.V0;
        c.StoreWord(2, m, (c.S0 + 0x80u));
        if ((int)c.A0 >= 0) {
            c.CopyRegister(2, 4);
            goto L80030DDC;
        }
        c.CopyRegister(2, 4);
        c.V0 = c.A0 + 0x3Fu;
        L80030DDC: ;
        c.LoadWord(3, m, (c.S0 + 0x88u));
        c.V0 = (uint)((int)c.V0 >> 6);
        c.V0 = c.A0 - c.V0;
        c.StoreWord(2, m, (c.S0 + 0x84u));
        if ((int)c.V1 >= 0) {
            c.CopyRegister(2, 3);
            goto L80030DF8;
        }
        c.CopyRegister(2, 3);
        c.V0 = c.V1 + 0x3Fu;
        L80030DF8: ;
        c.V0 = (uint)((int)c.V0 >> 6);
        c.V0 = c.V1 - c.V0;
        c.StoreWord(2, m, (c.S0 + 0x88u));
        c.CopyRegister(5, 0);
        c.CopyRegister(6, 16);
        L80030E0C: ;
        c.LoadWord(4, m, (c.A2 + 0x110u));
        if (c.A0 == 0u) {
            goto L80030E30;
        }
        c.V0 = (uint)(short)m.ReadU16((c.A0 + 0x6u));
        c.V1 = m.ReadU16((c.A0 + 0x6u));
        if (c.V0 == 0u) {
            c.V0 = c.V1 - 0x1u;
            goto L80030E30;
        }
        c.V0 = c.V1 - 0x1u;
        m.WriteU16((c.A0 + 0x6u), (ushort)c.V0);
        L80030E30: ;
        c.A1 = c.A1 + 0x1u;
        c.V0 = (int)c.A1 < 3 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.A2 = c.A2 + 0x4u;
            goto L80030E0C;
        }
        c.A2 = c.A2 + 0x4u;
        c.CopyRegister(5, 0);
        c.CopyRegister(3, 16);
        L80030E48: ;
        c.V0 = m.ReadU16((c.V1 + 0x11Cu));
        if (c.V0 == 0u) {
            c.V0 = c.V0 - 0x1u;
            goto L80030E5C;
        }
        c.V0 = c.V0 - 0x1u;
        m.WriteU16((c.V1 + 0x11Cu), (ushort)c.V0);
        L80030E5C: ;
        c.A1 = c.A1 + 0x1u;
        c.V0 = (int)c.A1 < 3 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.V1 = c.V1 + 0x2u;
            goto L80030E48;
        }
        c.V1 = c.V1 + 0x2u;
        c.LoadWord(2, m, c.S0);
        c.V1 = 0x00800000u;
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            goto L80030F1C;
        }
        c.V0 = m.ReadU16((c.S0 + 0x120u));
        if (c.V0 == 0u) {
            goto L80030F04;
        }
        c.LoadWord(2, m, (c.S0 + 0x24u));
        c.LoadWord(4, m, (c.S0 + 0x48u));
        c.V0 = c.V0 - c.A0;
        if ((int)c.V0 >= 0) {
            goto L80030EAC;
        }
        c.V0 = c.V0 + 0x1Fu;
        L80030EAC: ;
        c.LoadWord(3, m, (c.S0 + 0x28u));
        c.LoadWord(5, m, (c.S0 + 0x4Cu));
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.A0 + c.V0;
        c.StoreWord(2, m, (c.S0 + 0x48u));
        c.V0 = c.V1 - c.A1;
        if ((int)c.V0 >= 0) {
            goto L80030ED0;
        }
        c.V0 = c.V0 + 0x1Fu;
        L80030ED0: ;
        c.LoadWord(3, m, (c.S0 + 0x2Cu));
        c.LoadWord(4, m, (c.S0 + 0x50u));
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.A1 + c.V0;
        c.StoreWord(2, m, (c.S0 + 0x4Cu));
        c.V0 = c.V1 - c.A0;
        if ((int)c.V0 >= 0) {
            goto L80030EF4;
        }
        c.V0 = c.V0 + 0x1Fu;
        L80030EF4: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.A0 + c.V0;
        c.StoreWord(2, m, (c.S0 + 0x50u));
        goto L80030F1C;
        L80030F04: ;
        c.LoadWord(9, m, (c.S0 + 0x24u));
        c.LoadWord(10, m, (c.S0 + 0x28u));
        c.LoadWord(11, m, (c.S0 + 0x2Cu));
        c.StoreWord(9, m, (c.S0 + 0x48u));
        c.StoreWord(10, m, (c.S0 + 0x4Cu));
        c.StoreWord(11, m, (c.S0 + 0x50u));
        L80030F1C: ;
        c.LoadWord(31, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x20u;
        return;
    }
    public static void FUN_80017324(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        c.StoreWord(31, m, (c.SP + 0x18u));
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.LoadWord(5, m, (c.S0 + 0x90u));
        if ((int)c.A1 >= 0) {
            c.S1 = c.S0 + 0x10u;
            goto L8001734C;
        }
        c.S1 = c.S0 + 0x10u;
        c.A1 = c.A1 + 0x7Fu;
        L8001734C: ;
        c.LoadWord(6, m, (c.S0 + 0x94u));
        if ((int)c.A2 >= 0) {
            c.A1 = (uint)((int)c.A1 >> 7);
            goto L80017360;
        }
        c.A1 = (uint)((int)c.A1 >> 7);
        c.A2 = c.A2 + 0x7Fu;
        L80017360: ;
        c.LoadWord(7, m, (c.S0 + 0x98u));
        if ((int)c.A3 >= 0) {
            c.A2 = (uint)((int)c.A2 >> 7);
            goto L80017374;
        }
        c.A2 = (uint)((int)c.A2 >> 7);
        c.A3 = c.A3 + 0x7Fu;
        L80017374: ;
        c.CopyRegister(4, 17);
        c.A3 = (uint)((int)c.A3 >> 7);
        c.RA = 0x80017380u;
        Dispatcher.Call(c, m, 0x800439b8u);
        c.LoadWord(3, m, (c.S0 + 0x80u));
        if ((int)c.V1 >= 0) {
            goto L80017394;
        }
        c.V1 = c.V1 + 0x7Fu;
        L80017394: ;
        c.LoadWord(2, m, (c.S0 + 0x24u));
        c.LoadWord(4, m, (c.S0 + 0x84u));
        c.V1 = (uint)((int)c.V1 >> 7);
        c.V0 = c.V0 + c.V1;
        if ((int)c.A0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x24u));
            goto L800173B0;
        }
        c.StoreWord(2, m, (c.S0 + 0x24u));
        c.A0 = c.A0 + 0x7Fu;
        L800173B0: ;
        c.LoadWord(2, m, (c.S0 + 0x28u));
        c.LoadWord(6, m, (c.S0 + 0x88u));
        c.V1 = (uint)((int)c.A0 >> 7);
        c.V0 = c.V0 + c.V1;
        if ((int)c.A2 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x28u));
            goto L800173CC;
        }
        c.StoreWord(2, m, (c.S0 + 0x28u));
        c.A2 = c.A2 + 0x7Fu;
        L800173CC: ;
        c.LoadWord(3, m, (c.S0 + 0x2Cu));
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 4);
        c.V0 = (uint)((int)c.A2 >> 7);
        c.V1 = c.V1 + c.V0;
        c.StoreWord(3, m, (c.S0 + 0x2Cu));
        c.RA = 0x800173E8u;
        Dispatcher.Call(c, m, 0x80059A0Cu);
        c.LoadWord(31, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x20u;
        return;
    }
    public static void FUN_800439b8(CpuContext c, IMemory m)
    {
        c.LoadWord(8, m, c.A0);
        c.LoadWord(9, m, (c.A0 + 0x4u));
        c.LoadWord(10, m, (c.A0 + 0x8u));
        c.LoadWord(11, m, (c.A0 + 0xCu));
        c.LoadWord(12, m, (c.A0 + 0x10u));
        RecompOne.Runtime.Gte.WriteControl(0, c.T0);
        RecompOne.Runtime.Gte.WriteControl(1, c.T1);
        RecompOne.Runtime.Gte.WriteControl(2, c.T2);
        RecompOne.Runtime.Gte.WriteControl(3, c.T3);
        RecompOne.Runtime.Gte.WriteControl(4, c.T4);
        c.T4 = 0x00001000u;
        RecompOne.Runtime.Gte.Write(9, c.T4);
        RecompOne.Runtime.Gte.Write(10, c.A3);
        c.T0 = 0u - c.A2;
        RecompOne.Runtime.Gte.Write(11, c.T0);
        c.T0 = 0u - c.A3;
        RecompOne.Runtime.Gte.Execute(0x4A49E012u);
        c.LoadGteWord(9, 9);
        c.LoadGteWord(10, 10);
        c.LoadGteWord(11, 11);
        RecompOne.Runtime.Gte.Write(9, c.T0);
        RecompOne.Runtime.Gte.Write(10, c.T4);
        RecompOne.Runtime.Gte.Write(11, c.A1);
        RecompOne.Runtime.Gte.Execute(0x4A49E012u);
        m.WriteU16(c.A0, (ushort)c.T1);
        m.WriteU16((c.A0 + 0x6u), (ushort)c.T2);
        m.WriteU16((c.A0 + 0xCu), (ushort)c.T3);
        c.T0 = 0u - c.A1;
        c.LoadGteWord(9, 9);
        c.LoadGteWord(10, 10);
        c.LoadGteWord(11, 11);
        RecompOne.Runtime.Gte.Write(9, c.A2);
        RecompOne.Runtime.Gte.Write(10, c.T0);
        RecompOne.Runtime.Gte.Write(11, c.T4);
        RecompOne.Runtime.Gte.Execute(0x4A49E012u);
        m.WriteU16((c.A0 + 0x2u), (ushort)c.T1);
        m.WriteU16((c.A0 + 0x8u), (ushort)c.T2);
        m.WriteU16((c.A0 + 0xEu), (ushort)c.T3);
        c.LoadGteWord(9, 9);
        c.LoadGteWord(10, 10);
        c.LoadGteWord(11, 11);
        m.WriteU16((c.A0 + 0x4u), (ushort)c.T1);
        m.WriteU16((c.A0 + 0xAu), (ushort)c.T2);
        m.WriteU16((c.A0 + 0x10u), (ushort)c.T3);
        return;
    }
}
