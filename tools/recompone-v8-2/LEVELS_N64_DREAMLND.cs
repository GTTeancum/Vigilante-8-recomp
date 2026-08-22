using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace Recompiled;

public static partial class Vigilante82PC
{
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamContactActor(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x38u;
        c.StoreWord(17, m, (c.SP + 0x2Cu));
        c.CopyRegister(17, 4);
        c.StoreWord(31, m, (c.SP + 0x30u));
        if (c.A1 == 0u) {
            c.StoreWord(16, m, (c.SP + 0x28u));
            goto L801002EC;
        }
        c.StoreWord(16, m, (c.SP + 0x28u));
        c.V0 = 0x00000002u;
        if (c.A1 == c.V0) {
            goto L80100344;
        }
        goto L80100624;
        L801002EC: ;
        c.LoadWord(3, m, (c.S1 + 0x80u));
        if ((int)c.V1 < 0) {
            c.V1 = c.V1 + 0x7Fu;
            goto L801002F8;
        }
        L801002F8: ;
        c.V1 = (uint)((int)c.V1 >> 7);
        c.LoadWord(2, m, (c.S1 + 0x34u));
        c.LoadWord(5, m, (c.S1 + 0x84u));
        c.V0 = c.V0 + c.V1;
        if ((int)c.A1 >= 0) {
            c.StoreWord(2, m, (c.S1 + 0x34u));
            goto L80100314;
        }
        c.StoreWord(2, m, (c.S1 + 0x34u));
        c.A1 = c.A1 + 0x7Fu;
        L80100314: ;
        c.V1 = (uint)((int)c.A1 >> 7);
        c.LoadWord(2, m, (c.S1 + 0x38u));
        c.LoadWord(4, m, (c.S1 + 0x88u));
        c.V0 = c.V0 + c.V1;
        if ((int)c.A0 >= 0) {
            c.StoreWord(2, m, (c.S1 + 0x38u));
            goto L80100330;
        }
        c.StoreWord(2, m, (c.S1 + 0x38u));
        c.A0 = c.A0 + 0x7Fu;
        L80100330: ;
        c.LoadWord(2, m, (c.S1 + 0x3Cu));
        c.V1 = (uint)((int)c.A0 >> 7);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x3Cu));
        goto L80100624;
        L80100344: ;
        c.V0 = m.ReadU8((c.S1 + 0x8u));
        c.V1 = c.V0 + 0x1u;
        c.V0 = c.V0 << 24;
        m.WriteU8((c.S1 + 0x8u), (byte)c.V1);
        c.V1 = (uint)((int)c.V0 >> 24);
        c.V0 = 0x00000001u;
        if (c.V1 == c.V0) {
            c.V0 = (int)c.V1 < 2 ? 1u : 0u;
            goto L801004A4;
        }
        c.V0 = (int)c.V1 < 2 ? 1u : 0u;
        if (c.V0 == 0u) {
            goto L8010037C;
        }
        if (c.V1 == 0u) {
            goto L80100394;
        }
        goto L80100624;
        L8010037C: ;
        if (c.V1 == c.A1) {
            c.V0 = 0x00000003u;
            goto L801004E0;
        }
        c.V0 = 0x00000003u;
        if (c.V1 == c.V0) {
            goto L801005EC;
        }
        goto L80100624;
        L80100394: ;
        c.A0 = m.ReadU8((c.S1 + 0xDEu));
        c.RA = 0x801003A0u;
        Vigilante82PC.func_800318D0(c, m);
        c.CopyRegister(16, 2);
        c.CopyRegister(4, 16);
        c.RA = 0x801003ACu;
        Vigilante82PC.func_8004DCD8(c, m);
        if (c.V0 == 0u) {
            c.CopyRegister(4, 16);
            goto L801003BC;
        }
        c.CopyRegister(4, 16);
        c.A1 = 0x0000005Au;
        c.RA = 0x801003BCu;
        Vigilante82PC.func_80030CB0(c, m);
        L801003BC: ;
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(3, m, (c.S1 + 0x34u));
        c.V0 = c.V0 | 0x0002u;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(2, m, (c.S0 + 0x24u));
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 7;
        c.V0 = (uint)((int)c.V0 >> 7);
        c.StoreWord(2, m, (c.S1 + 0x80u));
        c.LoadWord(2, m, (c.S0 + 0x28u));
        c.LoadWord(3, m, (c.S1 + 0x38u));
        c.A1 = c.S1 + 0x80u;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 7;
        c.V0 = (uint)((int)c.V0 >> 7);
        c.StoreWord(2, m, (c.A1 + 0x4u));
        c.LoadWord(2, m, (c.S0 + 0x2Cu));
        c.LoadWord(3, m, (c.S1 + 0x3Cu));
        c.A0 = c.SP + 0x20u;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 7;
        c.V0 = (uint)((int)c.V0 >> 7);
        c.StoreWord(2, m, (c.A1 + 0x8u));
        c.V0 = 0xFFFFC000u;
        c.StoreWord(2, m, (c.S1 + 0x90u));
        c.V0 = c.S1 + 0x90u;
        c.StoreWord(0, m, (c.V0 + 0x4u));
        c.StoreWord(0, m, (c.V0 + 0x8u));
        c.V0 = 0x0000038Eu;
        m.WriteU16((c.SP + 0x20u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.S0 + 0x42u));
        c.A1 = c.S1 + 0x20u;
        m.WriteU16((c.A0 + 0x4u), (ushort)0u);
        m.WriteU16((c.A0 + 0x2u), (ushort)c.V0);
        c.RA = 0x80100448u;
        Vigilante82PC.func_8005A78C(c, m);
        c.CopyRegister(4, 17);
        c.A1 = 0x00000080u;
        c.RA = 0x80100454u;
        Vigilante82PC.func_80030CB0(c, m);
        c.LoadWord(3, m, (c.S1 + 0xECu));
        if (c.V1 == 0u) {
            goto L80100624;
        }
        c.LoadWord(2, m, (c.S1 + 0x80u));
        if ((int)c.V0 >= 0) {
            c.A0 = c.V1 + 0x84u;
            goto L80100470;
        }
        c.A0 = c.V1 + 0x84u;
        c.V0 = c.V0 + 0x7Fu;
        L80100470: ;
        c.V0 = (uint)((int)c.V0 >> 7);
        m.WriteU16((c.V1 + 0x84u), (ushort)c.V0);
        c.LoadWord(2, m, (c.S1 + 0x84u));
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x7Fu;
            goto L80100484;
        }
        L80100484: ;
        c.V0 = (uint)((int)c.V0 >> 7);
        m.WriteU16((c.A0 + 0x2u), (ushort)c.V0);
        c.LoadWord(2, m, (c.S1 + 0x88u));
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x7Fu;
            goto L80100498;
        }
        L80100498: ;
        c.V0 = (uint)((int)c.V0 >> 7);
        m.WriteU16((c.A0 + 0x4u), (ushort)c.V0);
        goto L80100624;
        L801004A4: ;
        c.V0 = c.S1 + 0x80u;
        c.StoreWord(0, m, (c.S1 + 0x80u));
        c.StoreWord(0, m, (c.V0 + 0x4u));
        c.StoreWord(0, m, (c.V0 + 0x8u));
        c.LoadWord(2, m, (c.S1 + 0xECu));
        if (c.V0 == 0u) {
            c.CopyRegister(4, 17);
            goto L801004D0;
        }
        c.CopyRegister(4, 17);
        m.WriteU16((c.V0 + 0x84u), (ushort)0u);
        c.V0 = c.V0 + 0x84u;
        m.WriteU16((c.V0 + 0x2u), (ushort)0u);
        m.WriteU16((c.V0 + 0x4u), (ushort)0u);
        L801004D0: ;
        c.A1 = 0x0000001Eu;
        c.RA = 0x801004D8u;
        Vigilante82PC.func_80030CB0(c, m);
        goto L80100624;
        L801004E0: ;
        c.S0 = c.S1 + 0x34u;
        c.CopyRegister(4, 16);
        c.A2 = 0x000001FDu;
        c.V0 = 0x0000005Fu;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.V0 = 0x00000004u;
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.V0 = 0x80070000u;
        c.LoadWord(3, m, 0x8006B814u);
        c.V0 = 0x0000003Cu;
        c.StoreWord(2, m, (c.SP + 0x18u));
        c.LoadWord(5, m, (c.V1 + 0x5Cu));
        c.A3 = 0x000001FCu;
        c.RA = 0x80100518u;
        Vigilante82PC.DreamlandContactEffect(c, m);
        c.RA = 0x80100520u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.V0 = 0x80070000u;
        c.LoadWord(2, m, 0x8006B41Cu);
        c.A2 = 0x00000063u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 16);
        c.RA = 0x8010053Cu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E580u);
        c.A1 = 0xFEFF0000u;
        c.A1 = c.A1 | 0xFFFDu;
        c.A0 = (uint)(short)m.ReadU16((c.S1 + 0x24u));
        c.LoadWord(3, m, (c.S1 + 0x4u));
        c.V0 = c.A0 << 3;
        c.V0 = c.V0 + c.A0;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.A0;
        c.V0 = c.V0 << 5;
        c.V0 = c.V0 + c.A0;
        c.V1 = c.V1 & c.A1;
        c.A0 = c.S1 + 0x80u;
        if ((int)c.V0 >= 0) {
            c.StoreWord(3, m, (c.S1 + 0x4u));
            goto L80100578;
        }
        c.StoreWord(3, m, (c.S1 + 0x4u));
        c.V0 = c.V0 + 0x1Fu;
        L80100578: ;
        c.V1 = (uint)(short)m.ReadU16((c.S1 + 0x2Au));
        c.V0 = (uint)((int)c.V0 >> 5);
        c.StoreWord(2, m, (c.S1 + 0x80u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 5;
        c.V0 = c.V0 + c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L801005A4;
        }
        L801005A4: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.StoreWord(2, m, (c.A0 + 0x4u));
        c.V1 = (uint)(short)m.ReadU16((c.S1 + 0x30u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 5;
        c.V0 = c.V0 + c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L801005D0;
        }
        L801005D0: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.StoreWord(2, m, (c.A0 + 0x8u));
        c.CopyRegister(4, 17);
        c.A1 = 0x0000002Du;
        c.RA = 0x801005E4u;
        Vigilante82PC.func_80030CB0(c, m);
        goto L80100624;
        L801005EC: ;
        c.LoadWord(4, m, (c.S1 + 0xECu));
        if (c.A0 == 0u) {
            c.V1 = 0xFFFB0000u;
            goto L80100608;
        }
        c.V1 = 0xFFFB0000u;
        c.LoadWord(2, m, c.A0);
        c.V1 = c.V1 | 0xFFFFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.A0);
        L80100608: ;
        c.CopyRegister(4, 17);
        c.LoadWord(2, m, (c.A0 + 0x4u));
        c.V1 = 0xFFFFFFDFu;
        m.WriteU8((c.A0 + 0x8u), (byte)0u);
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.A0 + 0x4u));
        c.RA = 0x80100624u;
        Vigilante82PC.func_80041FEC(c, m);
        L80100624: ;
        c.LoadWord(31, m, (c.SP + 0x30u));
        c.LoadWord(17, m, (c.SP + 0x2Cu));
        c.LoadWord(16, m, (c.SP + 0x28u));
        c.CopyRegister(2, 0);
        c.SP = c.SP + 0x38u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamLnd(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x38u;
        c.StoreWord(16, m, (c.SP + 0x20u));
        c.CopyRegister(16, 4);
        c.V0 = c.A1 < 0x00000013u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x30u));
        c.StoreWord(19, m, (c.SP + 0x2Cu));
        c.StoreWord(18, m, (c.SP + 0x28u));
        if (c.V0 == 0u) {
            c.StoreWord(17, m, (c.SP + 0x24u));
            goto L801009C8;
        }
        c.StoreWord(17, m, (c.SP + 0x24u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0xA8u;
        if (c.A1 == 0x00000010u) goto L801009C8;
        c.V1 = c.A1 < 0x00000011u ? c.A1 : c.A1 - 0x1u;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x8010067Cu: goto L8010067C;
            case 0x80100984u: goto L80100984;
            case 0x801009B4u: goto L801009B4;
            case 0x801009C8u: goto L801009C8;
            case 0x80100974u: goto L80100974;
            case 0x801006ECu: goto L801006EC;
            case 0x801006BCu: goto L801006BC;
            case 0x80100960u: goto L80100960;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L8010067C: ;
        if (c.A2 == 0u) {
            c.CopyRegister(4, 16);
            goto L801009C8;
        }
        c.CopyRegister(4, 16);
        c.LoadWord(2, m, (c.S0 + 0x80u));
        c.V0 = c.V0 - c.A2;
        if ((int)c.V0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x80u));
            goto L801009C8;
        }
        c.StoreWord(2, m, (c.S0 + 0x80u));
        L80100694: ;
        c.LoadWord(2, m, (c.A0 + 0x80u));
        c.LoadWord(3, m, (c.A0 + 0x84u));
        c.V0 = c.V0 + 0x6u;
        c.V1 = c.V1 + 0x1u;
        c.V1 = c.V1 & 0x0007u;
        c.StoreWord(2, m, (c.A0 + 0x80u));
        if ((int)c.V0 < 0) {
            c.StoreWord(3, m, (c.A0 + 0x84u));
            goto L80100694;
        }
        c.StoreWord(3, m, (c.A0 + 0x84u));
        c.CopyRegister(2, 0);
        goto L801009CC;
        L801006BC: ;
        c.LoadWord(4, m, (c.S0 + 0x84u));
        c.V0 = (int)c.A0 < 5 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.V0 = 0x0000002Bu;
            goto L801006D4;
        }
        c.V0 = 0x0000002Bu;
        c.A0 = c.A0 + 0x23u;
        goto L801006D8;
        L801006D4: ;
        c.A0 = c.V0 - c.A0;
        L801006D8: ;
        c.A1 = 0x00000010u;
        c.A2 = 0x000000A0u;
        c.RA = 0x801006E4u;
        RecompOne.Runtime.Sdk.V82NativeWaterDiagnostics.Frame(c, m);
        Vigilante82PC.func_80017EB8(c, m);
        c.CopyRegister(2, 0);
        goto L801009CC;
        L801006EC: ;
        c.V1 = m.ReadU8((c.S0 + 0x8u));
        c.V0 = 0x00000008u;
        if (c.V1 != c.V0) {
            c.V0 = 0x00000002u;
            goto L80100760;
        }
        c.V0 = 0x00000002u;
        c.CopyRegister(4, 16);
        c.RA = 0x80100704u;
        Vigilante82PC.func_8002CD78(c, m);
        c.CopyRegister(16, 2);
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0xAu));
        if ((int)c.A0 >= 0) {
            c.V1 = 0x55EB0000u;
            goto L80100758;
        }
        c.V1 = 0x55EB0000u;
        c.LoadWord(2, m, (c.S0 + 0x8Cu));
        c.V1 = c.V1 | 0xB1CDu;
        c.V0 = c.V0 << 5;
        { var _r = (long)(int)c.V0 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.A1 = 0x000000FFu;
        c.V0 = (uint)((int)c.V0 >> 31);
        c.T0 = c.HI;
        c.V1 = (uint)((int)c.T0 >> 10);
        c.V1 = c.V1 - c.V0;
        c.V0 = (int)c.V1 < (int)c.A1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.A0 = ~(0u | c.A0);
            goto L80100748;
        }
        c.A0 = ~(0u | c.A0);
        c.CopyRegister(5, 3);
        L80100748: ;
        c.A1 = c.A1 & 0x00FFu;
        c.A2 = 0x00000004u;
        c.A3 = 0x00000008u;
        c.RA = 0x80100758u;
        Vigilante82PC.func_80015B00(c, m);
        L80100758: ;
        c.V1 = m.ReadU8((c.S0 + 0x8u));
        c.V0 = 0x00000002u;
        L80100760: ;
        if (c.V1 != c.V0) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.V1 = 0x02000000u;
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V0 = m.ReadU16((c.S0 + 0x1Cu));
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x34u));
        c.V0 = (int)c.V1 < 874 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V0 = (int)c.V1 < 911 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x3Cu));
        c.V0 = (int)c.V1 < 1326 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V0 = (int)c.V1 < 1364 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(4, 16);
            goto L801009C8;
        }
        c.CopyRegister(4, 16);
        c.A1 = 0xFFFFFF6Au;
        c.A2 = 0x80100000u;
        c.A2 = c.A2 + 0x9Cu;
        c.CopyRegister(17, 16);
        c.LoadWord(18, m, (c.S1 + 0xECu));
        c.CopyRegister(7, 0);
        c.RA = 0x801007DCu;
        Vigilante82PC.func_8003A020(c, m);
        c.A0 = c.S1 + 0x34u;
        c.A2 = 0x000001FDu;
        c.V0 = 0x0000005Fu;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.V0 = 0x00000004u;
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.V0 = 0x80060000u;
        c.LoadWord(3, m, 0x8006B814u);
        c.V0 = 0x0000003Cu;
        c.StoreWord(2, m, (c.SP + 0x18u));
        c.LoadWord(5, m, (c.V1 + 0x5Cu));
        c.A3 = 0x000001FCu;
        c.RA = 0x80100810u;
        Vigilante82PC.DreamlandContactEffect(c, m);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x9u));
        c.CopyRegister(5, 0);
        c.RA = 0x8010081Cu;
        Vigilante82PC.func_8001E2C8(c, m);
        c.V0 = 0x80060000u;
        c.S3 = 0x8006B830u;
        c.CopyRegister(4, 19);
        c.A1 = 0x00000021u;
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x2C0u;
        c.StoreWord(2, m, (c.S1 + 0x0u));
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.V1 = 0x03000000u;
        c.V0 = c.V0 | c.V1;
        c.V1 = 0xFFFFFFFDu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.RA = 0x80100854u;
        Vigilante82PC.func_80030250(c, m);
        c.CopyRegister(4, 19);
        c.A1 = 0x00000022u;
        c.S0 = 0u < c.V0 ? 1u : 0u;
        c.RA = 0x80100864u;
        Vigilante82PC.func_80030250(c, m);
        if (c.V0 != 0u) {
            c.S0 = c.S0 | 0x0002u;
            goto L8010086C;
        }
        L8010086C: ;
        c.CopyRegister(4, 19);
        c.A1 = 0x00000023u;
        c.RA = 0x80100878u;
        Vigilante82PC.func_80030250(c, m);
        if (c.V0 != 0u) {
            c.S0 = c.S0 | 0x0004u;
            goto L80100880;
        }
        L80100880: ;
        if (c.S0 == 0u) {
            c.S0 = 0x00000007u;
            goto L80100888;
        }
        L80100888: ;
        c.RA = 0x80100890u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V0 = (uint)((int)c.S0 >> (int)(c.V1 & 31u));
        c.V0 = c.V0 & 0x0001u;
        if (c.V0 == 0u) {
            c.V0 = 0x00020000u;
            goto L80100888;
        }
        c.V0 = 0x00020000u;
        c.V0 = c.V0 | 0xFA80u;
        c.CopyRegister(4, 17);
        c.A1 = 0x00000078u;
        c.V1 = c.V1 + 0x21u;
        m.WriteU8((c.A0 + 0xDEu), (byte)c.V1);
        c.V1 = c.A0 + 0x80u;
        m.WriteU8((c.A0 + 0x8u), (byte)0u);
        c.StoreWord(0, m, (c.A0 + 0x80u));
        c.StoreWord(2, m, (c.V1 + 0x4u));
        c.StoreWord(0, m, (c.V1 + 0x8u));
        c.RA = 0x801008D8u;
        Vigilante82PC.func_80030CB0(c, m);
        if (c.S2 == 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.A0 = (uint)(short)m.ReadU16((c.S2 + 0x14u));
        c.A1 = c.S2 + 0x84u;
        c.A0 = 0u - c.A0;
        c.V1 = c.A0 << 1;
        c.V1 = c.V1 + c.A0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 2;
        c.A0 = c.V0 + c.A0;
        c.LoadWord(2, m, c.S2);
        c.V1 = 0x000C0000u;
        c.V0 = c.V0 | c.V1;
        if ((int)c.A0 >= 0) {
            c.StoreWord(2, m, c.S2);
            goto L8010091C;
        }
        c.StoreWord(2, m, c.S2);
        c.A0 = c.A0 + 0xFFFu;
        L8010091C: ;
        c.V0 = (uint)((int)c.A0 >> 12);
        m.WriteU16((c.S2 + 0x84u), (ushort)c.V0);
        c.V0 = 0xFFFFFA0Bu;
        m.WriteU16((c.A1 + 0x2u), (ushort)c.V0);
        c.A0 = (uint)(short)m.ReadU16((c.S2 + 0x20u));
        c.A0 = 0u - c.A0;
        c.V1 = c.A0 << 1;
        c.V1 = c.V1 + c.A0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 2;
        c.V0 = c.V0 + c.A0;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L80100954;
        }
        L80100954: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        m.WriteU16((c.A1 + 0x4u), (ushort)c.V0);
        goto L801009C8;
        L80100960: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 6);
        c.RA = 0x8010096Cu;
        Vigilante82PC.func_800327CC(c, m);
        c.CopyRegister(2, 0);
        goto L801009CC;
        L80100974: ;
        c.A0 = 0x0000008Cu;
        c.RA = 0x8010097Cu;
        Vigilante82PC.func_8002CBE8(c, m);
        goto L801009CC;
        L80100984: ;
        // Native V8:2 global-water setup translated from the authored V8 plane.
        c.StoreWord(6, m, (c.SP + 0x34u));
        c.A0 = 0x0002A400u;
        c.A1 = 0x7FFF0000u;
        RecompOne.Runtime.Sdk.V82NativeWaterDiagnostics.Initialize(c, m, c.A0, c.A1);
        Vigilante82PC.func_80017F34(c, m);
        c.LoadWord(6, m, (c.SP + 0x34u));
        c.CopyRegister(4, 16);
        c.A0 = 0x80060000u;
        c.A0 = 0x8006B868u;
        c.A1 = 0x00000100u;
        c.V0 = 0x00000080u;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.RA = 0x8010099Cu;
        Vigilante82PC.func_80030250(c, m);
        c.A0 = 0x7F000000u;
        c.CopyRegister(5, 2);
        c.RA = 0x801009A8u;
        Vigilante82PC.func_8004AC1C(c, m);
        c.V1 = 0x80060000u;
        c.V0 = 0u < c.V0 ? 1u : 0u;
        c.StoreWord(2, m, 0x8006B828u);
        L801009B4: ;
        c.RA = 0x801009BCu;
        Vigilante82PC.func_80034B34(c, m);
        c.CopyRegister(4, 16);
        c.A1 = 0x000000F0u;
        c.RA = 0x801009C8u;
        Vigilante82PC.func_80030CB0(c, m);
        L801009C8: ;
        c.CopyRegister(2, 0);
        L801009CC: ;
        c.LoadWord(31, m, (c.SP + 0x30u));
        c.LoadWord(19, m, (c.SP + 0x2Cu));
        c.LoadWord(18, m, (c.SP + 0x28u));
        c.LoadWord(17, m, (c.SP + 0x24u));
        c.LoadWord(16, m, (c.SP + 0x20u));
        c.SP = c.SP + 0x38u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Butterfly(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x18u;
        c.StoreWord(31, m, (c.SP + 0x10u));
        c.V0 = 0x00000001u;
        if (c.A1 == c.V0) {
            c.CopyRegister(3, 4);
            goto L80100A24;
        }
        c.CopyRegister(3, 4);
        c.V0 = 0x00000007u;
        if (c.A1 != c.V0) {
            c.CopyRegister(2, 0);
            goto L80100A30;
        }
        c.CopyRegister(2, 0);
        c.V0 = 0x80060000u;
        c.LoadWord(2, m, 0x8006B814u);
        c.A0 = 0x00000080u;
        c.StoreWord(3, m, (c.V0 + 0x5Cu));
        c.RA = 0x80100A1Cu;
        Vigilante82PC.func_8002CBE8(c, m);
        goto L80100A30;
        L80100A24: ;
        c.CopyRegister(4, 3);
        c.RA = 0x80100A2Cu;
        Vigilante82PC.func_800520D8(c, m);
        c.V0 = 0xFFFFFFFFu;
        L80100A30: ;
        c.LoadWord(31, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x18u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void cobblebridge(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x18u));
        c.CopyRegister(16, 4);
        c.V0 = 0x00000003u;
        if (c.A1 == c.V0) {
            c.StoreWord(31, m, (c.SP + 0x1Cu));
            goto L80100A68;
        }
        c.StoreWord(31, m, (c.SP + 0x1Cu));
        c.V0 = 0x00000008u;
        if (c.A1 != c.V0) {
            c.CopyRegister(4, 16);
            goto L80100B70;
        }
        c.CopyRegister(4, 16);
        goto L80100A84;
        L80100A68: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 6);
        c.RA = 0x80100A74u;
        Vigilante82PC.func_80032CF0(c, m);
        if (c.V0 != 0u) {
            c.V0 = 0x00000001u;
            goto L80100A94;
        }
        c.V0 = 0x00000001u;
        goto L80100B70;
        L80100A84: ;
        c.CopyRegister(5, 6);
        c.RA = 0x80100A8Cu;
        Vigilante82PC.func_80032B90(c, m);
        if (c.V0 == 0u) {
            c.V0 = 0x00000001u;
            goto L80100B70;
        }
        c.V0 = 0x00000001u;
        L80100A94: ;
        m.WriteU16((c.SP + 0x16u), (ushort)c.V0);
        m.WriteU16((c.SP + 0x14u), (ushort)c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x24u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x34u));
        c.V0 = c.V0 << 3;
        c.V1 = c.V1 + c.V0;
        if ((int)c.V1 >= 0) {
            c.V0 = (uint)((int)c.V1 >> 16);
            goto L80100AC8;
        }
        c.V0 = (uint)((int)c.V1 >> 16);
        c.V0 = 0u | 0xFFFFu;
        c.V1 = c.V1 + c.V0;
        c.V0 = (uint)((int)c.V1 >> 16);
        L80100AC8: ;
        m.WriteU16((c.SP + 0x10u), (ushort)c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x30u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x3Cu));
        c.V0 = c.V0 << 3;
        c.V1 = c.V1 + c.V0;
        if ((int)c.V1 >= 0) {
            c.A0 = c.SP + 0x10u;
            goto L80100AF4;
        }
        c.A0 = c.SP + 0x10u;
        c.V0 = 0u | 0xFFFFu;
        c.V1 = c.V1 + c.V0;
        L80100AF4: ;
        c.CopyRegister(5, 0);
        c.V0 = (uint)((int)c.V1 >> 16);
        m.WriteU16((c.SP + 0x12u), (ushort)c.V0);
        c.RA = 0x80100B04u;
        Vigilante82PC.func_800359CC(c, m);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x24u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x34u));
        c.V0 = c.V0 << 3;
        c.V1 = c.V1 - c.V0;
        if ((int)c.V1 >= 0) {
            c.V0 = (uint)((int)c.V1 >> 16);
            goto L80100B30;
        }
        c.V0 = (uint)((int)c.V1 >> 16);
        c.V0 = 0u | 0xFFFFu;
        c.V1 = c.V1 + c.V0;
        c.V0 = (uint)((int)c.V1 >> 16);
        L80100B30: ;
        m.WriteU16((c.SP + 0x10u), (ushort)c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x30u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x3Cu));
        c.V0 = c.V0 << 3;
        c.V1 = c.V1 - c.V0;
        if ((int)c.V1 >= 0) {
            c.V0 = (uint)((int)c.V1 >> 16);
            goto L80100B60;
        }
        c.V0 = (uint)((int)c.V1 >> 16);
        c.V0 = 0u | 0xFFFFu;
        c.V1 = c.V1 + c.V0;
        c.V0 = (uint)((int)c.V1 >> 16);
        L80100B60: ;
        m.WriteU16((c.SP + 0x12u), (ushort)c.V0);
        c.A0 = c.SP + 0x10u;
        c.CopyRegister(5, 0);
        c.RA = 0x80100B70u;
        Vigilante82PC.func_800359CC(c, m);
        L80100B70: ;
        c.LoadWord(31, m, (c.SP + 0x1Cu));
        c.LoadWord(16, m, (c.SP + 0x18u));
        c.CopyRegister(2, 0);
        c.SP = c.SP + 0x20u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DBridge(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        c.V0 = c.A1 < 0x00000009u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x18u));
        if (c.V0 == 0u) {
            c.StoreWord(17, m, (c.SP + 0x14u));
            goto L80100C94;
        }
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x118u;
        c.V1 = c.A1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80100BBCu: goto L80100BBC;
            case 0x80100C14u: goto L80100C14;
            case 0x80100C28u: goto L80100C28;
            case 0x80100C64u: goto L80100C64;
            case 0x80100C94u: goto L80100C94;
            case 0x80100C78u: goto L80100C78;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L80100BBC: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.LoadWord(4, m, (c.S0 + 0x10u));
        if (c.V0 == 0u) {
            goto L80100BE4;
        }
        c.V1 = m.ReadU16((c.A0 + 0x44u));
        c.V1 = c.V1 - 0x4u;
        c.V0 = c.V1 << 16;
        c.V0 = (uint)((int)c.V0 >> 16);
        c.S1 = (int)c.V0 < -1023 ? 1u : 0u;
        goto L80100BFC;
        L80100BE4: ;
        c.V1 = m.ReadU16((c.A0 + 0x44u));
        c.V1 = c.V1 + 0x4u;
        c.V0 = c.V1 << 16;
        c.V0 = (uint)((int)c.V0 >> 16);
        c.V0 = ~(0u | c.V0);
        c.S1 = c.V0 >> 31;
        L80100BFC: ;
        m.WriteU16((c.A0 + 0x44u), (ushort)c.V1);
        c.RA = 0x80100C04u;
        Vigilante82PC.func_8002CF44(c, m);
        if (c.S1 == 0u) {
            goto L80100C94;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80100C14u;
        Vigilante82PC.func_80030BA8(c, m);
        L80100C14: ;
        c.CopyRegister(4, 16);
        c.A1 = 0x00000258u;
        c.RA = 0x80100C20u;
        Vigilante82PC.func_80030CB0(c, m);
        goto L80100C94;
        L80100C28: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.CopyRegister(4, 16);
        c.V0 = c.V0 < 0x00000001u ? 1u : 0u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V0);
        c.RA = 0x80100C3Cu;
        Vigilante82PC.func_80030B78(c, m);
        c.RA = 0x80100C44u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.LoadWord(3, m, (c.S0 + 0x5Cu));
        c.A2 = 0x00000046u;
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.A3 = c.S0 + 0x4Cu;
        c.RA = 0x80100C5Cu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        goto L80100C94;
        L80100C64: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 6);
        c.RA = 0x80100C70u;
        Vigilante82PC.func_80032CF0(c, m);
        goto L80100C84;
        L80100C78: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 6);
        c.RA = 0x80100C84u;
        Vigilante82PC.func_80032B90(c, m);
        L80100C84: ;
        if (c.V0 == 0u) {
            goto L80100C94;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80100C94u;
        Vigilante82PC.func_80030C68(c, m);
        L80100C94: ;
        c.LoadWord(31, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(2, 0);
        c.SP = c.SP + 0x20u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Rainbow(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(18, m, (c.SP + 0x18u));
        c.CopyRegister(18, 4);
        c.V0 = 0x00000001u;
        c.StoreWord(31, m, (c.SP + 0x1Cu));
        c.StoreWord(17, m, (c.SP + 0x14u));
        if (c.A1 == c.V0) {
            c.StoreWord(16, m, (c.SP + 0x10u));
            goto L80100D20;
        }
        c.StoreWord(16, m, (c.SP + 0x10u));
        if (c.A1 == 0u) {
            c.V1 = 0x00000002u;
            goto L80100CEC;
        }
        c.V1 = 0x00000002u;
        if (c.A1 == c.V1) {
            c.V0 = 0x00000003u;
            goto L80100D48;
        }
        c.V0 = 0x00000003u;
        if (c.A1 == c.V0) {
            goto L80100DA8;
        }
        goto L80100E00;
        L80100CEC: ;
        c.LoadWord(2, m, (c.S2 + 0x3Cu));
        c.A1 = c.V0 - 0x11E1u;
        c.V0 = 0x05000000u;
        c.V0 = (int)c.V0 < (int)c.A1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.StoreWord(5, m, (c.S2 + 0x3Cu));
            goto L80100D18;
        }
        c.StoreWord(5, m, (c.S2 + 0x3Cu));
        c.LoadWord(4, m, (c.S2 + 0x34u));
        c.RA = 0x80100D10u;
        Vigilante82PC.func_8001B750(c, m);
        c.StoreWord(2, m, (c.S2 + 0x38u));
        goto L80100E00;
        L80100D18: ;
        c.CopyRegister(4, 18);
        c.RA = 0x80100D20u;
        Vigilante82PC.func_80030BA8(c, m);
        L80100D20: ;
        c.CopyRegister(4, 18);
        c.A1 = 0x00000E10u;
        c.LoadWord(2, m, (c.A0 + 0x4u));
        c.V1 = 0x00000003u;
        m.WriteU8((c.A0 + 0x8u), (byte)c.V1);
        c.V0 = c.V0 | 0x0122u;
        c.StoreWord(2, m, (c.A0 + 0x4u));
        c.RA = 0x80100D40u;
        Vigilante82PC.func_80030CB0(c, m);
        goto L80100E00;
        L80100D48: ;
        c.RA = 0x80100D50u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V1 = c.V0 << 3;
        c.V1 = c.V1 - c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 1;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.A1 = c.V1 + 0x35Bu;
        c.V0 = (int)c.A1 < 884 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.A1 = c.V1 + 0x374u;
            goto L80100D78;
        }
        L80100D78: ;
        c.CopyRegister(4, 18);
        c.V0 = c.A1 << 16;
        c.StoreWord(2, m, (c.A0 + 0x34u));
        c.V0 = 0x05800000u;
        c.StoreWord(2, m, (c.A0 + 0x3Cu));
        c.LoadWord(2, m, (c.A0 + 0x4u));
        c.V1 = 0xFFFFFFDDu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.A0 + 0x4u));
        c.RA = 0x80100DA0u;
        Vigilante82PC.func_80030B78(c, m);
        goto L80100E00;
        L80100DA8: ;
        c.LoadWord(16, m, c.A2);
        c.V0 = m.ReadU8((c.S0 + 0x4u));
        if (c.V0 != c.V1) {
            c.CopyRegister(4, 16);
            goto L80100E00;
        }
        c.CopyRegister(4, 16);
        c.A1 = 0x000001F4u;
        c.RA = 0x80100DC0u;
        Vigilante82PC.DreamlandVehicleHazardDamage(c, m);
        c.S1 = c.S0 + 0x24u;
        c.RA = 0x80100DC8u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.V0 = 0x80060000u;
        c.A2 = 0x0000008Eu;
        c.LoadWord(5, m, 0x8006B41Cu);
        c.CopyRegister(7, 17);
        c.RA = 0x80100DE0u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x6u));
        if ((int)c.V0 >= 0) {
            c.CopyRegister(4, 17);
            goto L80100D18;
        }
        c.CopyRegister(4, 17);
        c.A1 = 0x00800000u;
        c.A1 = c.A1 | 0x0008u;
        c.RA = 0x80100DF8u;
        Vigilante82PC.func_8004E414(c, m);
        goto L80100D18;
        L80100E00: ;
        c.LoadWord(31, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(2, 0);
        c.SP = c.SP + 0x20u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void well(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x38u;
        c.StoreWord(16, m, (c.SP + 0x20u));
        c.CopyRegister(16, 4);
        c.V0 = 0x00000003u;
        c.StoreWord(31, m, (c.SP + 0x30u));
        c.StoreWord(19, m, (c.SP + 0x2Cu));
        c.StoreWord(18, m, (c.SP + 0x28u));
        if (c.A1 == c.V0) {
            c.StoreWord(17, m, (c.SP + 0x24u));
            goto L80100E70;
        }
        c.StoreWord(17, m, (c.SP + 0x24u));
        c.V0 = c.A1 < 0x00000004u ? 1u : 0u;
        if (c.V0 == 0u) {
            c.V0 = 0x00000002u;
            goto L80100E5C;
        }
        c.V0 = 0x00000002u;
        if (c.A1 == c.V0) {
            goto L80101080;
        }
        goto L80101088;
        L80100E5C: ;
        c.V0 = 0x00000008u;
        if (c.A1 == c.V0) {
            c.CopyRegister(4, 16);
            goto L80101070;
        }
        c.CopyRegister(4, 16);
        goto L80101088;
        L80100E70: ;
        c.LoadWord(17, m, c.A2);
        c.V1 = m.ReadU8((c.S1 + 0x4u));
        c.V0 = 0x00000002u;
        if (c.V1 != c.V0) {
            c.CopyRegister(4, 16);
            goto L80101060;
        }
        c.CopyRegister(4, 16);
        c.LoadWord(2, m, c.S1);
        c.V1 = 0x02000000u;
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            goto L80101060;
        }
        c.V0 = m.ReadU16((c.S1 + 0xCu));
        if (c.V0 == 0u) {
            c.V0 = 0x00000001u;
            goto L80101060;
        }
        c.V0 = 0x00000001u;
        c.LoadWord(3, m, (c.A2 + 0x4u));
        c.S2 = m.ReadU16(c.V1);
        if (c.S2 != c.V0) {
            goto L80101060;
        }
        c.V0 = m.ReadU16((c.V1 + 0x2u));
        if (c.V0 == 0u) {
            c.A0 = c.S0 + 0x34u;
            goto L8010105C;
        }
        c.A0 = c.S0 + 0x34u;
        c.V0 = 0x0000005Fu;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.V0 = 0x00000004u;
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.V0 = 0x80060000u;
        c.LoadWord(3, m, 0x8006B814u);
        c.V0 = 0x0000003Cu;
        c.A2 = 0x000001FDu;
        c.StoreWord(2, m, (c.SP + 0x18u));
        c.LoadWord(5, m, (c.V1 + 0x5Cu));
        c.A3 = 0x000001FCu;
        c.RA = 0x80100EF0u;
        Vigilante82PC.DreamlandContactEffect(c, m);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x5u));
        c.CopyRegister(5, 0);
        c.RA = 0x80100EFCu;
        Vigilante82PC.func_8001E2C8(c, m);
        c.V0 = 0x80060000u;
        c.S3 = 0x8006B830u;
        c.CopyRegister(4, 19);
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x2C0u;
        c.StoreWord(2, m, c.S1);
        c.LoadWord(2, m, c.S1);
        c.V1 = 0x03000000u;
        c.V0 = c.V0 | c.V1;
        c.V1 = 0xFFFFFFFDu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S1);
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0xAu));
        c.A1 = 0x00000021u;
        c.V0 = c.V0 - 0x21u;
        c.V0 = c.S2 << (int)(c.V0 & 31u);
        c.V0 = ~(0u | c.V0);
        c.S2 = c.V0 & 0x0007u;
        c.RA = 0x80100F48u;
        Vigilante82PC.func_80030250(c, m);
        c.CopyRegister(4, 19);
        c.A1 = 0x00000022u;
        c.S0 = 0u < c.V0 ? 1u : 0u;
        c.RA = 0x80100F58u;
        Vigilante82PC.func_80030250(c, m);
        if (c.V0 != 0u) {
            c.S0 = c.S0 | 0x0002u;
            goto L80100F60;
        }
        L80100F60: ;
        c.CopyRegister(4, 19);
        c.A1 = 0x00000023u;
        c.RA = 0x80100F6Cu;
        Vigilante82PC.func_80030250(c, m);
        if (c.V0 != 0u) {
            c.S0 = c.S0 | 0x0004u;
            goto L80100F74;
        }
        L80100F74: ;
        c.V1 = c.S2 & c.S0;
        if (c.V1 == 0u) {
            c.CopyRegister(3, 18);
            goto L80100F80;
        }
        L80100F80: ;
        c.CopyRegister(18, 3);
        L80100F84: ;
        c.RA = 0x80100F8Cu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.S0 = (uint)((int)c.V1 >> 15);
        c.V0 = (uint)((int)c.S2 >> (int)(c.S0 & 31u));
        c.V0 = c.V0 & 0x0001u;
        if (c.V0 == 0u) {
            c.V0 = 0x00020000u;
            goto L80100F84;
        }
        c.V0 = 0x00020000u;
        c.V0 = c.V0 | 0xFA80u;
        c.CopyRegister(4, 17);
        c.A1 = 0x00000078u;
        c.V1 = c.S0 + 0x31u;
        m.WriteU8((c.S1 + 0xD2u), (byte)c.V1);
        c.V1 = c.S1 + 0x80u;
        m.WriteU8((c.S1 + 0x8u), (byte)0u);
        c.StoreWord(0, m, (c.S1 + 0x80u));
        c.StoreWord(2, m, (c.V1 + 0x4u));
        c.StoreWord(0, m, (c.V1 + 0x8u));
        c.RA = 0x80100FD4u;
        Vigilante82PC.func_80030CB0(c, m);
        c.LoadWord(5, m, (c.S1 + 0xE0u));
        if (c.A1 == 0u) {
            c.A2 = c.A1 + 0x84u;
            goto L80101088;
        }
        c.A2 = c.A1 + 0x84u;
        c.A0 = (uint)(short)m.ReadU16((c.A1 + 0x14u));
        c.A0 = 0u - c.A0;
        c.V1 = c.A0 << 1;
        c.V1 = c.V1 + c.A0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 2;
        c.A0 = c.V0 + c.A0;
        c.LoadWord(2, m, c.A1);
        c.V1 = 0x000C0000u;
        c.V0 = c.V0 | c.V1;
        if ((int)c.A0 >= 0) {
            c.StoreWord(2, m, c.A1);
            goto L80101018;
        }
        c.StoreWord(2, m, c.A1);
        c.A0 = c.A0 + 0xFFFu;
        L80101018: ;
        c.V0 = (uint)((int)c.A0 >> 12);
        m.WriteU16((c.A1 + 0x84u), (ushort)c.V0);
        c.V0 = 0xFFFFFA0Bu;
        m.WriteU16((c.A2 + 0x2u), (ushort)c.V0);
        c.A0 = (uint)(short)m.ReadU16((c.A1 + 0x20u));
        c.A0 = 0u - c.A0;
        c.V1 = c.A0 << 1;
        c.V1 = c.V1 + c.A0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 2;
        c.V0 = c.V0 + c.A0;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L80101050;
        }
        L80101050: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        m.WriteU16((c.A2 + 0x4u), (ushort)c.V0);
        goto L80101088;
        L8010105C: ;
        c.CopyRegister(4, 16);
        L80101060: ;
        c.CopyRegister(5, 6);
        c.RA = 0x80101068u;
        Vigilante82PC.func_80032CF0(c, m);
        goto L80101088;
        L80101070: ;
        c.CopyRegister(5, 6);
        c.RA = 0x80101078u;
        Vigilante82PC.func_80032B90(c, m);
        goto L80101088;
        L80101080: ;
        c.CopyRegister(4, 16);
        c.RA = 0x80101088u;
        Vigilante82PC.func_8004DC94(c, m);
        L80101088: ;
        c.LoadWord(31, m, (c.SP + 0x30u));
        c.LoadWord(19, m, (c.SP + 0x2Cu));
        c.LoadWord(18, m, (c.SP + 0x28u));
        c.LoadWord(17, m, (c.SP + 0x24u));
        c.LoadWord(16, m, (c.SP + 0x20u));
        c.CopyRegister(2, 0);
        c.SP = c.SP + 0x38u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void castle(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.CopyRegister(17, 6);
        if (c.A1 == 0u) {
            c.StoreWord(31, m, (c.SP + 0x18u));
            goto L801010D8;
        }
        c.StoreWord(31, m, (c.SP + 0x18u));
        c.V0 = 0x00000003u;
        if (c.A1 == c.V0) {
            c.CopyRegister(2, 0);
            goto L801011EC;
        }
        c.CopyRegister(2, 0);
        goto L80101234;
        L801010D8: ;
        c.S1 = c.S0 + 0x4Cu;
        c.LoadWord(2, m, (c.S0 + 0x4Cu));
        c.LoadWord(3, m, (c.S0 + 0x88u));
        c.A0 = c.S0 + 0x88u;
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x4Cu));
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(3, m, (c.A0 + 0x4u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(2, m, (c.S1 + 0x8u));
        c.LoadWord(3, m, (c.A0 + 0x8u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x8u));
        c.LoadWord(8, m, (c.S0 + 0x4Cu));
        c.LoadWord(9, m, (c.S0 + 0x50u));
        c.LoadWord(10, m, (c.S0 + 0x54u));
        c.StoreWord(8, m, (c.S0 + 0x34u));
        c.StoreWord(9, m, (c.S0 + 0x38u));
        c.StoreWord(10, m, (c.S0 + 0x3Cu));
        c.LoadWord(2, m, (c.S0 + 0x8Cu));
        c.V0 = c.V0 + 0x38u;
        c.V0 = c.V0 < 0x00000038u ? 1u : 0u;
        if (c.V0 == 0u) {
            c.A2 = 0x00000097u;
            goto L80101164;
        }
        c.A2 = 0x00000097u;
        c.V0 = 0x80060000u;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x9u));
        c.LoadWord(5, m, 0x8006B41Cu);
        c.CopyRegister(7, 17);
        c.RA = 0x80101150u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E580u);
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.V1 = 0xFFFFFFDFu;
        m.WriteU16((c.S0 + 0xAu), (ushort)0u);
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        L80101164: ;
        c.LoadWord(2, m, (c.S0 + 0x8Cu));
        c.CopyRegister(4, 16);
        c.V0 = c.V0 + 0x38u;
        c.StoreWord(2, m, (c.S0 + 0x8Cu));
        c.RA = 0x80101178u;
        Vigilante82PC.func_8002D1DC(c, m);
        c.LoadWord(4, m, (c.S0 + 0x4Cu));
        c.LoadWord(5, m, (c.S0 + 0x54u));
        c.RA = 0x80101188u;
        Vigilante82PC.func_8001B750(c, m);
        c.LoadWord(3, m, (c.S0 + 0x50u));
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(4, 17);
            goto L80101230;
        }
        c.CopyRegister(4, 17);
        c.A1 = 0x00000030u;
        c.A2 = 0x0000012Cu;
        c.RA = 0x801011A4u;
        Vigilante82PC.func_8004E128(c, m);
        c.CopyRegister(4, 2);
        c.A1 = c.A0 + 0x48u;
        c.A2 = c.A0 + 0x10u;
        c.RA = 0x801011B4u;
        Vigilante82PC.func_8002D114(c, m);
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 0);
        c.RA = 0x801011C0u;
        Vigilante82PC.func_8004DE54(c, m);
        c.A2 = 0x000000A6u;
        c.V0 = 0x80060000u;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x9u));
        c.LoadWord(5, m, 0x8006B41Cu);
        c.CopyRegister(7, 17);
        c.RA = 0x801011D8u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 16);
        c.A1 = 0x00000001u;
        c.RA = 0x801011E4u;
        Vigilante82PC.func_800309C8(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L80101234;
        L801011EC: ;
        c.LoadWord(4, m, c.S1);
        c.V1 = m.ReadU8((c.A0 + 0x4u));
        if (c.V1 == c.A1) {
            c.V0 = 0x00000002u;
            goto L80101230;
        }
        c.V0 = 0x00000002u;
        if (c.V1 != c.V0) {
            c.CopyRegister(4, 16);
            goto L80101218;
        }
        c.RA = 0x8010120Cu;
        Vigilante82PC.func_80039AF8(c, m);
        c.V0 = 0x000003E8u;
        m.WriteU16((c.S0 + 0x1Cu), (ushort)c.V0);
        c.CopyRegister(4, 16);
        L80101218: ;
        c.CopyRegister(5, 17);
        c.A2 = 0x0000000Cu;
        c.A3 = 0x000000A6u;
        c.RA = 0x80101228u;
        Vigilante82PC.func_80042638(c, m);
        goto L80101234;
        L80101230: ;
        c.CopyRegister(2, 0);
        L80101234: ;
        c.LoadWord(31, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x20u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Canon(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x28u;
        c.StoreWord(18, m, (c.SP + 0x18u));
        c.CopyRegister(18, 4);
        c.V1 = c.A1 - 0x1u;
        c.V0 = c.V1 < 0x00000008u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x24u));
        c.StoreWord(20, m, (c.SP + 0x20u));
        c.StoreWord(19, m, (c.SP + 0x1Cu));
        c.StoreWord(17, m, (c.SP + 0x14u));
        if (c.V0 == 0u) {
            c.StoreWord(16, m, (c.SP + 0x10u));
            goto L801014E8;
        }
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x160u;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x801014A4u: goto L801014A4;
            case 0x801012C8u: goto L801012C8;
            case 0x80101290u: goto L80101290;
            case 0x801014E8u: goto L801014E8;
            case 0x80101468u: goto L80101468;
            case 0x801012A4u: goto L801012A4;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L80101290: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 6);
        c.RA = 0x8010129Cu;
        Vigilante82PC.func_80032CF0(c, m);
        goto L801012B0;
        L801012A4: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 6);
        c.RA = 0x801012B0u;
        Vigilante82PC.func_80032B90(c, m);
        L801012B0: ;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801014EC;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x801012C0u;
        Vigilante82PC.func_80030C68(c, m);
        c.CopyRegister(2, 0);
        goto L801014EC;
        L801012C8: ;
        c.LoadWord(2, m, (c.S2 + 0x4u));
        c.V0 = c.V0 & 0x0004u;
        if (c.V0 == 0u) {
            c.CopyRegister(4, 18);
            goto L80101444;
        }
        c.CopyRegister(4, 18);
        c.A1 = 0u | 0x8000u;
        c.RA = 0x801012E0u;
        Vigilante82PC.func_8002C5F4(c, m);
        c.A1 = 0x0000012Bu;
        c.A2 = 0x00000098u;
        c.A3 = 0x00000008u;
        c.LoadWord(4, m, (c.S2 + 0x5Cu));
        c.CopyRegister(16, 2);
        c.RA = 0x801012F8u;
        Vigilante82PC.func_8002C17C(c, m);
        c.CopyRegister(17, 2);
        c.V0 = 0x00000007u;
        m.WriteU8((c.S1 + 0x8u), (byte)c.V0);
        c.V0 = m.ReadU16((c.S2 + 0xAu));
        m.WriteU16((c.S1 + 0xAu), (ushort)c.V0);
        c.S3 = (uint)(short)m.ReadU16((c.S0 + 0x12u));
        c.S4 = c.S1 + 0x20u;
        c.RA = 0x80101318u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 20);
        c.CopyRegister(5, 18);
        c.CopyRegister(6, 16);
        c.V1 = c.V0 << 3;
        c.V1 = c.V1 - c.V0;
        c.V1 = c.V1 << 4;
        c.V1 = c.V1 + c.V0;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.S3 + c.V1;
        c.V1 = c.V1 - 0x38u;
        m.WriteU16((c.S0 + 0x12u), (ushort)c.V1);
        c.RA = 0x80101348u;
        Vigilante82PC.func_8002CEAC(c, m);
        c.CopyRegister(4, 17);
        c.V0 = 0x00000490u;
        c.LoadWord(8, m, (c.S1 + 0x34u));
        c.LoadWord(9, m, (c.S1 + 0x38u));
        c.LoadWord(10, m, (c.S1 + 0x3Cu));
        c.StoreWord(8, m, (c.S1 + 0x4Cu));
        c.StoreWord(9, m, (c.S1 + 0x50u));
        c.StoreWord(10, m, (c.S1 + 0x54u));
        m.WriteU16((c.S0 + 0x12u), (ushort)c.S3);
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.V0 = 0x0000012Cu;
        m.WriteU16((c.S1 + 0x1Cu), (ushort)c.V0);
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x10A8u;
        c.StoreWord(2, m, (c.S1 + 0x0u));
        c.RA = 0x80101388u;
        Vigilante82PC.func_800305FC(c, m);
        m.WriteU16((c.S1 + 0x94u), (ushort)0u);
        c.RA = 0x80101390u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 11;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V0 = (uint)(short)m.ReadU16((c.S1 + 0x24u));
        c.S3 = c.V1 + 0x2800u;
        { var _r = (long)(int)c.V0 * (int)c.S3; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.S0 = c.S1 + 0x4Cu;
        c.A0 = c.LO;
        if ((int)c.A0 >= 0) {
            c.A1 = c.S1 + 0x88u;
            goto L801013C0;
        }
        c.A1 = c.S1 + 0x88u;
        c.A0 = c.A0 + 0xFFFu;
        L801013C0: ;
        c.V0 = (uint)(short)m.ReadU16((c.S1 + 0x2Au));
        { var _r = (long)(int)c.V0 * (int)c.S3; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = (uint)((int)c.A0 >> 12);
        c.V1 = c.LO;
        if ((int)c.V1 >= 0) {
            c.StoreWord(2, m, (c.S1 + 0x88u));
            goto L801013DC;
        }
        c.StoreWord(2, m, (c.S1 + 0x88u));
        c.V1 = c.V1 + 0xFFFu;
        L801013DC: ;
        c.V0 = (uint)((int)c.V1 >> 12);
        c.StoreWord(2, m, (c.A1 + 0x4u));
        c.V0 = (uint)(short)m.ReadU16((c.S1 + 0x30u));
        { var _r = (long)(int)c.V0 * (int)c.S3; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.LO;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L801013F8;
        }
        L801013F8: ;
        c.CopyRegister(4, 20);
        c.V0 = (uint)((int)c.V0 >> 12);
        c.StoreWord(2, m, (c.A1 + 0x8u));
        c.RA = 0x80101408u;
        Vigilante82PC.func_8002A39C(c, m);
        c.CopyRegister(4, 16);
        c.A1 = 0x00000011u;
        c.RA = 0x80101414u;
        Vigilante82PC.func_8004DE54(c, m);
        c.RA = 0x8010141Cu;
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.A2 = 0x0000008Du;
        m.WriteU8((c.S1 + 0x9u), (byte)c.V0);
        c.V0 = 0x80060000u;
        c.LoadWord(5, m, 0x8006B41Cu);
        c.CopyRegister(7, 16);
        c.RA = 0x8010143Cu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E580u);
        c.CopyRegister(2, 0);
        goto L801014EC;
        L80101444: ;
        c.CopyRegister(4, 18);
        c.RA = 0x8010144Cu;
        Vigilante82PC.func_8002C05C(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x80101454u;
        Vigilante82PC.func_80030BF0(c, m);
        c.CopyRegister(4, 18);
        c.A1 = 0x00000021u;
        c.RA = 0x80101460u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(2, 0);
        goto L801014EC;
        L80101468: ;
        c.RA = 0x80101470u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 18);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.A1 = c.V1 << 4;
        c.A1 = c.A1 - c.V1;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x258u;
        c.RA = 0x80101494u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x8010149Cu;
        Vigilante82PC.func_80030C20(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L801014EC;
        L801014A4: ;
        c.RA = 0x801014ACu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 18);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.A1 = c.V1 << 4;
        c.A1 = c.A1 - c.V1;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x258u;
        c.RA = 0x801014D0u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x801014D8u;
        Vigilante82PC.func_80030C20(c, m);
        c.V0 = 0x80104D8Cu;
        c.LoadWord(3, m, (c.S2 + 0x10u));
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x0u));
        L801014E8: ;
        c.CopyRegister(2, 0);
        L801014EC: ;
        c.LoadWord(31, m, (c.SP + 0x24u));
        c.LoadWord(20, m, (c.SP + 0x20u));
        c.LoadWord(19, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x28u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamProjectileSpawn(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x28u;
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.CopyRegister(17, 4);
        c.StoreWord(31, m, (c.SP + 0x20u));
        c.StoreWord(19, m, (c.SP + 0x1Cu));
        c.StoreWord(18, m, (c.SP + 0x18u));
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.LoadWord(4, m, (c.S1 + 0x10u));
        c.CopyRegister(19, 5);
        c.RA = 0x80101534u;
        Vigilante82PC.func_8002CE50(c, m);
        c.LoadWord(4, m, (c.S1 + 0x10u));
        c.CopyRegister(18, 2);
        c.RA = 0x80101540u;
        Vigilante82PC.func_8002CCBC(c, m);
        c.CopyRegister(4, 2);
        c.A1 = 0x00000098u;
        c.RA = 0x8010154Cu;
        Vigilante82PC.func_80052188(c, m);
        c.CopyRegister(16, 2);
        c.V0 = m.ReadU16((c.S1 + 0xAu));
        c.CopyRegister(4, 16);
        m.WriteU16((c.S0 + 0xAu), (ushort)c.V0);
        c.V0 = 0x00000007u;
        c.LoadWord(3, m, c.S2);
        c.LoadWord(6, m, (c.S2 + 0x4u));
        c.LoadWord(7, m, (c.S2 + 0x8u));
        c.LoadWord(8, m, (c.S2 + 0xCu));
        c.StoreWord(3, m, (c.S0 + 0x20u));
        c.StoreWord(6, m, (c.S0 + 0x24u));
        c.StoreWord(7, m, (c.S0 + 0x28u));
        c.StoreWord(8, m, (c.S0 + 0x2Cu));
        c.LoadWord(3, m, (c.S2 + 0x10u));
        c.LoadWord(6, m, (c.S2 + 0x14u));
        c.LoadWord(7, m, (c.S2 + 0x18u));
        c.LoadWord(8, m, (c.S2 + 0x1Cu));
        c.StoreWord(3, m, (c.S0 + 0x30u));
        c.StoreWord(6, m, (c.S0 + 0x34u));
        c.StoreWord(7, m, (c.S0 + 0x38u));
        c.StoreWord(8, m, (c.S0 + 0x3Cu));
        c.LoadWord(3, m, (c.S2 + 0x14u));
        c.LoadWord(6, m, (c.S2 + 0x18u));
        c.LoadWord(7, m, (c.S2 + 0x1Cu));
        c.StoreWord(3, m, (c.S0 + 0x4Cu));
        c.StoreWord(6, m, (c.S0 + 0x50u));
        c.StoreWord(7, m, (c.S0 + 0x54u));
        m.WriteU8((c.S0 + 0x8u), (byte)c.V0);
        c.V0 = 0x00000080u;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.StoreWord(19, m, (c.S0 + 0x0u));
        c.RA = 0x801015CCu;
        Vigilante82PC.func_800305FC(c, m);
        c.CopyRegister(2, 16);
        c.LoadWord(31, m, (c.SP + 0x20u));
        c.LoadWord(19, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x28u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamGoodProjectile(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x40u;
        c.StoreWord(17, m, (c.SP + 0x34u));
        c.CopyRegister(17, 4);
        c.StoreWord(31, m, (c.SP + 0x38u));
        if (c.A1 == 0u) {
            c.StoreWord(16, m, (c.SP + 0x30u));
            goto L80101618;
        }
        c.StoreWord(16, m, (c.SP + 0x30u));
        c.V0 = 0x00000003u;
        if (c.A1 == c.V0) {
            c.CopyRegister(2, 0);
            goto L801018C4;
        }
        c.CopyRegister(2, 0);
        goto L801018F0;
        L80101618: ;
        c.S0 = c.S1 + 0x4Cu;
        c.LoadWord(2, m, (c.S1 + 0x4Cu));
        c.LoadWord(3, m, (c.S1 + 0x88u));
        c.A0 = c.S1 + 0x88u;
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4Cu));
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(3, m, (c.A0 + 0x4u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(2, m, (c.S0 + 0x8u));
        c.LoadWord(3, m, (c.A0 + 0x8u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x8u));
        c.LoadWord(9, m, (c.S1 + 0x4Cu));
        c.LoadWord(10, m, (c.S1 + 0x50u));
        c.LoadWord(11, m, (c.S1 + 0x54u));
        c.StoreWord(9, m, (c.S1 + 0x34u));
        c.StoreWord(10, m, (c.S1 + 0x38u));
        c.StoreWord(11, m, (c.S1 + 0x3Cu));
        c.LoadWord(2, m, (c.S1 + 0x8Cu));
        c.V0 = c.V0 + 0x38u;
        c.V0 = c.V0 < 0x00000038u ? 1u : 0u;
        if (c.V0 == 0u) {
            goto L801016AC;
        }
        c.RA = 0x80101684u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.A2 = 0x00000097u;
        c.V0 = 0x80060000u;
        c.LoadWord(5, m, 0x8006B41Cu);
        c.CopyRegister(7, 16);
        c.RA = 0x8010169Cu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E580u);
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.V1 = 0xFFFFFFDFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        L801016AC: ;
        c.LoadWord(2, m, (c.S1 + 0x8Cu));
        c.V1 = m.ReadU16((c.S1 + 0x8Au));
        c.V0 = c.V0 + 0x38u;
        c.StoreWord(2, m, (c.S1 + 0x8Cu));
        m.WriteU16((c.SP + 0x10u), (ushort)c.V1);
        c.V0 = m.ReadU16((c.S1 + 0x8Eu));
        c.A1 = c.SP + 0x10u;
        m.WriteU16((c.A1 + 0x2u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.S1 + 0x92u));
        c.A0 = c.S1 + 0x20u;
        m.WriteU16((c.A1 + 0x4u), (ushort)c.V0);
        c.RA = 0x801016DCu;
        Vigilante82PC.func_8002A5EC(c, m);
        c.LoadWord(5, m, (c.S1 + 0x84u));
        if (c.A1 == 0u) {
            c.T0 = c.SP + 0x18u;
            goto L8010181C;
        }
        c.T0 = c.SP + 0x18u;
        c.LoadWord(3, m, (c.A1 + 0x48u));
        c.A1 = c.A1 + 0x48u;
        c.LoadWord(2, m, (c.S1 + 0x4Cu));
        c.V1 = c.V1 - c.V0;
        c.StoreWord(3, m, (c.SP + 0x18u));
        c.LoadWord(2, m, (c.A1 + 0x4u));
        c.LoadWord(4, m, (c.S0 + 0x4u));
        { var _r = (long)(int)c.V1 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.V0 - c.A0;
        c.StoreWord(2, m, (c.T0 + 0x4u));
        c.LoadWord(2, m, (c.A1 + 0x8u));
        c.LoadWord(4, m, (c.S0 + 0x8u));
        c.V0 = c.V0 - c.A0;
        c.StoreWord(2, m, (c.T0 + 0x8u));
        c.A2 = c.HI;
        c.A3 = c.LO;
        c.LoadWord(2, m, (c.SP + 0x20u));
        { var _r = (long)(int)c.V0 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.T2 = c.HI;
        c.T3 = c.LO;
        c.StoreWord(10, m, (c.SP + 0x28u));
        c.StoreWord(11, m, (c.SP + 0x2Cu));
        c.A1 = c.A3 + c.T3;
        c.V0 = c.A1 < c.T3 ? 1u : 0u;
        c.A0 = c.A2 + c.T2;
        c.A0 = c.A0 + c.V0;
        c.RA = 0x80101758u;
        Vigilante82PC.func_8002ABC4(c, m);
        c.LoadWord(6, m, (c.SP + 0x1Cu));
        c.LoadWord(3, m, (c.S1 + 0x8Cu));
        c.V0 = c.A2 << 3;
        { var _r = (long)(int)c.V1 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.V0 - c.A2;
        c.V0 = c.V0 << 4;
        c.CopyRegister(5, 2);
        c.A0 = (uint)((int)c.V0 >> 31);
        c.T2 = c.HI;
        c.T3 = c.LO;
        c.StoreWord(10, m, (c.SP + 0x28u));
        c.StoreWord(11, m, (c.SP + 0x2Cu));
        c.T3 = c.T3 + c.A1;
        c.V0 = c.T3 < c.A1 ? 1u : 0u;
        c.T2 = c.T2 + c.A0;
        c.T2 = c.T2 + c.V0;
        c.CopyRegister(4, 10);
        c.CopyRegister(5, 11);
        c.RA = 0x801017A4u;
        Vigilante82PC.func_8002ABC4(c, m);
        c.LoadWord(3, m, (c.S1 + 0x8Cu));
        c.A0 = c.V0 - c.V1;
        if (c.A0 == 0u) {
            goto L8010181C;
        }
        c.LoadWord(3, m, (c.SP + 0x18u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 3;
        if (c.A0 != 0u) { if ((int)c.V0 == int.MinValue && (int)c.A0 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.V0 / (int)c.A0); c.HI = (uint)((int)c.V0 % (int)c.A0); } }
        c.V0 = c.LO;
        c.LoadWord(3, m, (c.S1 + 0x88u));
        c.V0 = c.V0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L801017DC;
        }
        L801017DC: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.V1 + c.V0;
        c.StoreWord(2, m, (c.S1 + 0x88u));
        c.LoadWord(3, m, (c.SP + 0x20u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 3;
        if (c.A0 != 0u) { if ((int)c.V0 == int.MinValue && (int)c.A0 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.V0 / (int)c.A0); c.HI = (uint)((int)c.V0 % (int)c.A0); } }
        c.V0 = c.LO;
        c.LoadWord(3, m, (c.S1 + 0x90u));
        c.V0 = c.V0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L80101810;
        }
        L80101810: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.V1 + c.V0;
        c.StoreWord(2, m, (c.S1 + 0x90u));
        L8010181C: ;
        c.V0 = m.ReadU16((c.S1 + 0x94u));
        c.V0 = c.V0 - 0x1u;
        m.WriteU16((c.S1 + 0x94u), (ushort)c.V0);
        c.V0 = c.V0 & 0x0003u;
        if (c.V0 != 0u) {
            c.A0 = c.S1 + 0x4Cu;
            goto L80101864;
        }
        c.A0 = c.S1 + 0x4Cu;
        c.A1 = 0x00000013u;
        c.RA = 0x8010183Cu;
        Vigilante82PC.func_8004DE54(c, m);
        c.LoadWord(3, m, c.V0);
        c.V1 = c.V1 | 0x0400u;
        c.StoreWord(3, m, c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.S1 + 0x94u));
        c.CopyRegister(4, 2);
        c.V0 = c.V1 << 1;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 << 5;
        m.WriteU16((c.A0 + 0x44u), (ushort)c.V0);
        c.RA = 0x80101864u;
        Vigilante82PC.func_8002CF74(c, m);
        L80101864: ;
        c.LoadWord(4, m, (c.S1 + 0x4Cu));
        c.LoadWord(5, m, (c.S1 + 0x54u));
        c.RA = 0x80101874u;
        Vigilante82PC.func_8001B750(c, m);
        c.LoadWord(3, m, (c.S1 + 0x50u));
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.S0 = c.S1 + 0x4Cu;
            goto L801018EC;
        }
        c.S0 = c.S1 + 0x4Cu;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 0);
        c.RA = 0x80101890u;
        Vigilante82PC.func_8004DE54(c, m);
        c.RA = 0x80101898u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.V0 = 0x80060000u;
        c.LoadWord(5, m, 0x8006B41Cu);
        c.A2 = 0x000000A6u;
        c.CopyRegister(7, 16);
        c.RA = 0x801018B0u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 17);
        c.A1 = 0x00000001u;
        c.RA = 0x801018BCu;
        Vigilante82PC.func_800309C8(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L801018F0;
        L801018C4: ;
        c.LoadWord(2, m, c.A2);
        c.V0 = m.ReadU8((c.V0 + 0x4u));
        if (c.V0 == c.A1) {
            c.CopyRegister(4, 17);
            goto L801018EC;
        }
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 6);
        c.A2 = 0x0000001Au;
        c.A3 = 0x000000A6u;
        c.RA = 0x801018E4u;
        Vigilante82PC.func_80042638(c, m);
        goto L801018F0;
        L801018EC: ;
        c.CopyRegister(2, 0);
        L801018F0: ;
        c.LoadWord(31, m, (c.SP + 0x38u));
        c.LoadWord(17, m, (c.SP + 0x34u));
        c.LoadWord(16, m, (c.SP + 0x30u));
        c.SP = c.SP + 0x40u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void mushroom_good(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x28u;
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.CopyRegister(17, 4);
        c.StoreWord(19, m, (c.SP + 0x1Cu));
        c.CopyRegister(19, 5);
        c.CopyRegister(5, 6);
        c.V1 = c.S3 - 0x1u;
        c.V0 = c.V1 < 0x00000008u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x20u));
        c.StoreWord(18, m, (c.SP + 0x18u));
        if (c.V0 == 0u) {
            c.StoreWord(16, m, (c.SP + 0x10u));
            goto L80101B58;
        }
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x190u;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80101B00u: goto L80101B00;
            case 0x80101AB8u: goto L80101AB8;
            case 0x80101950u: goto L80101950;
            case 0x80101B58u: goto L80101B58;
            case 0x80101AA8u: goto L80101AA8;
            case 0x80101B34u: goto L80101B34;
            case 0x8010196Cu: goto L8010196C;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L80101950: ;
        c.LoadWord(5, m, c.A1);
        c.V1 = m.ReadU8((c.A1 + 0x4u));
        c.V0 = 0x00000007u;
        if (c.V1 != c.V0) {
            c.CopyRegister(2, 0);
            goto L80101B5C;
        }
        c.CopyRegister(2, 0);
        c.S2 = (uint)(short)m.ReadU16((c.A1 + 0x6u));
        c.A1 = m.ReadU16((c.A1 + 0xCu));
        L8010196C: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80101974u;
        Vigilante82PC.func_80032B90(c, m);
        if (c.V0 == 0u) {
            c.CopyRegister(4, 17);
            goto L80101B58;
        }
        c.CopyRegister(4, 17);
        c.A1 = 0x80100000u;
        c.A1 = c.A1 + 0x15ECu;
        c.RA = 0x80101988u;
        Vigilante82PC.DreamProjectileSpawn(c, m);
        c.CopyRegister(5, 0);
        c.CopyRegister(16, 2);
        c.V0 = 0x00000003u;
        if (c.S3 != c.V0) {
            c.A0 = c.S1 + 0x4Cu;
            goto L801019A0;
        }
        c.A0 = c.S1 + 0x4Cu;
        c.CopyRegister(5, 18);
        L801019A0: ;
        c.RA = 0x801019A8u;
        Vigilante82PC.DreamlandFindNearestObject(c, m);
        c.A0 = c.S0 + 0x24u;
        c.StoreWord(2, m, (c.S0 + 0x84u));
        c.LoadWord(5, m, (c.S0 + 0x84u));
        c.V0 = 0x00000064u;
        m.WriteU16((c.S0 + 0xCu), (ushort)c.V0);
        c.LoadWord(2, m, c.S0);
        c.A1 = c.A1 + 0x24u;
        c.V0 = c.V0 | 0x0020u;
        c.StoreWord(2, m, c.S0);
        c.RA = 0x801019D0u;
        Vigilante82PC.func_80029F6C(c, m);
        c.V1 = (uint)((int)c.V0 >> 9);
        c.V0 = (int)c.V1 < 4096 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.A1 = 0x00001000u;
            goto L801019F4;
        }
        c.A1 = 0x00001000u;
        c.A1 = 0x00002000u;
        c.CopyRegister(2, 5);
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(5, 3);
            goto L801019F4;
        }
        L801019F4: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x12u));
        c.V0 = 0u - c.V0;
        { var _r = (long)(int)c.V0 * (int)c.A1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.A0 = c.LO;
        if ((int)c.A0 >= 0) {
            c.A2 = c.S0 + 0x88u;
            goto L80101A10;
        }
        c.A2 = c.S0 + 0x88u;
        c.A0 = c.A0 + 0xFFFu;
        L80101A10: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x18u));
        c.V0 = 0u - c.V0;
        { var _r = (long)(int)c.V0 * (int)c.A1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = (uint)((int)c.A0 >> 12);
        c.V1 = c.LO;
        if ((int)c.V1 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x88u));
            goto L80101A30;
        }
        c.StoreWord(2, m, (c.S0 + 0x88u));
        c.V1 = c.V1 + 0xFFFu;
        L80101A30: ;
        c.V0 = (uint)((int)c.V1 >> 12);
        c.StoreWord(2, m, (c.A2 + 0x4u));
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x1Eu));
        c.V0 = 0u - c.V0;
        { var _r = (long)(int)c.V0 * (int)c.A1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.LO;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L80101A50;
        }
        L80101A50: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        c.StoreWord(2, m, (c.A2 + 0x8u));
        c.RA = 0x80101A5Cu;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S1 + 0x5Cu));
        c.A2 = 0x0000004Fu;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S0 + 0x24u;
        c.RA = 0x80101A74u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.RA = 0x80101A7Cu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 17);
        c.A1 = c.V0 << 3;
        c.A1 = c.A1 - c.V0;
        c.A1 = c.A1 << 5;
        c.A1 = c.A1 + c.V0;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x708u;
        c.RA = 0x80101AA0u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(2, 0);
        goto L80101B5C;
        L80101AA8: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80101AB0u;
        Vigilante82PC.func_80030C20(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L80101B5C;
        L80101AB8: ;
        c.CopyRegister(4, 17);
        c.A2 = 0x000001A4u;
        c.LoadWord(5, m, (c.S1 + 0x5Cu));
        c.A3 = 0x00000008u;
        c.RA = 0x80101ACCu;
        Vigilante82PC.func_8002C344(c, m);
        c.CopyRegister(4, 17);
        c.RA = 0x80101AD4u;
        Vigilante82PC.func_8002D1DC(c, m);
        c.CopyRegister(4, 17);
        c.RA = 0x80101ADCu;
        Vigilante82PC.func_8002C7D0(c, m);
        c.CopyRegister(4, 17);
        c.RA = 0x80101AE4u;
        Vigilante82PC.func_80030BF0(c, m);
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.V1 = 0xFFFFFFDFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(3, m, (c.S1 + 0x10u));
        c.V0 = 0x80104D8Cu;
        goto L80101B0C;
        L80101B00: ;
        c.LoadWord(3, m, (c.S1 + 0x10u));
        if (c.V1 == 0u) {
            c.V0 = 0x80104D8Cu;
            goto L80101B18;
        }
        c.V0 = 0x80104D8Cu;
        L80101B0C: ;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, c.V1);
        goto L80101B58;
        L80101B18: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80101B20u;
        Vigilante82PC.func_80030C20(c, m);
        c.CopyRegister(4, 17);
        c.A1 = 0x0000012Cu;
        c.RA = 0x80101B2Cu;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(2, 0);
        goto L80101B5C;
        L80101B34: ;
        c.RA = 0x80101B3Cu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V0 = c.V0 & 0x0001u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80101B5C;
        }
        c.CopyRegister(2, 0);
        c.A0 = 0x00000080u;
        c.RA = 0x80101B50u;
        Vigilante82PC.func_8002CBE8(c, m);
        c.StoreWord(17, m, (c.V0 + 0x5Cu));
        goto L80101B5C;
        L80101B58: ;
        c.CopyRegister(2, 0);
        L80101B5C: ;
        c.LoadWord(31, m, (c.SP + 0x20u));
        c.LoadWord(19, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x28u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamBadProjectile(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x38u;
        c.StoreWord(17, m, (c.SP + 0x2Cu));
        c.CopyRegister(17, 4);
        c.StoreWord(18, m, (c.SP + 0x30u));
        c.CopyRegister(18, 6);
        c.V0 = 0x00000003u;
        c.StoreWord(31, m, (c.SP + 0x34u));
        if (c.A1 == c.V0) {
            c.StoreWord(16, m, (c.SP + 0x28u));
            goto L80101DE0;
        }
        c.StoreWord(16, m, (c.SP + 0x28u));
        c.V0 = c.A1 < 0x00000004u ? 1u : 0u;
        if (c.V0 == 0u) {
            c.V0 = 0x00000004u;
            goto L80101BB8;
        }
        c.V0 = 0x00000004u;
        if (c.A1 == 0u) {
            c.CopyRegister(2, 0);
            goto L80101BC8;
        }
        c.CopyRegister(2, 0);
        goto L80101E18;
        L80101BB8: ;
        if (c.A1 == c.V0) {
            c.CopyRegister(2, 0);
            goto L80101E08;
        }
        c.CopyRegister(2, 0);
        goto L80101E18;
        L80101BC8: ;
        c.S0 = c.S1 + 0x4Cu;
        c.V0 = m.ReadU16((c.S1 + 0x46u));
        c.LoadWord(3, m, (c.S1 + 0x4Cu));
        c.LoadWord(4, m, (c.S1 + 0x88u));
        c.V0 = c.V0 + 0x88u;
        c.V1 = c.V1 + c.A0;
        c.A0 = c.S1 + 0x88u;
        m.WriteU16((c.S1 + 0x46u), (ushort)c.V0);
        c.StoreWord(3, m, (c.S1 + 0x4Cu));
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(3, m, (c.A0 + 0x4u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(2, m, (c.S0 + 0x8u));
        c.LoadWord(3, m, (c.A0 + 0x8u));
        c.CopyRegister(4, 17);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x8u));
        c.RA = 0x80101C14u;
        Vigilante82PC.func_8002CF74(c, m);
        c.LoadWord(2, m, (c.S1 + 0x8Cu));
        c.LoadWord(5, m, (c.S1 + 0x84u));
        c.V0 = c.V0 + 0x1Cu;
        if (c.A1 == 0u) {
            c.StoreWord(2, m, (c.S1 + 0x8Cu));
            goto L80101D60;
        }
        c.StoreWord(2, m, (c.S1 + 0x8Cu));
        c.LoadWord(3, m, (c.A1 + 0x48u));
        c.A1 = c.A1 + 0x48u;
        c.LoadWord(2, m, (c.S1 + 0x4Cu));
        c.T0 = c.SP + 0x10u;
        c.V1 = c.V1 - c.V0;
        c.StoreWord(3, m, (c.SP + 0x10u));
        c.LoadWord(2, m, (c.A1 + 0x4u));
        c.LoadWord(4, m, (c.S0 + 0x4u));
        { var _r = (long)(int)c.V1 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.V0 - c.A0;
        c.StoreWord(2, m, (c.T0 + 0x4u));
        c.LoadWord(2, m, (c.A1 + 0x8u));
        c.LoadWord(4, m, (c.S0 + 0x8u));
        c.V0 = c.V0 - c.A0;
        c.StoreWord(2, m, (c.T0 + 0x8u));
        c.A2 = c.HI;
        c.A3 = c.LO;
        c.LoadWord(2, m, (c.SP + 0x18u));
        { var _r = (long)(int)c.V0 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.T2 = c.HI;
        c.T3 = c.LO;
        c.StoreWord(10, m, (c.SP + 0x20u));
        c.StoreWord(11, m, (c.SP + 0x24u));
        c.A1 = c.A3 + c.T3;
        c.V0 = c.A1 < c.T3 ? 1u : 0u;
        c.A0 = c.A2 + c.T2;
        c.A0 = c.A0 + c.V0;
        c.RA = 0x80101C9Cu;
        Vigilante82PC.func_8002ABC4(c, m);
        c.LoadWord(6, m, (c.SP + 0x14u));
        c.LoadWord(3, m, (c.S1 + 0x8Cu));
        c.V0 = c.A2 << 3;
        { var _r = (long)(int)c.V1 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.V0 - c.A2;
        c.V0 = c.V0 << 4;
        c.CopyRegister(5, 2);
        c.A0 = (uint)((int)c.V0 >> 31);
        c.T2 = c.HI;
        c.T3 = c.LO;
        c.StoreWord(10, m, (c.SP + 0x20u));
        c.StoreWord(11, m, (c.SP + 0x24u));
        c.T3 = c.T3 + c.A1;
        c.V0 = c.T3 < c.A1 ? 1u : 0u;
        c.T2 = c.T2 + c.A0;
        c.T2 = c.T2 + c.V0;
        c.CopyRegister(4, 10);
        c.CopyRegister(5, 11);
        c.RA = 0x80101CE8u;
        Vigilante82PC.func_8002ABC4(c, m);
        c.LoadWord(3, m, (c.S1 + 0x8Cu));
        c.A0 = c.V0 - c.V1;
        if (c.A0 == 0u) {
            goto L80101D60;
        }
        c.LoadWord(3, m, (c.SP + 0x10u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 2;
        if (c.A0 != 0u) { if ((int)c.V0 == int.MinValue && (int)c.A0 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.V0 / (int)c.A0); c.HI = (uint)((int)c.V0 % (int)c.A0); } }
        c.V0 = c.LO;
        c.LoadWord(3, m, (c.S1 + 0x88u));
        c.V0 = c.V0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L80101D20;
        }
        L80101D20: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.V1 + c.V0;
        c.StoreWord(2, m, (c.S1 + 0x88u));
        c.LoadWord(3, m, (c.SP + 0x18u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 2;
        if (c.A0 != 0u) { if ((int)c.V0 == int.MinValue && (int)c.A0 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.V0 / (int)c.A0); c.HI = (uint)((int)c.V0 % (int)c.A0); } }
        c.V0 = c.LO;
        c.LoadWord(3, m, (c.S1 + 0x90u));
        c.V0 = c.V0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L80101D54;
        }
        L80101D54: ;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.V0 = c.V1 + c.V0;
        c.StoreWord(2, m, (c.S1 + 0x90u));
        L80101D60: ;
        c.LoadWord(4, m, (c.S1 + 0x4Cu));
        c.LoadWord(5, m, (c.S1 + 0x54u));
        c.RA = 0x80101D70u;
        Vigilante82PC.func_8001B750(c, m);
        c.LoadWord(3, m, (c.S1 + 0x50u));
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.S0 = c.S1 + 0x4Cu;
            goto L80101DBC;
        }
        c.S0 = c.S1 + 0x4Cu;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 0);
        c.RA = 0x80101D8Cu;
        Vigilante82PC.func_8004DE54(c, m);
        c.A2 = 0x000000A6u;
        c.V0 = 0x80060000u;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x9u));
        c.LoadWord(5, m, 0x8006B41Cu);
        c.CopyRegister(7, 16);
        c.RA = 0x80101DA4u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 17);
        c.A1 = 0x00000001u;
        m.WriteU8((c.A0 + 0x9u), (byte)0u);
        c.RA = 0x80101DB4u;
        Vigilante82PC.func_800309C8(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L80101E18;
        L80101DBC: ;
        if (c.S2 == 0u) {
            c.CopyRegister(2, 0);
            goto L80101E18;
        }
        c.CopyRegister(2, 0);
        c.A0 = c.S1 + 0x34u;
        c.RA = 0x80101DCCu;
        Vigilante82PC.func_8001E478(c, m);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x9u));
        c.CopyRegister(5, 2);
        c.RA = 0x80101DD8u;
        Vigilante82PC.func_8001E2C8(c, m);
        c.CopyRegister(2, 0);
        goto L80101E18;
        L80101DE0: ;
        c.LoadWord(2, m, c.S2);
        c.V0 = m.ReadU8((c.V0 + 0x4u));
        if (c.V0 == c.A1) {
            c.CopyRegister(4, 17);
            goto L80101E14;
        }
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 18);
        c.A2 = 0x0000001Au;
        c.A3 = 0x000000A6u;
        c.RA = 0x80101E00u;
        Vigilante82PC.func_80042638(c, m);
        goto L80101E18;
        L80101E08: ;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x9u));
        c.RA = 0x80101E14u;
        Vigilante82PC.func_8001DE78(c, m);
        L80101E14: ;
        c.CopyRegister(2, 0);
        L80101E18: ;
        c.LoadWord(31, m, (c.SP + 0x34u));
        c.LoadWord(18, m, (c.SP + 0x30u));
        c.LoadWord(17, m, (c.SP + 0x2Cu));
        c.LoadWord(16, m, (c.SP + 0x28u));
        c.SP = c.SP + 0x38u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void mushroom_bad(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x28u;
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.CopyRegister(17, 4);
        c.StoreWord(19, m, (c.SP + 0x1Cu));
        c.CopyRegister(19, 5);
        c.CopyRegister(5, 6);
        c.V1 = c.S3 - 0x1u;
        c.V0 = c.V1 < 0x00000008u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x20u));
        c.StoreWord(18, m, (c.SP + 0x18u));
        if (c.V0 == 0u) {
            c.StoreWord(16, m, (c.SP + 0x10u));
            goto L801020CC;
        }
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x1C0u;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80102054u: goto L80102054;
            case 0x80102014u: goto L80102014;
            case 0x80101E7Cu: goto L80101E7C;
            case 0x801020CCu: goto L801020CC;
            case 0x80102004u: goto L80102004;
            case 0x801020A8u: goto L801020A8;
            case 0x80101E98u: goto L80101E98;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L80101E7C: ;
        c.LoadWord(5, m, c.A1);
        c.V1 = m.ReadU8((c.A1 + 0x4u));
        c.V0 = 0x00000007u;
        if (c.V1 != c.V0) {
            c.CopyRegister(2, 0);
            goto L801020D0;
        }
        c.CopyRegister(2, 0);
        c.S2 = (uint)(short)m.ReadU16((c.A1 + 0x6u));
        c.A1 = m.ReadU16((c.A1 + 0xCu));
        L80101E98: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80101EA0u;
        Vigilante82PC.func_80032B90(c, m);
        if (c.V0 == 0u) {
            c.CopyRegister(4, 17);
            goto L801020CC;
        }
        c.CopyRegister(4, 17);
        c.A1 = 0x80100000u;
        c.A1 = c.A1 + 0x1B78u;
        c.RA = 0x80101EB4u;
        Vigilante82PC.DreamProjectileSpawn(c, m);
        c.CopyRegister(5, 0);
        c.CopyRegister(16, 2);
        c.V0 = 0x00000003u;
        if (c.S3 != c.V0) {
            c.A0 = c.S1 + 0x4Cu;
            goto L80101ECC;
        }
        c.A0 = c.S1 + 0x4Cu;
        c.CopyRegister(5, 18);
        L80101ECC: ;
        c.RA = 0x80101ED4u;
        Vigilante82PC.DreamlandFindNearestObject(c, m);
        c.A0 = c.S0 + 0x24u;
        c.StoreWord(2, m, (c.S0 + 0x84u));
        c.CopyRegister(5, 2);
        c.V0 = 0x00000064u;
        c.A1 = c.A1 + 0x24u;
        m.WriteU16((c.S0 + 0xCu), (ushort)c.V0);
        c.RA = 0x80101EF0u;
        Vigilante82PC.func_80029F6C(c, m);
        c.A0 = (uint)((int)c.V0 >> 9);
        c.V0 = (int)c.A0 < 4096 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.V1 = 0x00001000u;
            goto L80101F1C;
        }
        c.V1 = 0x00001000u;
        c.V1 = 0x00002000u;
        c.CopyRegister(2, 3);
        c.V0 = (int)c.V0 < (int)c.A0 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(3, 4);
            goto L80101F1C;
        }
        c.V0 = c.V1 >> 31;
        goto L80101F20;
        L80101F1C: ;
        c.V0 = c.V1 >> 31;
        L80101F20: ;
        c.V0 = c.V1 + c.V0;
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x12u));
        c.A1 = (uint)((int)c.V0 >> 1);
        c.V1 = 0u - c.V1;
        { var _r = (long)(int)c.V1 * (int)c.A1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        if ((int)c.V1 >= 0) {
            c.A2 = c.S0 + 0x88u;
            goto L80101F44;
        }
        c.A2 = c.S0 + 0x88u;
        c.V1 = c.V1 + 0xFFFu;
        L80101F44: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x18u));
        c.V0 = 0u - c.V0;
        { var _r = (long)(int)c.V0 * (int)c.A1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = (uint)((int)c.V1 >> 12);
        c.A0 = c.LO;
        if ((int)c.A0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x88u));
            goto L80101F64;
        }
        c.StoreWord(2, m, (c.S0 + 0x88u));
        c.A0 = c.A0 + 0xFFFu;
        L80101F64: ;
        c.V0 = (uint)((int)c.A0 >> 12);
        c.StoreWord(2, m, (c.A2 + 0x4u));
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x1Eu));
        c.V0 = 0u - c.V0;
        { var _r = (long)(int)c.V0 * (int)c.A1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.LO;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L80101F84;
        }
        L80101F84: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        c.StoreWord(2, m, (c.A2 + 0x8u));
        c.RA = 0x80101F90u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S1 + 0x5Cu));
        c.A2 = 0x0000004Fu;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S0 + 0x24u;
        c.RA = 0x80101FA8u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.RA = 0x80101FB0u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        m.WriteU8((c.S0 + 0x5u), (byte)c.V0);
        c.LoadWord(2, m, (c.S1 + 0x5Cu));
        c.A2 = 0x00000049u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x80101FD0u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E098u);
        c.RA = 0x80101FD8u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 17);
        c.A1 = c.V0 << 3;
        c.A1 = c.A1 - c.V0;
        c.A1 = c.A1 << 5;
        c.A1 = c.A1 + c.V0;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x708u;
        c.RA = 0x80101FFCu;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(2, 0);
        goto L801020D0;
        L80102004: ;
        c.CopyRegister(4, 17);
        c.RA = 0x8010200Cu;
        Vigilante82PC.func_80030C20(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L801020D0;
        L80102014: ;
        c.CopyRegister(4, 17);
        c.A2 = 0x000001AFu;
        c.LoadWord(5, m, (c.S1 + 0x5Cu));
        c.A3 = 0x00000008u;
        c.RA = 0x80102028u;
        Vigilante82PC.func_8002C344(c, m);
        c.CopyRegister(4, 17);
        c.RA = 0x80102030u;
        Vigilante82PC.func_8002D1DC(c, m);
        c.CopyRegister(4, 17);
        c.RA = 0x80102038u;
        Vigilante82PC.func_80030BF0(c, m);
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.V1 = 0xFFFFFFDFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(3, m, (c.S1 + 0x10u));
        c.V0 = 0x80104D8Cu;
        goto L80102060;
        L80102054: ;
        c.LoadWord(3, m, (c.S1 + 0x10u));
        if (c.V1 == 0u) {
            c.V0 = 0x80104D8Cu;
            goto L8010206C;
        }
        c.V0 = 0x80104D8Cu;
        L80102060: ;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, c.V1);
        goto L801020CC;
        L8010206C: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80102074u;
        Vigilante82PC.func_80030C20(c, m);
        c.RA = 0x8010207Cu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 17);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.A1 = c.V1 << 4;
        c.A1 = c.A1 - c.V1;
        c.A1 = c.A1 << 2;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x12Cu;
        c.RA = 0x801020A0u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(2, 0);
        goto L801020D0;
        L801020A8: ;
        c.RA = 0x801020B0u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V0 = c.V0 & 0x0001u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801020D0;
        }
        c.CopyRegister(2, 0);
        c.A0 = 0x00000080u;
        c.RA = 0x801020C4u;
        Vigilante82PC.func_8002CBE8(c, m);
        c.StoreWord(17, m, (c.V0 + 0x5Cu));
        goto L801020D0;
        L801020CC: ;
        c.CopyRegister(2, 0);
        L801020D0: ;
        c.LoadWord(31, m, (c.SP + 0x20u));
        c.LoadWord(19, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x28u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamOrbitingActor(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x18u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        if (c.A1 != 0u) {
            c.StoreWord(31, m, (c.SP + 0x14u));
            goto L801021EC;
        }
        c.StoreWord(31, m, (c.SP + 0x14u));
        c.V1 = 0x80060000u;
        c.V0 = m.ReadU16((c.S0 + 0x86u));
        c.A1 = 0x80065C90u;
        c.V0 = c.V0 & 0x0FFFu;
        c.V0 = c.V0 << 2;
        c.V0 = c.V0 + c.A1;
        c.V1 = (uint)(short)m.ReadU16(c.V0);
        c.LoadWord(2, m, (c.S0 + 0x98u));
        { var _r = (long)(int)c.V0 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V0 = c.LO;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L80102130;
        }
        L80102130: ;
        c.A0 = (uint)((int)c.V0 >> 12);
        c.LoadWord(3, m, (c.S0 + 0x4Cu));
        c.V0 = m.ReadU16((c.S0 + 0x86u));
        c.V1 = c.V1 + c.A0;
        c.V0 = c.V0 & 0x0FFFu;
        c.V0 = c.V0 << 2;
        c.V0 = c.V0 + c.A1;
        c.StoreWord(3, m, (c.S0 + 0x34u));
        c.V1 = (uint)(short)m.ReadU16((c.V0 + 0x2u));
        c.LoadWord(2, m, (c.S0 + 0x98u));
        { var _r = (long)(int)c.V0 * (int)c.V1; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        if ((int)c.V1 < 0) {
            c.V1 = c.V1 + 0xFFFu;
            goto L80102168;
        }
        L80102168: ;
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x82u));
        c.LoadWord(2, m, (c.S0 + 0x98u));
        c.A0 = c.A0 << 16;
        if (c.V0 != 0u) { if ((int)c.A0 == int.MinValue && (int)c.V0 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.A0 / (int)c.V0); c.HI = (uint)((int)c.A0 % (int)c.V0); } }
        c.A0 = c.LO;
        c.V1 = (uint)((int)c.V1 >> 12);
        c.LoadWord(2, m, (c.S0 + 0x54u));
        c.LoadWord(5, m, (c.S0 + 0x8Cu));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x3Cu));
        c.LoadWord(2, m, (c.S0 + 0x38u));
        c.V1 = m.ReadU16((c.S0 + 0x86u));
        c.V0 = c.V0 + c.A1;
        c.StoreWord(2, m, (c.S0 + 0x38u));
        c.LoadWord(2, m, (c.S0 + 0x98u));
        c.V1 = c.V1 + c.A0;
        c.LoadWord(4, m, (c.S0 + 0xA0u));
        m.WriteU16((c.S0 + 0x86u), (ushort)c.V1);
        c.V1 = c.V1 + 0x400u;
        m.WriteU16((c.S0 + 0x46u), (ushort)c.V1);
        c.V0 = c.V0 + c.A0;
        if (c.A2 == 0u) {
            c.StoreWord(2, m, (c.S0 + 0x98u));
            goto L801021CC;
        }
        c.StoreWord(2, m, (c.S0 + 0x98u));
        c.CopyRegister(4, 16);
        c.RA = 0x801021CCu;
        Vigilante82PC.func_8002CF44(c, m);
        L801021CC: ;
        c.LoadWord(2, m, (c.S0 + 0x94u));
        c.V0 = c.V0 - 0x1u;
        if (c.V0 != 0u) {
            c.StoreWord(2, m, (c.S0 + 0x94u));
            goto L801021EC;
        }
        c.StoreWord(2, m, (c.S0 + 0x94u));
        c.CopyRegister(4, 16);
        c.RA = 0x801021E4u;
        Vigilante82PC.func_800309A0(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L801021F0;
        L801021EC: ;
        c.CopyRegister(2, 0);
        L801021F0: ;
        c.LoadWord(31, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x18u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamActorDestroy(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        c.StoreWord(31, m, (c.SP + 0x18u));
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.LoadWord(4, m, (c.S0 + 0x40u));
        c.CopyRegister(17, 5);
        c.RA = 0x80102220u;
        Vigilante82PC.func_8001FEB8(c, m);
        c.LoadWord(4, m, (c.S0 + 0x10u));
        c.RA = 0x8010222Cu;
        Vigilante82PC.func_8002C4B4(c, m);
        c.CopyRegister(4, 16);
        c.A2 = c.S1 & 0xFFFFu;
        c.LoadWord(5, m, (c.A0 + 0x5Cu));
        c.A3 = 0x00000008u;
        c.RA = 0x80102240u;
        Vigilante82PC.func_8002C344(c, m);
        c.LoadWord(31, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x20u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamActorBurst(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x38u;
        c.StoreWord(18, m, (c.SP + 0x18u));
        c.CopyRegister(18, 4);
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.CopyRegister(17, 0);
        c.StoreWord(30, m, (c.SP + 0x30u));
        c.FP = 0x80060000u;
        c.StoreWord(23, m, (c.SP + 0x2Cu));
        c.S7 = 0x000000A4u;
        c.StoreWord(22, m, (c.SP + 0x28u));
        c.S6 = 0x80100000u;
        c.StoreWord(21, m, (c.SP + 0x24u));
        c.S5 = 0x00000200u;
        c.V0 = 0x80100000u;
        c.StoreWord(20, m, (c.SP + 0x20u));
        c.S4 = c.V0 + 0x20ECu;
        c.StoreWord(19, m, (c.SP + 0x1Cu));
        c.S3 = 0x000000B4u;
        c.StoreWord(31, m, (c.SP + 0x34u));
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.A1 = 0x000001F6u;
        L801022A8: ;
        c.A2 = 0x000000A0u;
        c.LoadWord(2, m, 0x8006B814u);
        c.A3 = 0x00000008u;
        c.LoadWord(4, m, (c.V0 + 0x5Cu));
        c.S1 = c.S1 + 0x1u;
        c.RA = 0x801022C0u;
        Vigilante82PC.func_8002C17C(c, m);
        c.CopyRegister(16, 2);
        c.LoadWord(8, m, (c.S2 + 0x4Cu));
        c.LoadWord(9, m, (c.S2 + 0x50u));
        c.LoadWord(10, m, (c.S2 + 0x54u));
        c.StoreWord(8, m, (c.S0 + 0x4Cu));
        c.StoreWord(9, m, (c.S0 + 0x50u));
        c.StoreWord(10, m, (c.S0 + 0x54u));
        c.StoreWord(23, m, (c.S0 + 0x4u));
        c.T3 = c.S6 + 0x1E0u;
        c.T0 = m.ReadWordLeft(c.T0, c.T3);
        c.T0 = m.ReadWordRight(c.T0, (c.T3 + 0x3u));
        c.T1 = (uint)(short)m.ReadU16((c.T3 + 0x4u));
        m.WriteWordLeft((c.S0 + 0x80u), c.T0);
        m.WriteWordRight((c.S0 + 0x83u), c.T0);
        m.WriteU16((c.S0 + 0x84u), (ushort)c.T1);
        c.RA = 0x80102304u;
        Vigilante82PC.func_8002AC5C(c, m);
        m.WriteU16((c.S0 + 0x86u), (ushort)c.V0);
        c.StoreWord(21, m, (c.S0 + 0xA0u));
        c.RA = 0x80102310u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 16);
        c.V0 = c.V0 << 10;
        c.V0 = 0u - c.V0;
        c.V0 = (uint)((int)c.V0 >> 15);
        c.V0 = c.V0 - 0x400u;
        c.StoreWord(2, m, (c.A0 + 0x8Cu));
        c.StoreWord(20, m, (c.A0 + 0x0u));
        c.StoreWord(19, m, (c.A0 + 0x94u));
        c.RA = 0x80102334u;
        Vigilante82PC.func_8003066C(c, m);
        c.V0 = (int)c.S1 < 3 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.A1 = 0x000001F6u;
            goto L801022A8;
        }
        c.S0 = 0x80100000u;
        c.RA = 0x80102348u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(17, 2);
        c.S0 = c.S0 + 0x3210u;
        c.RA = 0x80102354u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 17);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 >> 15;
        c.V1 = c.V1 << 1;
        c.V1 = c.V1 + c.S0;
        c.S0 = c.S2 + 0x34u;
        c.LoadWord(2, m, (c.S2 + 0x5Cu));
        c.A2 = m.ReadU16(c.V1);
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 16);
        c.RA = 0x80102384u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 16);
        c.A1 = 0x00000011u;
        c.RA = 0x80102390u;
        Vigilante82PC.func_8004DE54(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x80102398u;
        Vigilante82PC.func_800309A0(c, m);
        c.LoadWord(31, m, (c.SP + 0x34u));
        c.LoadWord(30, m, (c.SP + 0x30u));
        c.LoadWord(23, m, (c.SP + 0x2Cu));
        c.LoadWord(22, m, (c.SP + 0x28u));
        c.LoadWord(21, m, (c.SP + 0x24u));
        c.LoadWord(20, m, (c.SP + 0x20u));
        c.LoadWord(19, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x38u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamActorPathStart(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x40u;
        c.StoreWord(16, m, (c.SP + 0x30u));
        c.CopyRegister(16, 4);
        c.StoreWord(17, m, (c.SP + 0x34u));
        c.S1 = c.S0 + 0x4Cu;
        c.CopyRegister(4, 17);
        c.StoreWord(18, m, (c.SP + 0x38u));
        c.StoreWord(31, m, (c.SP + 0x3Cu));
        c.CopyRegister(18, 6);
        c.RA = 0x801023F0u;
        Vigilante82PC.func_800518DC(c, m);
        c.CopyRegister(5, 17);
        c.StoreWord(2, m, (c.S0 + 0x80u));
        c.CopyRegister(4, 2);
        c.V0 = 0x00000001u;
        c.A2 = c.SP + 0x10u;
        c.StoreWord(2, m, (c.S0 + 0x88u));
        c.RA = 0x8010240Cu;
        Vigilante82PC.func_80051334(c, m);
        c.LoadWord(3, m, (c.S0 + 0x4u));
        c.V0 = c.V0 << 16;
        c.StoreWord(2, m, (c.S0 + 0x8Cu));
        c.StoreWord(18, m, (c.S0 + 0x84u));
        c.V1 = c.V1 | 0x0180u;
        c.StoreWord(3, m, (c.S0 + 0x4u));
        c.LoadWord(31, m, (c.SP + 0x3Cu));
        c.LoadWord(18, m, (c.SP + 0x38u));
        c.LoadWord(17, m, (c.SP + 0x34u));
        c.LoadWord(16, m, (c.SP + 0x30u));
        c.SP = c.SP + 0x40u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void DreamActorPathStep(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x40u;
        c.StoreWord(16, m, (c.SP + 0x30u));
        c.CopyRegister(16, 4);
        c.StoreWord(31, m, (c.SP + 0x38u));
        c.StoreWord(17, m, (c.SP + 0x34u));
        c.LoadWord(5, m, (c.S0 + 0x8Cu));
        if ((int)c.A1 >= 0) {
            c.A1 = (uint)((int)c.A1 >> 16);
            goto L80102468;
        }
        c.V0 = 0u | 0xFFFFu;
        c.A1 = c.A1 + c.V0;
        c.A1 = (uint)((int)c.A1 >> 16);
        L80102468: ;
        c.A2 = c.S0 + 0x34u;
        c.LoadWord(4, m, (c.S0 + 0x80u));
        c.A3 = c.SP + 0x10u;
        c.RA = 0x80102478u;
        Vigilante82PC.func_800285E4(c, m);
        c.LoadWord(2, m, (c.S0 + 0x88u));
        if (c.V0 != 0u) {
            c.S1 = c.SP + 0x20u;
            goto L8010249C;
        }
        c.S1 = c.SP + 0x20u;
        c.LoadWord(2, m, (c.SP + 0x10u));
        c.LoadWord(3, m, (c.SP + 0x18u));
        c.V0 = 0u - c.V0;
        c.V1 = 0u - c.V1;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.StoreWord(3, m, (c.SP + 0x18u));
        L8010249C: ;
        c.LoadWord(4, m, (c.S0 + 0x34u));
        c.LoadWord(5, m, (c.S0 + 0x3Cu));
        c.CopyRegister(6, 17);
        c.RA = 0x801024ACu;
        Vigilante82PC.func_8001B998(c, m);
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x20u));
        c.LoadWord(2, m, (c.SP + 0x10u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x24u));
        c.A0 = c.LO;
        c.LoadWord(2, m, (c.SP + 0x18u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.V0 = c.A0 + c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0x1Fu;
            goto L801024DC;
        }
        L801024DC: ;
        c.A0 = c.SP + 0x10u;
        c.A1 = c.SP + 0x28u;
        c.V0 = (uint)((int)c.V0 >> 5);
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.RA = 0x801024F0u;
        Vigilante82PC.func_800598D8(c, m);
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 4);
        c.RA = 0x801024FCu;
        Vigilante82PC.func_8005991C(c, m);
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x2Au));
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x24u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x22u));
        c.A0 = c.LO;
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x2Cu));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.V0 = c.A0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L8010252C;
        }
        L8010252C: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        m.WriteU16((c.S0 + 0x20u), (ushort)c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x2Cu));
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x20u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x24u));
        c.A0 = c.LO;
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x28u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.V0 = c.A0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L80102564;
        }
        L80102564: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        m.WriteU16((c.S0 + 0x26u), (ushort)c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x28u));
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x22u));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x20u));
        c.A0 = c.LO;
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x2Au));
        { var _r = (long)(int)c.V1 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.V0 = c.A0 - c.V1;
        if ((int)c.V0 < 0) {
            c.V0 = c.V0 + 0xFFFu;
            goto L8010259C;
        }
        L8010259C: ;
        c.V0 = (uint)((int)c.V0 >> 12);
        m.WriteU16((c.S0 + 0x2Cu), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x20u));
        c.V0 = 0u - c.V0;
        m.WriteU16((c.S0 + 0x22u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x22u));
        c.V0 = 0u - c.V0;
        m.WriteU16((c.S0 + 0x28u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x24u));
        c.V0 = 0u - c.V0;
        m.WriteU16((c.S0 + 0x2Eu), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x28u));
        m.WriteU16((c.S0 + 0x24u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x2Au));
        m.WriteU16((c.S0 + 0x2Au), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x2Cu));
        m.WriteU16((c.S0 + 0x30u), (ushort)c.V0);
        c.LoadWord(2, m, (c.SP + 0x10u));
        { var _r = (long)(int)c.V0 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.LoadWord(2, m, (c.SP + 0x18u));
        { var _r = (long)(int)c.V0 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.T1 = c.LO;
        c.A0 = c.V1 + c.T1;
        c.RA = 0x80102604u;
        Vigilante82PC.func_800597BC(c, m);
        c.CopyRegister(3, 2);
        c.LoadWord(2, m, (c.S0 + 0x88u));
        c.LoadWord(4, m, (c.S0 + 0x8Cu));
        if (c.V0 == 0u) {
            goto L80102624;
        }
        c.LoadWord(2, m, (c.S0 + 0x84u));
        c.V0 = c.V0 << 16;
        goto L80102630;
        L80102624: ;
        c.LoadWord(2, m, (c.S0 + 0x84u));
        c.V0 = 0u - c.V0;
        c.V0 = c.V0 << 16;
        L80102630: ;
        if (c.V1 != 0u) { if ((int)c.V0 == int.MinValue && (int)c.V1 == -1) { c.LO = 0x80000000u; c.HI = 0u; } else { c.LO = (uint)((int)c.V0 / (int)c.V1); c.HI = (uint)((int)c.V0 % (int)c.V1); } }
        c.V0 = c.LO;
        c.V0 = c.A0 + c.V0;
        c.StoreWord(2, m, (c.S0 + 0x8Cu));
        c.S1 = 0x10000000u;
        c.V0 = c.S1 < c.V0 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L8010267C;
        }
        c.CopyRegister(2, 0);
        c.LoadWord(4, m, (c.S0 + 0x80u));
        c.A1 = c.S0 + 0x88u;
        c.RA = 0x8010265Cu;
        Vigilante82PC.func_800512A8(c, m);
        if (c.V0 == 0u) {
            c.StoreWord(2, m, (c.S0 + 0x80u));
            goto L80102678;
        }
        c.StoreWord(2, m, (c.S0 + 0x80u));
        c.LoadWord(2, m, (c.S0 + 0x88u));
        c.V0 = c.V0 < 0x00000001u ? 1u : 0u;
        c.V0 = 0u - c.V0;
        c.V0 = c.V0 & c.S1;
        c.StoreWord(2, m, (c.S0 + 0x8Cu));
        L80102678: ;
        c.V0 = 0x00000001u;
        L8010267C: ;
        c.LoadWord(31, m, (c.SP + 0x38u));
        c.LoadWord(17, m, (c.SP + 0x34u));
        c.LoadWord(16, m, (c.SP + 0x30u));
        c.SP = c.SP + 0x40u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Knight(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x80u;
        c.StoreWord(18, m, (c.SP + 0x78u));
        c.CopyRegister(18, 4);
        c.StoreWord(16, m, (c.SP + 0x70u));
        c.CopyRegister(16, 6);
        c.V0 = c.A1 < 0x00000009u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x7Cu));
        if (c.V0 == 0u) {
            c.StoreWord(17, m, (c.SP + 0x74u));
            goto L80102864;
        }
        c.StoreWord(17, m, (c.SP + 0x74u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x1F0u;
        c.V1 = c.A1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x801026D0u: goto L801026D0;
            case 0x80102814u: goto L80102814;
            case 0x80102864u: goto L80102864;
            case 0x801026FCu: goto L801026FC;
            case 0x8010280Cu: goto L8010280C;
            case 0x801027E8u: goto L801027E8;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L801026D0: ;
        c.CopyRegister(4, 18);
        c.RA = 0x801026D8u;
        Dispatcher.Call(c, m, 0x8010243Cu);
        if (c.S0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102868;
        }
        c.CopyRegister(2, 0);
        c.A0 = c.S2 + 0x34u;
        c.RA = 0x801026E8u;
        Vigilante82PC.func_8001E7A8(c, m);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x9u));
        c.CopyRegister(5, 2);
        c.RA = 0x801026F4u;
        Vigilante82PC.func_8001E2C8(c, m);
        c.CopyRegister(2, 0);
        goto L80102868;
        L801026FC: ;
        c.LoadWord(2, m, c.S0);
        c.V1 = m.ReadU8((c.V0 + 0x4u));
        c.V0 = 0x00000002u;
        if (c.V1 != c.V0) {
            c.CopyRegister(4, 18);
            goto L801027D8;
        }
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 16);
        c.A2 = c.SP + 0x10u;
        c.RA = 0x8010271Cu;
        Vigilante82PC.func_8002FB70(c, m);
        c.A0 = c.S2 + 0x20u;
        c.A1 = c.SP + 0x30u;
        c.A2 = c.SP + 0x48u;
        c.RA = 0x8010272Cu;
        Vigilante82PC.func_80024238(c, m);
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x4Cu));
        c.V0 = (int)c.V0 < 2049 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L80102868;
        }
        c.CopyRegister(2, 0);
        c.A1 = c.SP + 0x50u;
        c.S1 = c.SP + 0x24u;
        c.CopyRegister(6, 17);
        c.LoadWord(16, m, c.S0);
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x38u));
        c.V1 = (uint)(short)m.ReadU16((c.SP + 0x3Cu));
        c.CopyRegister(4, 16);
        c.V0 = c.V0 << 6;
        c.StoreWord(2, m, (c.SP + 0x50u));
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x3Au));
        c.V1 = c.V1 << 6;
        c.StoreWord(3, m, (c.SP + 0x5Cu));
        c.V0 = c.V0 << 6;
        c.StoreWord(2, m, (c.SP + 0x54u));
        c.RA = 0x80102778u;
        Vigilante82PC.func_8002B1FC(c, m);
        c.CopyRegister(4, 16);
        c.A1 = 0xFFFFFFCEu;
        c.CopyRegister(6, 17);
        c.A3 = 0x00000001u;
        c.RA = 0x8010278Cu;
        Vigilante82PC.func_8003A020(c, m);
        c.A0 = c.S0 + 0x10u;
        c.CopyRegister(5, 17);
        c.S0 = c.SP + 0x60u;
        c.CopyRegister(6, 16);
        c.RA = 0x801027A0u;
        Vigilante82PC.func_80024148(c, m);
        c.CopyRegister(4, 16);
        c.A1 = 0x80800000u;
        c.A1 = c.A1 | 0x8008u;
        c.RA = 0x801027B0u;
        Vigilante82PC.func_8004E414(c, m);
        c.RA = 0x801027B8u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S2 + 0x5Cu));
        c.A2 = 0x00000045u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 16);
        c.RA = 0x801027D0u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(2, 0);
        goto L80102868;
        L801027D8: ;
        c.CopyRegister(5, 16);
        c.RA = 0x801027E0u;
        Vigilante82PC.func_80032CF0(c, m);
        goto L801027F4;
        L801027E8: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 16);
        c.RA = 0x801027F4u;
        Vigilante82PC.func_80032B90(c, m);
        L801027F4: ;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102868;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x80102804u;
        Vigilante82PC.DreamActorBurst(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L80102868;
        L8010280C: ;
        c.V0 = 0x00000090u;
        goto L80102868;
        L80102814: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 0);
        c.A2 = 0x000011E1u;
        c.RA = 0x80102824u;
        Vigilante82PC.DreamActorPathStart(c, m);
        c.RA = 0x8010282Cu;
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.LoadWord(3, m, (c.S2 + 0x5Cu));
        c.A2 = 0x00000050u;
        m.WriteU8((c.S2 + 0x9u), (byte)c.V0);
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x8010284Cu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E098u);
        c.LoadWord(2, m, (c.S2 + 0x4u));
        c.V0 = c.V0 & 0x0008u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102868;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x80102864u;
        Vigilante82PC.func_8004C9C8(c, m);
        L80102864: ;
        c.CopyRegister(2, 0);
        L80102868: ;
        c.LoadWord(31, m, (c.SP + 0x7Cu));
        c.LoadWord(18, m, (c.SP + 0x78u));
        c.LoadWord(17, m, (c.SP + 0x74u));
        c.LoadWord(16, m, (c.SP + 0x70u));
        c.SP = c.SP + 0x80u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Footman_Walk(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x28u;
        c.StoreWord(18, m, (c.SP + 0x18u));
        c.CopyRegister(18, 4);
        c.StoreWord(19, m, (c.SP + 0x1Cu));
        c.CopyRegister(19, 6);
        c.V0 = c.A1 < 0x00000009u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x20u));
        c.StoreWord(17, m, (c.SP + 0x14u));
        if (c.V0 == 0u) {
            c.StoreWord(16, m, (c.SP + 0x10u));
            goto L80102B30;
        }
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x228u;
        c.V1 = c.A1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x801028C4u: goto L801028C4;
            case 0x80102AB0u: goto L80102AB0;
            case 0x8010295Cu: goto L8010295C;
            case 0x801029F0u: goto L801029F0;
            case 0x80102B18u: goto L80102B18;
            case 0x80102B30u: goto L80102B30;
            case 0x80102AA8u: goto L80102AA8;
            case 0x80102A84u: goto L80102A84;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L801028C4: ;
        if (c.S3 == 0u) {
            goto L80102918;
        }
        c.A0 = c.S2 + 0x34u;
        c.RA = 0x801028D4u;
        Vigilante82PC.func_8001E7A8(c, m);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x9u));
        c.CopyRegister(5, 2);
        c.RA = 0x801028E0u;
        Vigilante82PC.func_8001E2C8(c, m);
        c.LoadWord(3, m, (c.S2 + 0x4u));
        c.V0 = 0x00020000u;
        c.V0 = c.V1 & c.V0;
        if (c.V0 != 0u) {
            c.V1 = 0xFFFD0000u;
            goto L80102908;
        }
        c.V0 = 0xFFFE0000u;
        c.V0 = c.V0 | 0xFFFFu;
        c.V0 = c.V1 & c.V0;
        c.StoreWord(2, m, (c.S2 + 0x4u));
        c.V1 = 0xFFFD0000u;
        L80102908: ;
        c.LoadWord(2, m, (c.S2 + 0x4u));
        c.V1 = c.V1 | 0xFFFFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S2 + 0x4u));
        L80102918: ;
        c.CopyRegister(4, 18);
        c.RA = 0x80102920u;
        Dispatcher.Call(c, m, 0x8010243Cu);
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x9u));
        m.WriteU8((c.S2 + 0x8u), (byte)0u);
        c.RA = 0x80102934u;
        Vigilante82PC.func_8001DE78(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x8010293Cu;
        Vigilante82PC.func_80030BA8(c, m);
        c.CopyRegister(4, 18);
        c.A1 = 0x000000F0u;
        c.RA = 0x80102948u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(4, 18);
        c.A1 = 0x000001CAu;
        c.RA = 0x80102954u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.CopyRegister(2, 0);
        goto L80102B34;
        L8010295C: ;
        c.V1 = m.ReadU8((c.S2 + 0x8u));
        c.V0 = 0x00000001u;
        c.V0 = c.V0 - c.V1;
        m.WriteU8((c.S2 + 0x8u), (byte)c.V0);
        c.V0 = c.V0 << 24;
        if (c.V0 == 0u) {
            goto L801029BC;
        }
        c.RA = 0x80102980u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.LoadWord(3, m, (c.S2 + 0x5Cu));
        c.A2 = 0x00000050u;
        m.WriteU8((c.S2 + 0x9u), (byte)c.V0);
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x801029A0u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E098u);
        c.CopyRegister(4, 18);
        c.A1 = 0x000001B8u;
        c.RA = 0x801029ACu;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x801029B4u;
        Vigilante82PC.func_80030B78(c, m);
        c.CopyRegister(4, 18);
        goto L801029E0;
        L801029BC: ;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x9u));
        c.RA = 0x801029C8u;
        Vigilante82PC.func_8001DE78(c, m);
        c.CopyRegister(4, 18);
        c.A1 = 0x000001CAu;
        c.RA = 0x801029D4u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x801029DCu;
        Vigilante82PC.func_80030BA8(c, m);
        c.CopyRegister(4, 18);
        L801029E0: ;
        c.A1 = 0x000000F0u;
        c.RA = 0x801029E8u;
        Vigilante82PC.func_80030CB0(c, m);
        c.CopyRegister(2, 0);
        goto L80102B34;
        L801029F0: ;
        c.LoadWord(2, m, c.S3);
        c.V1 = m.ReadU8((c.V0 + 0x4u));
        c.V0 = 0x00000007u;
        if (c.V1 == c.V0) {
            c.V0 = 0x00000002u;
            goto L80102A0C;
        }
        c.V0 = 0x00000002u;
        if (c.V1 != c.V0) {
            c.CopyRegister(4, 18);
            goto L80102A74;
        }
        c.CopyRegister(4, 18);
        L80102A0C: ;
        c.LoadWord(2, m, (c.S2 + 0x4u));
        c.V1 = 0x00010000u;
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            goto L80102A60;
        }
        c.S0 = 0x80100000u;
        c.RA = 0x80102A28u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(17, 2);
        c.S0 = c.S0 + 0x3210u;
        c.RA = 0x80102A34u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.CopyRegister(4, 17);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 >> 15;
        c.V1 = c.V1 << 1;
        c.V1 = c.V1 + c.S0;
        c.LoadWord(2, m, (c.S2 + 0x5Cu));
        c.A2 = m.ReadU16(c.V1);
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S2 + 0x34u;
        c.RA = 0x80102A60u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        L80102A60: ;
        c.LoadWord(2, m, (c.S2 + 0x4u));
        c.V1 = 0x00030000u;
        c.V0 = c.V0 | c.V1;
        c.StoreWord(2, m, (c.S2 + 0x4u));
        c.CopyRegister(4, 18);
        L80102A74: ;
        c.CopyRegister(5, 19);
        c.RA = 0x80102A7Cu;
        Vigilante82PC.func_80032CF0(c, m);
        goto L80102A90;
        L80102A84: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 19);
        c.RA = 0x80102A90u;
        Vigilante82PC.func_80032B90(c, m);
        L80102A90: ;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x80102AA0u;
        Vigilante82PC.DreamActorBurst(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L80102B34;
        L80102AA8: ;
        c.V0 = 0x00000090u;
        goto L80102B34;
        L80102AB0: ;
        c.CopyRegister(4, 18);
        c.A1 = 0x0000000Au;
        c.A2 = 0x00000BEBu;
        c.RA = 0x80102AC0u;
        Vigilante82PC.DreamActorPathStart(c, m);
        c.RA = 0x80102AC8u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.LoadWord(3, m, (c.S2 + 0x5Cu));
        c.A2 = 0x00000050u;
        m.WriteU8((c.S2 + 0x9u), (byte)c.V0);
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x80102AE8u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E098u);
        c.LoadWord(2, m, (c.S2 + 0x4u));
        c.V1 = 0x00000001u;
        m.WriteU8((c.S2 + 0x8u), (byte)c.V1);
        c.V0 = c.V0 | 0x0080u;
        c.StoreWord(2, m, (c.S2 + 0x4u));
        c.V0 = c.V0 & 0x0008u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x80102B10u;
        Vigilante82PC.func_8004C9C8(c, m);
        c.CopyRegister(2, 0);
        goto L80102B34;
        L80102B18: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x8u));
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x9u));
        c.RA = 0x80102B30u;
        Vigilante82PC.func_8001DE78(c, m);
        L80102B30: ;
        c.CopyRegister(2, 0);
        L80102B34: ;
        c.LoadWord(31, m, (c.SP + 0x20u));
        c.LoadWord(19, m, (c.SP + 0x1Cu));
        c.LoadWord(18, m, (c.SP + 0x18u));
        c.LoadWord(17, m, (c.SP + 0x14u));
        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x28u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Chicken_Walk(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x28u;
        c.StoreWord(16, m, (c.SP + 0x18u));
        c.CopyRegister(16, 4);
        c.StoreWord(18, m, (c.SP + 0x20u));
        c.CopyRegister(18, 6);
        c.V0 = c.A1 < 0x00000009u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x24u));
        if (c.V0 == 0u) {
            c.StoreWord(17, m, (c.SP + 0x1Cu));
            goto L80102FC4;
        }
        c.StoreWord(17, m, (c.SP + 0x1Cu));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x260u;
        c.V1 = c.A1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80102B90u: goto L80102B90;
            case 0x80102F98u: goto L80102F98;
            case 0x80102FC4u: goto L80102FC4;
            case 0x80102D70u: goto L80102D70;
            case 0x80102E88u: goto L80102E88;
            case 0x80102E64u: goto L80102E64;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L80102B90: ;
        c.V1 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.S2 = 0x00000001u;
        if (c.V1 == c.S2) {
            c.V0 = (int)c.V1 < 2 ? 1u : 0u;
            goto L80102BB8;
        }
        c.V0 = (int)c.V1 < 2 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.V0 = 0x00000002u;
            goto L80102D34;
        }
        c.V0 = 0x00000002u;
        if (c.V1 == c.V0) {
            goto L80102C5C;
        }
        goto L80102D34;
        L80102BB8: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x24u));
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.A0 = c.V0 << 1;
        if ((int)c.A0 < 0) {
            c.A0 = c.A0 + 0xFFFu;
            goto L80102BD8;
        }
        L80102BD8: ;
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x30u));
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.T0 = c.V0 << 1;
        c.LoadWord(2, m, (c.S0 + 0x34u));
        c.V1 = (uint)((int)c.A0 >> 12);
        c.V0 = c.V0 + c.V1;
        if ((int)c.T0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x34u));
            goto L80102C08;
        }
        c.StoreWord(2, m, (c.S0 + 0x34u));
        c.T0 = c.T0 + 0xFFFu;
        L80102C08: ;
        c.CopyRegister(4, 16);
        c.S1 = c.S0 + 0x34u;
        c.CopyRegister(5, 17);
        c.A2 = c.SP + 0x10u;
        c.CopyRegister(7, 0);
        c.LoadWord(2, m, (c.S0 + 0x3Cu));
        c.V1 = (uint)((int)c.T0 >> 12);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x3Cu));
        c.RA = 0x80102C30u;
        Vigilante82PC.func_8002CFBC(c, m);
        c.StoreWord(2, m, (c.S0 + 0x38u));
        c.RA = 0x80102C38u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V0 = c.V0 & 0x007Fu;
        if (c.V0 != 0u) {
            c.A1 = 0x00000047u;
            goto L80102D34;
        }
        c.A1 = 0x00000047u;
        c.LoadWord(2, m, (c.S0 + 0x5Cu));
        c.LoadWord(4, m, (c.V0 + 0x8u));
        c.CopyRegister(6, 17);
        c.RA = 0x80102C54u;
        Vigilante82PC.func_8001E8B0(c, m);
        goto L80102D34;
        L80102C5C: ;
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x24u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.V1;
        c.A0 = c.V0 << 4;
        if ((int)c.A0 < 0) {
            c.A0 = c.A0 + 0xFFFu;
            goto L80102C7C;
        }
        L80102C7C: ;
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x30u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.V1;
        c.T0 = c.V0 << 4;
        c.LoadWord(2, m, (c.S0 + 0x34u));
        c.V1 = (uint)((int)c.A0 >> 12);
        c.V0 = c.V0 + c.V1;
        if ((int)c.T0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x34u));
            goto L80102CAC;
        }
        c.StoreWord(2, m, (c.S0 + 0x34u));
        c.T0 = c.T0 + 0xFFFu;
        L80102CAC: ;
        c.CopyRegister(4, 16);
        c.A1 = c.S0 + 0x34u;
        c.A2 = c.SP + 0x10u;
        c.CopyRegister(7, 0);
        c.LoadWord(2, m, (c.S0 + 0x3Cu));
        c.V1 = (uint)((int)c.T0 >> 12);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x3Cu));
        c.RA = 0x80102CD0u;
        Vigilante82PC.func_8002CFBC(c, m);
        c.LoadWord(3, m, (c.S0 + 0x38u));
        c.LoadWord(4, m, (c.S0 + 0x50u));
        c.S1 = c.V1 + c.A0;
        c.CopyRegister(3, 2);
        c.V0 = (int)c.V1 < (int)c.S1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.V0 = c.A0 + 0x38u;
            goto L80102D2C;
        }
        c.V0 = c.A0 + 0x38u;
        c.CopyRegister(17, 3);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001E2u;
        c.RA = 0x80102CFCu;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.RA = 0x80102D04u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V0 = c.V0 << 2;
        c.V0 = (uint)((int)c.V0 >> 15);
        c.V0 = c.V0 + 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V0);
        c.V0 = 0x80104D8Cu;
        c.LoadWord(3, m, (c.S0 + 0x10u));
        c.V0 = c.V0 - 0x184Cu;
        m.WriteU8((c.S0 + 0x8u), (byte)c.S2);
        c.StoreWord(2, m, (c.V1 + 0x0u));
        goto L80102D30;
        L80102D2C: ;
        c.StoreWord(2, m, (c.S0 + 0x50u));
        L80102D30: ;
        c.StoreWord(17, m, (c.S0 + 0x38u));
        L80102D34: ;
        c.LoadWord(3, m, (c.S0 + 0x4u));
        c.V0 = 0x00020000u;
        c.V0 = c.V1 & c.V0;
        if (c.V0 != 0u) {
            c.V1 = 0xFFFD0000u;
            goto L80102D5C;
        }
        c.V0 = 0xFFFE0000u;
        c.V0 = c.V0 | 0xFFFFu;
        c.V0 = c.V1 & c.V0;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.V1 = 0xFFFD0000u;
        L80102D5C: ;
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.V1 = c.V1 | 0xFFFFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        goto L80102FC4;
        L80102D70: ;
        c.LoadWord(2, m, c.S2);
        c.V1 = m.ReadU8((c.V0 + 0x4u));
        c.S1 = 0x00000002u;
        if (c.V1 == c.S1) {
            c.V0 = 0x00000007u;
            goto L80102D8C;
        }
        c.V0 = 0x00000007u;
        if (c.V1 != c.V0) {
            goto L80102DD8;
        }
        L80102D8C: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.V0 = (int)c.V0 < 2 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(4, 16);
            goto L80102E54;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80102DA4u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S0 + 0x5Cu));
        c.A2 = 0x00000048u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S0 + 0x34u;
        c.RA = 0x80102DBCu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001EEu;
        c.RA = 0x80102DC8u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.V0 = 0xFFFFF415u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.S1);
        c.StoreWord(2, m, (c.S0 + 0x50u));
        goto L80102E50;
        L80102DD8: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        if (c.V0 == 0u) {
            c.V1 = 0x00010000u;
            goto L80102E50;
        }
        c.V1 = 0x00010000u;
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            c.CopyRegister(4, 16);
            goto L80102E54;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80102DFCu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.LoadWord(3, m, (c.S0 + 0x4u));
        c.A0 = 0x00030000u;
        c.V1 = c.V1 | c.A0;
        c.StoreWord(3, m, (c.S0 + 0x4u));
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.V0 = c.V1 << 6;
        c.V1 = c.V1 + c.V0;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V0 = (int)c.V1 < 682 ? 1u : 0u;
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x46u));
        if (c.V0 != 0u) {
            c.V0 = c.A0 - 0x2AAu;
            goto L80102E3C;
        }
        c.V0 = c.A0 - 0x2AAu;
        c.V0 = c.A0 + 0x2AAu;
        L80102E3C: ;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 & 0x0FFFu;
        m.WriteU16((c.S0 + 0x46u), (ushort)c.V0);
        c.CopyRegister(4, 16);
        c.RA = 0x80102E50u;
        Vigilante82PC.func_8002CF44(c, m);
        L80102E50: ;
        c.CopyRegister(4, 16);
        L80102E54: ;
        c.CopyRegister(5, 18);
        c.RA = 0x80102E5Cu;
        Vigilante82PC.func_80032CF0(c, m);
        goto L80102E70;
        L80102E64: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 18);
        c.RA = 0x80102E70u;
        Vigilante82PC.func_80032B90(c, m);
        L80102E70: ;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102FC8;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 16);
        c.RA = 0x80102E80u;
        Vigilante82PC.DreamActorBurst(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L80102FC8;
        L80102E88: ;
        c.V1 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.V0 = 0x00000001u;
        if (c.V1 == c.V0) {
            c.V0 = (int)c.V1 < 2 ? 1u : 0u;
            goto L80102F40;
        }
        c.V0 = (int)c.V1 < 2 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102FC8;
        }
        c.CopyRegister(2, 0);
        if (c.V1 != 0u) {
            goto L80102FC8;
        }
        c.V0 = m.ReadU8((c.S0 + 0x9u));
        c.V0 = c.V0 - 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V0);
        c.V0 = c.V0 & 0x00FFu;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L80102FC8;
        }
        c.CopyRegister(2, 0);
        c.RA = 0x80102EC8u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V0 = c.V0 << 10;
        c.V1 = (uint)((int)c.V0 >> 15);
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x46u));
        c.V0 = (int)c.V1 < 512 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.V0 = c.A0 - 0x400u;
            goto L80102EE8;
        }
        c.V0 = c.A0 - 0x400u;
        c.V0 = c.A0 + c.V1;
        goto L80102EEC;
        L80102EE8: ;
        c.V0 = c.V0 + c.V1;
        L80102EEC: ;
        c.V0 = c.V0 & 0x0FFFu;
        m.WriteU16((c.S0 + 0x46u), (ushort)c.V0);
        c.CopyRegister(4, 16);
        c.RA = 0x80102EFCu;
        Vigilante82PC.func_8002CF44(c, m);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001E2u;
        c.RA = 0x80102F08u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.RA = 0x80102F10u;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V1 = c.V0 << 2;
        c.V0 = 0xFFFFFFFEu;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.V1 + 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V1);
        c.V1 = 0x00000001u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V1);
        c.V1 = 0x80104D8Cu;
        c.LoadWord(4, m, (c.S0 + 0x10u));
        c.V1 = c.V1 - 0x184Cu;
        c.StoreWord(3, m, (c.A0 + 0x0u));
        goto L80102FC8;
        L80102F40: ;
        c.V0 = m.ReadU8((c.S0 + 0x9u));
        c.V0 = c.V0 - 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V0);
        c.V0 = c.V0 & 0x00FFu;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L80102FC8;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001E8u;
        c.RA = 0x80102F64u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.RA = 0x80102F6Cu;
        Vigilante82PC.func_8002AC5C(c, m);
        c.V1 = c.V0 << 2;
        c.V0 = 0xFFFFFFFEu;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.V1 + 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V1);
        c.V1 = 0x80104D8Cu;
        c.LoadWord(4, m, (c.S0 + 0x10u));
        c.V1 = c.V1 - 0x184Cu;
        m.WriteU8((c.S0 + 0x8u), (byte)0u);
        c.StoreWord(3, m, (c.A0 + 0x0u));
        goto L80102FC8;
        L80102F98: ;
        c.V0 = 0x00000001u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V0);
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.V1 = 0x00000004u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V1);
        c.LoadWord(3, m, (c.S0 + 0x10u));
        c.V0 = c.V0 | 0x0080u;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.V0 = 0x80104D8Cu;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x0u));
        L80102FC4: ;
        c.CopyRegister(2, 0);
        L80102FC8: ;
        c.LoadWord(31, m, (c.SP + 0x24u));
        c.LoadWord(18, m, (c.SP + 0x20u));
        c.LoadWord(17, m, (c.SP + 0x1Cu));
        c.LoadWord(16, m, (c.SP + 0x18u));
        c.SP = c.SP + 0x28u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void Piggy(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x30u;
        c.StoreWord(16, m, (c.SP + 0x18u));
        c.CopyRegister(16, 4);
        c.StoreWord(20, m, (c.SP + 0x28u));
        c.CopyRegister(20, 6);
        c.V0 = c.A1 < 0x00000009u ? 1u : 0u;
        c.StoreWord(31, m, (c.SP + 0x2Cu));
        c.StoreWord(19, m, (c.SP + 0x24u));
        c.StoreWord(18, m, (c.SP + 0x20u));
        if (c.V0 == 0u) {
            c.StoreWord(17, m, (c.SP + 0x1Cu));
            goto L801031E4;
        }
        c.StoreWord(17, m, (c.SP + 0x1Cu));
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x290u;
        c.V1 = c.A1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (Dispatcher.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80103028u: goto L80103028;
            case 0x801031C8u: goto L801031C8;
            case 0x801031E4u: goto L801031E4;
            case 0x80103054u: goto L80103054;
            case 0x80103198u: goto L80103198;
            case 0x80103174u: goto L80103174;
            default: Dispatcher.Call(c, m, c.V0); return;
        }
        L80103028: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L801031E8;
        }
        c.CopyRegister(2, 0);
        c.V0 = m.ReadU16((c.S0 + 0x46u));
        c.V0 = c.V0 + 0x4u;
        if (c.S4 == 0u) {
            m.WriteU16((c.S0 + 0x46u), (ushort)c.V0);
            goto L801031E4;
        }
        m.WriteU16((c.S0 + 0x46u), (ushort)c.V0);
        c.CopyRegister(4, 16);
        c.RA = 0x8010304Cu;
        Vigilante82PC.func_8002CF44(c, m);
        c.CopyRegister(2, 0);
        goto L801031E8;
        L80103054: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.LoadWord(17, m, c.S4);
        if (c.V0 != 0u) {
            c.CopyRegister(4, 16);
            goto L80103164;
        }
        c.CopyRegister(4, 16);
        c.V1 = m.ReadU8((c.S1 + 0x4u));
        c.S3 = 0x00000002u;
        if (c.V1 == c.S3) {
            c.V0 = 0x00000007u;
            goto L8010307C;
        }
        c.V0 = 0x00000007u;
        if (c.V1 != c.V0) {
            goto L80103164;
        }
        L8010307C: ;
        c.S2 = c.S0 + 0x34u;
        c.RA = 0x80103084u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.A2 = 0x0000004Eu;
        c.LoadWord(2, m, (c.S0 + 0x5Cu));
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 18);
        c.RA = 0x8010309Cu;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001A0u;
        c.RA = 0x801030A8u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.V0 = 0x80104D8Cu;
        c.LoadWord(3, m, (c.S0 + 0x10u));
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x0u));
        c.V0 = 0x00000001u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V0);
        c.V0 = m.ReadU8((c.S1 + 0x4u));
        if (c.V0 != c.S3) {
            c.CopyRegister(4, 16);
            goto L80103164;
        }
        c.CopyRegister(4, 16);
        c.A0 = c.SP + 0x10u;
        c.CopyRegister(5, 18);
        c.A2 = c.S1 + 0x24u;
        c.RA = 0x801030DCu;
        Vigilante82PC.func_8002A168(c, m);
        c.CopyRegister(4, 17);
        c.A1 = 0xFFFFFFCEu;
        c.A2 = 0x80100000u;
        c.A2 = c.A2 + 0x9Cu;
        c.A3 = 0x00000001u;
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x10u));
        c.LoadWord(3, m, (c.A0 + 0x80u));
        c.V0 = c.V0 << 8;
        c.V1 = c.V1 + c.V0;
        c.StoreWord(3, m, (c.A0 + 0x80u));
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x12u));
        c.LoadWord(3, m, (c.A0 + 0x84u));
        c.V0 = c.V0 << 8;
        c.V1 = c.V1 + c.V0;
        c.StoreWord(3, m, (c.A0 + 0x84u));
        c.V0 = (uint)(short)m.ReadU16((c.SP + 0x14u));
        c.LoadWord(3, m, (c.A0 + 0x88u));
        c.V0 = c.V0 << 8;
        c.V1 = c.V1 + c.V0;
        c.StoreWord(3, m, (c.A0 + 0x88u));
        c.RA = 0x80103130u;
        Vigilante82PC.func_8003A020(c, m);
        c.CopyRegister(4, 18);
        c.A1 = 0x80800000u;
        c.A1 = c.A1 | 0x8008u;
        c.RA = 0x80103140u;
        Vigilante82PC.func_8004E414(c, m);
        c.RA = 0x80103148u;
        Vigilante82PC.func_8001DD9C(c, m);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S0 + 0x5Cu));
        c.A2 = 0x00000045u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 18);
        c.RA = 0x80103160u;
        Vigilante82PC.DreamlandPlaySound(c, m, 0x8001E628u);
        c.CopyRegister(4, 16);
        L80103164: ;
        c.CopyRegister(5, 20);
        c.RA = 0x8010316Cu;
        Vigilante82PC.func_80032CF0(c, m);
        goto L80103180;
        L80103174: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 20);
        c.RA = 0x80103180u;
        Vigilante82PC.func_80032B90(c, m);
        L80103180: ;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801031E8;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 16);
        c.RA = 0x80103190u;
        Vigilante82PC.DreamActorBurst(c, m);
        c.V0 = 0xFFFFFFFFu;
        goto L801031E8;
        L80103198: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        if (c.V0 == 0u) {
            c.CopyRegister(4, 16);
            goto L801031E4;
        }
        c.CopyRegister(4, 16);
        c.A1 = 0x0000019Eu;
        c.RA = 0x801031ACu;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.V0 = 0xFFFFFFFEu;
        c.V1 = 0x80104D8Cu;
        c.LoadWord(4, m, (c.S0 + 0x10u));
        c.V1 = c.V1 - 0x184Cu;
        m.WriteU8((c.S0 + 0x8u), (byte)0u);
        c.StoreWord(3, m, (c.A0 + 0x0u));
        goto L801031E8;
        L801031C8: ;
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(3, m, (c.S0 + 0x10u));
        c.V0 = c.V0 | 0x0080u;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.V0 = 0x80104D8Cu;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x0u));
        L801031E4: ;
        c.CopyRegister(2, 0);
        L801031E8: ;
        c.LoadWord(31, m, (c.SP + 0x2Cu));
        c.LoadWord(20, m, (c.SP + 0x28u));
        c.LoadWord(19, m, (c.SP + 0x24u));
        c.LoadWord(18, m, (c.SP + 0x20u));
        c.LoadWord(17, m, (c.SP + 0x1Cu));
        c.LoadWord(16, m, (c.SP + 0x18u));
        c.SP = c.SP + 0x30u;
        return;
    }

    // Native V8:2 implementations emitted as part of the converted arena
    // overlay.  These are map behavior, not runtime compatibility hooks.
    public static void DreamlandVehicleHazardDamage(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        int amount = unchecked((int)c.A1);
        uint impulse = c.SP - 0x10u;
        m.WriteU32(impulse, 0u);
        m.WriteU32(impulse + 4u, 0u);
        m.WriteU32(impulse + 8u, 0u);
        c.A1 = unchecked((uint)-System.Math.Abs(amount));
        c.A2 = impulse;
        c.A3 = 1u;
        Vigilante82PC.func_8003A020(c, m);
        c.RA = savedRa;
    }

    static uint DreamlandNativeSoundId(uint sourceId) => sourceId switch
    {
        0x45u => 0x42u, 0x46u => 0x43u, 0x48u => 0x44u,
        0x49u => 0x45u, 0x4Au => 0x46u, 0x4Bu => 0x47u,
        0x4Cu => 0x48u, 0x4Eu => 0x49u, 0x4Fu => 0x4Au,
        0x50u => 0x4Bu, 0x5Fu => 0x4Cu,
        0x8Du => 0x30u, 0x8Eu => 0x27u, 0x97u => 0x35u, 0xA6u => 0x39u,
        _ => throw new System.InvalidOperationException(
            $"converted arena contains unmapped source SFX 0x{sourceId:X2}"),
    };

    public static void DreamlandPlaySound(
        CpuContext c, IMemory m, uint nativeFunction)
    {
        uint savedRa = c.RA;
        c.A1 = m.ReadU32(0x8006B41Cu);
        c.A2 = DreamlandNativeSoundId(c.A2);
        Dispatcher.Call(c, m, nativeFunction);
        c.RA = savedRa;
    }

    public static void DreamlandFindNearestObject(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint query = c.A0;
        int requestedId = unchecked((int)c.A1);
        int requestedFamily = -1;
        if (requestedId < 0)
            requestedFamily = unchecked((sbyte)m.ReadU8(
                0x8006AD74u + unchecked((uint)~requestedId)));

        uint exact = 0u, family = 0u;
        uint exactDistance = uint.MaxValue, familyDistance = uint.MaxValue;
        uint listRoot = m.ReadU32(0x8006B830u);
        for (uint node = m.ReadU32(listRoot); node != 0u; node = m.ReadU32(node))
        {
            uint obj = m.ReadU32(node + 8u);
            if (obj == 0u || m.ReadU8(obj + 8u) != 2u ||
                m.ReadU16(obj + 0x1Cu) == 0u)
                continue;
            c.A0 = query;
            c.A1 = obj + 0x4Cu;
            Vigilante82PC.func_80029F6C(c, m);
            uint distance = c.V0;
            int objectId = unchecked((short)m.ReadU16(obj + 0x1Au));
            if (objectId == requestedId && distance < exactDistance)
            {
                exact = obj;
                exactDistance = distance;
            }
            else if (objectId <= 0)
            {
                int objectFamily = unchecked((sbyte)m.ReadU8(
                    0x8006AD74u + unchecked((uint)~objectId)));
                if (objectFamily == requestedFamily && distance < familyDistance)
                {
                    family = obj;
                    familyDistance = distance;
                }
            }
        }
        c.V0 = exact != 0u ? exact : family;
        c.RA = savedRa;
    }

    public static void DreamlandContactEffect(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint position = c.A0;
        uint resource = c.A1;
        uint primary = c.A2 & 0xFFFFu;
        uint secondary = c.A3 & 0xFFFFu;
        uint sound = m.ReadU32(c.SP + 0x10u) & 0xFFFFu;
        uint interval = m.ReadU32(c.SP + 0x14u);
        uint lifetime = m.ReadU32(c.SP + 0x18u);

        c.A0 = resource; c.A1 = primary; c.A2 = 0x9Cu; c.A3 = 8u;
        Vigilante82PC.func_8002C17C(c, m);
        uint effect = c.V0;
        c.A0 = effect + 0x20u;
        Vigilante82PC.func_8002A39C(c, m);
        m.WriteU32(effect + 0x34u, m.ReadU32(position));
        m.WriteU32(effect + 0x38u, m.ReadU32(position + 4u));
        m.WriteU32(effect + 0x3Cu, m.ReadU32(position + 8u));
        m.WriteU32(effect, 0x80103500u);
        m.WriteU32(effect + 0x58u, 0x8000u);
        m.WriteU32(effect + 0x98u, resource);
        m.WriteU16(effect + 0x96u, (ushort)secondary);
        m.WriteU32(effect + 4u, m.ReadU32(effect + 4u) | 0xA4u);
        m.WriteU16(effect + 0x82u, (ushort)interval);
        c.A0 = effect; Vigilante82PC.func_800305FC(c, m);
        c.A0 = effect; c.A1 = lifetime; Vigilante82PC.func_80030CB0(c, m);
        Vigilante82PC.func_8001DD9C(c, m);
        c.A0 = c.V0; c.A1 = m.ReadU32(0x8006B41Cu);
        c.A2 = DreamlandNativeSoundId(sound); c.A3 = position;
        Vigilante82PC.func_8001E580(c, m);
        c.V0 = effect;
        c.RA = savedRa;
    }

    public static void DreamlandContactEffectUpdate(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint effect = c.A0;
        if (c.A1 == 2u)
        {
            m.WriteU16(effect + 0x80u, 0xFFFF);
            c.V0 = 0u; c.RA = savedRa; return;
        }
        if (c.A1 == 5u)
        {
            m.WriteU32(effect + 0x64u, 0u);
            m.WriteU32(effect + 4u, m.ReadU32(effect + 4u) | 2u);
            c.V0 = uint.MaxValue; c.RA = savedRa; return;
        }
        if (c.A1 != 0u) { c.V0 = 0u; c.RA = savedRa; return; }

        ushort counter = unchecked((ushort)(m.ReadU16(effect + 0x80u) - 1));
        m.WriteU16(effect + 0x80u, counter);
        if (counter == 0xFFFF)
        {
            c.A0 = m.ReadU32(effect + 0x98u);
            c.A1 = m.ReadU16(effect + 0x96u);
            c.A2 = 0x80u; c.A3 = 8u;
            Vigilante82PC.func_8002C17C(c, m);
            uint particle = c.V0;
            c.A0 = particle + 0x20u;
            Vigilante82PC.func_8002A39C(c, m);
            int radius = unchecked((int)m.ReadU32(effect + 0x58u));
            Vigilante82PC.func_8002AC5C(c, m);
            m.WriteU32(particle + 0x34u,
                unchecked((uint)(((unchecked((int)c.V0) * 2L * radius) >> 15) - radius)));
            m.WriteU32(particle + 0x38u, 0u);
            Vigilante82PC.func_8002AC5C(c, m);
            m.WriteU32(particle + 0x3Cu,
                unchecked((uint)(((unchecked((int)c.V0) * 2L * radius) >> 15) - radius)));
            m.WriteU32(particle, 0x8004CA68u);
            c.A0 = effect; c.A1 = particle;
            Vigilante82PC.func_8002CC48(c, m);
            m.WriteU16(effect + 0x80u, m.ReadU16(effect + 0x82u));
        }
        if (m.ReadU32(effect + 0x10u) == 0u)
        {
            c.A0 = effect;
            Vigilante82PC.func_800309A0(c, m);
            c.V0 = uint.MaxValue;
        }
        else
        {
            c.V0 = 0u;
        }
        c.RA = savedRa;
    }

    public static void DreamlandLegacyModelEvent(CpuContext c, IMemory m)
    {
        uint savedRa = c.RA;
        uint eventCode = c.A1;
        uint actor = c.A2;
        Vigilante82PC.func_8002CD78(c, m);
        uint child = c.V0;
        uint callback = child == 0u ? 0u : m.ReadU32(child);
        if (callback >= 0x80000000u)
        {
            c.A0 = child;
            c.A1 = eventCode;
            c.A2 = actor;
            Dispatcher.Call(c, m, callback);
        }
        else
        {
            c.V0 = 0u;
        }
        c.RA = savedRa;
    }

}

public sealed class LEVELS_N64_DREAMLNDDispatchTable : IOverlay
{
    public string Name => "LEVELS_N64_DREAMLND";
    public int LbaStart => -1;
    public uint Base => 0x80100000u;
    public uint Size => 0x3684u;
    public uint ImageSize => 0x3218u;
    public IReadOnlyDictionary<uint, Action<CpuContext, IMemory>> Functions { get; } =
        new Dictionary<uint, Action<CpuContext, IMemory>>
        {
            [0x801002C0u] = Vigilante82PC.DreamContactActor,
            [0x8010063Cu] = Vigilante82PC.DreamLnd,
            [0x801009E8u] = Vigilante82PC.Butterfly,
            [0x80100A3Cu] = Vigilante82PC.cobblebridge,
            [0x80100B84u] = Vigilante82PC.DBridge,
            [0x80100CACu] = Vigilante82PC.Rainbow,
            [0x80100E1Cu] = Vigilante82PC.well,
            [0x801010A8u] = Vigilante82PC.castle,
            [0x80101248u] = Vigilante82PC.Canon,
            [0x8010150Cu] = Vigilante82PC.DreamProjectileSpawn,
            [0x801015ECu] = Vigilante82PC.DreamGoodProjectile,
            [0x80101904u] = Vigilante82PC.mushroom_good,
            [0x80101B78u] = Vigilante82PC.DreamBadProjectile,
            [0x80101E30u] = Vigilante82PC.mushroom_bad,
            [0x801020ECu] = Vigilante82PC.DreamOrbitingActor,
            [0x80102200u] = Vigilante82PC.DreamActorDestroy,
            [0x80102254u] = Vigilante82PC.DreamActorBurst,
            [0x801023C8u] = Vigilante82PC.DreamActorPathStart,
            [0x8010243Cu] = Vigilante82PC.DreamActorPathStep,
            [0x80102690u] = Vigilante82PC.Knight,
            [0x80102880u] = Vigilante82PC.Footman_Walk,
            [0x80102B50u] = Vigilante82PC.Chicken_Walk,
            [0x80102FE0u] = Vigilante82PC.Piggy,
            [0x80103500u] = Vigilante82PC.DreamlandContactEffectUpdate,
            [0x80103540u] = Vigilante82PC.DreamlandLegacyModelEvent,
        };
}
