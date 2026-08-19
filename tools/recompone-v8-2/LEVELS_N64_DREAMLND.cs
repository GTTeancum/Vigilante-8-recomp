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
        c.LoadWord(2, m, (c.S1 + 0x24u));
        c.LoadWord(5, m, (c.S1 + 0x84u));
        c.V0 = c.V0 + c.V1;
        if ((int)c.A1 >= 0) {
            c.StoreWord(2, m, (c.S1 + 0x24u));
            goto L80100314;
        }
        c.StoreWord(2, m, (c.S1 + 0x24u));
        c.A1 = c.A1 + 0x7Fu;
        L80100314: ;
        c.V1 = (uint)((int)c.A1 >> 7);
        c.LoadWord(2, m, (c.S1 + 0x28u));
        c.LoadWord(4, m, (c.S1 + 0x88u));
        c.V0 = c.V0 + c.V1;
        if ((int)c.A0 >= 0) {
            c.StoreWord(2, m, (c.S1 + 0x28u));
            goto L80100330;
        }
        c.StoreWord(2, m, (c.S1 + 0x28u));
        c.A0 = c.A0 + 0x7Fu;
        L80100330: ;
        c.LoadWord(2, m, (c.S1 + 0x2Cu));
        c.V1 = (uint)((int)c.A0 >> 7);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x2Cu));
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
        c.A0 = m.ReadU8((c.S1 + 0xD2u));
        c.RA = 0x801003A0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800318D0);
        c.CopyRegister(16, 2);
        c.CopyRegister(4, 16);
        c.RA = 0x801003ACu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DCD8);
        if (c.V0 == 0u) {
            c.CopyRegister(4, 16);
            goto L801003BC;
        }
        c.CopyRegister(4, 16);
        c.A1 = 0x0000005Au;
        c.RA = 0x801003BCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        L801003BC: ;
        c.LoadWord(2, m, c.S1);
        c.LoadWord(3, m, (c.S1 + 0x24u));
        c.V0 = c.V0 | 0x0002u;
        c.StoreWord(2, m, c.S1);
        c.LoadWord(2, m, (c.S0 + 0x24u));
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 7;
        c.V0 = (uint)((int)c.V0 >> 7);
        c.StoreWord(2, m, (c.S1 + 0x80u));
        c.LoadWord(2, m, (c.S0 + 0x28u));
        c.LoadWord(3, m, (c.S1 + 0x28u));
        c.A1 = c.S1 + 0x80u;
        c.V0 = c.V0 - c.V1;
        c.V0 = c.V0 << 7;
        c.V0 = (uint)((int)c.V0 >> 7);
        c.StoreWord(2, m, (c.A1 + 0x4u));
        c.LoadWord(2, m, (c.S0 + 0x2Cu));
        c.LoadWord(3, m, (c.S1 + 0x2Cu));
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
        c.A1 = c.S1 + 0x10u;
        m.WriteU16((c.A0 + 0x4u), (ushort)0u);
        m.WriteU16((c.A0 + 0x2u), (ushort)c.V0);
        c.RA = 0x80100448u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8005A78C);
        c.CopyRegister(4, 17);
        c.A1 = 0x00000080u;
        c.RA = 0x80100454u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.LoadWord(3, m, (c.S1 + 0xE0u));
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
        c.LoadWord(2, m, (c.S1 + 0xE0u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        goto L80100624;
        L801004E0: ;
        c.S0 = c.S1 + 0x24u;
        c.CopyRegister(4, 16);
        c.A2 = 0x000001FDu;
        c.V0 = 0x0000005Fu;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.V0 = 0x00000004u;
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.V0 = 0x80070000u;
        c.LoadWord(3, m, (c.V0 + 0x59FCu));
        c.V0 = 0x0000003Cu;
        c.StoreWord(2, m, (c.SP + 0x18u));
        c.LoadWord(5, m, (c.V1 + 0x58u));
        c.A3 = 0x000001FCu;
        c.RA = 0x80100518u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0108u);
        c.RA = 0x80100520u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.V0 = 0x80070000u;
        c.LoadWord(2, m, (c.V0 + 0x37E8u));
        c.A2 = 0x00000063u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 16);
        c.RA = 0x8010053Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0110u);
        c.A1 = 0xFEFF0000u;
        c.A1 = c.A1 | 0xFFFDu;
        c.A0 = (uint)(short)m.ReadU16((c.S1 + 0x14u));
        c.LoadWord(3, m, c.S1);
        c.V0 = c.A0 << 3;
        c.V0 = c.V0 + c.A0;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.A0;
        c.V0 = c.V0 << 5;
        c.V0 = c.V0 + c.A0;
        c.V1 = c.V1 & c.A1;
        c.A0 = c.S1 + 0x80u;
        if ((int)c.V0 >= 0) {
            c.StoreWord(3, m, c.S1);
            goto L80100578;
        }
        c.StoreWord(3, m, c.S1);
        c.V0 = c.V0 + 0x1Fu;
        L80100578: ;
        c.V1 = (uint)(short)m.ReadU16((c.S1 + 0x1Au));
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
        c.V1 = (uint)(short)m.ReadU16((c.S1 + 0x20u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        goto L80100624;
        L801005EC: ;
        c.LoadWord(4, m, (c.S1 + 0xE0u));
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
        c.LoadWord(2, m, c.A0);
        c.V1 = 0xFFFFFFDFu;
        m.WriteU8((c.A0 + 0x8u), (byte)0u);
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.A0);
        c.RA = 0x80100624u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80041FEC);
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
        c.V0 = c.A1 < 0x00000012u ? 1u : 0u;
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
        c.V1 = c.A1 << 2;
        c.V1 = c.V1 + c.V0;
        c.LoadWord(2, m, c.V1);
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x8010067Cu: goto L8010067C;
            case 0x80100984u: goto L80100984;
            case 0x801009B4u: goto L801009B4;
            case 0x801009C8u: goto L801009C8;
            case 0x80100974u: goto L80100974;
            case 0x801006ECu: goto L801006EC;
            case 0x801006BCu: goto L801006BC;
            case 0x80100960u: goto L80100960;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0104u);
        c.CopyRegister(2, 0);
        goto L801009CC;
        L801006EC: ;
        c.V1 = m.ReadU8((c.S0 + 0x4u));
        c.V0 = 0x00000008u;
        if (c.V1 != c.V0) {
            c.V0 = 0x00000002u;
            goto L80100760;
        }
        c.V0 = 0x00000002u;
        c.CopyRegister(4, 16);
        c.RA = 0x80100704u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CD78);
        c.CopyRegister(16, 2);
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x6u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80015B00);
        L80100758: ;
        c.V1 = m.ReadU8((c.S0 + 0x4u));
        c.V0 = 0x00000002u;
        L80100760: ;
        if (c.V1 != c.V0) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.LoadWord(2, m, c.S0);
        c.V1 = 0x02000000u;
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V0 = m.ReadU16((c.S0 + 0xCu));
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801009CC;
        }
        c.CopyRegister(2, 0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x24u));
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
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x2Cu));
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
        c.LoadWord(18, m, (c.S1 + 0xE0u));
        c.CopyRegister(7, 0);
        c.RA = 0x801007DCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8003A020);
        c.A0 = c.S1 + 0x24u;
        c.A2 = 0x000001FDu;
        c.V0 = 0x0000005Fu;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.V0 = 0x00000004u;
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.V0 = 0x80060000u;
        c.LoadWord(3, m, (c.V0 + 0x59FCu));
        c.V0 = 0x0000003Cu;
        c.StoreWord(2, m, (c.SP + 0x18u));
        c.LoadWord(5, m, (c.V1 + 0x58u));
        c.A3 = 0x000001FCu;
        c.RA = 0x80100810u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0108u);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x5u));
        c.CopyRegister(5, 0);
        c.RA = 0x8010081Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E2C8);
        c.V0 = 0x80060000u;
        c.S3 = c.V0 + 0x5A18u;
        c.CopyRegister(4, 19);
        c.A1 = 0x00000021u;
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x2C0u;
        c.StoreWord(2, m, (c.S1 + 0x64u));
        c.LoadWord(2, m, c.S1);
        c.V1 = 0x03000000u;
        c.V0 = c.V0 | c.V1;
        c.V1 = 0xFFFFFFFDu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S1);
        c.RA = 0x80100854u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
        c.CopyRegister(4, 19);
        c.A1 = 0x00000022u;
        c.S0 = 0u < c.V0 ? 1u : 0u;
        c.RA = 0x80100864u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
        if (c.V0 != 0u) {
            c.S0 = c.S0 | 0x0002u;
            goto L8010086C;
        }
        L8010086C: ;
        c.CopyRegister(4, 19);
        c.A1 = 0x00000023u;
        c.RA = 0x80100878u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
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
        m.WriteU8((c.A0 + 0xD2u), (byte)c.V1);
        c.V1 = c.A0 + 0x80u;
        m.WriteU8((c.A0 + 0x8u), (byte)0u);
        c.StoreWord(0, m, (c.A0 + 0x80u));
        c.StoreWord(2, m, (c.V1 + 0x4u));
        c.StoreWord(0, m, (c.V1 + 0x8u));
        c.RA = 0x801008D8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800327CC);
        c.CopyRegister(2, 0);
        goto L801009CC;
        L80100974: ;
        c.A0 = 0x0000008Cu;
        c.RA = 0x8010097Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CBE8);
        goto L801009CC;
        L80100984: ;
        c.A0 = 0x80060000u;
        c.A0 = c.A0 + 0x5A50u;
        c.A1 = 0x00000100u;
        c.V0 = 0x00000080u;
        c.StoreWord(2, m, c.S0);
        c.RA = 0x8010099Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
        c.A0 = 0x7F000000u;
        c.CopyRegister(5, 2);
        c.RA = 0x801009A8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004AC1C);
        c.V1 = 0x80060000u;
        c.V0 = 0u < c.V0 ? 1u : 0u;
        c.StoreWord(2, m, (c.V1 + 0x5A10u));
        L801009B4: ;
        c.RA = 0x801009BCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80034B34);
        c.CopyRegister(4, 16);
        c.A1 = 0x000000F0u;
        c.RA = 0x801009C8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
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
        c.LoadWord(2, m, (c.V0 + 0x59FCu));
        c.A0 = 0x00000080u;
        c.StoreWord(3, m, (c.V0 + 0x58u));
        c.RA = 0x80100A1Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CBE8);
        goto L80100A30;
        L80100A24: ;
        c.CopyRegister(4, 3);
        c.RA = 0x80100A2Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800520D8);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        if (c.V0 != 0u) {
            c.V0 = 0x00000001u;
            goto L80100A94;
        }
        c.V0 = 0x00000001u;
        goto L80100B70;
        L80100A84: ;
        c.CopyRegister(5, 6);
        c.RA = 0x80100A8Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
        if (c.V0 == 0u) {
            c.V0 = 0x00000001u;
            goto L80100B70;
        }
        c.V0 = 0x00000001u;
        L80100A94: ;
        m.WriteU16((c.SP + 0x16u), (ushort)c.V0);
        m.WriteU16((c.SP + 0x14u), (ushort)c.V0);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x14u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x24u));
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
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x20u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x2Cu));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800359CC);
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x14u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x24u));
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
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x20u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(3, m, (c.S0 + 0x2Cu));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800359CC);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80100BBCu: goto L80100BBC;
            case 0x80100C14u: goto L80100C14;
            case 0x80100C28u: goto L80100C28;
            case 0x80100C64u: goto L80100C64;
            case 0x80100C94u: goto L80100C94;
            case 0x80100C78u: goto L80100C78;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
        }
        L80100BBC: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.LoadWord(4, m, (c.S0 + 0x38u));
        if (c.V0 == 0u) {
            goto L80100BE4;
        }
        c.V1 = m.ReadU16((c.A0 + 0x40u));
        c.V1 = c.V1 - 0x4u;
        c.V0 = c.V1 << 16;
        c.V0 = (uint)((int)c.V0 >> 16);
        c.S1 = (int)c.V0 < -1023 ? 1u : 0u;
        goto L80100BFC;
        L80100BE4: ;
        c.V1 = m.ReadU16((c.A0 + 0x40u));
        c.V1 = c.V1 + 0x4u;
        c.V0 = c.V1 << 16;
        c.V0 = (uint)((int)c.V0 >> 16);
        c.V0 = ~(0u | c.V0);
        c.S1 = c.V0 >> 31;
        L80100BFC: ;
        m.WriteU16((c.A0 + 0x40u), (ushort)c.V1);
        c.RA = 0x80100C04u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF44);
        if (c.S1 == 0u) {
            goto L80100C94;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80100C14u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BA8);
        L80100C14: ;
        c.CopyRegister(4, 16);
        c.A1 = 0x00000258u;
        c.RA = 0x80100C20u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        goto L80100C94;
        L80100C28: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        c.CopyRegister(4, 16);
        c.V0 = c.V0 < 0x00000001u ? 1u : 0u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V0);
        c.RA = 0x80100C3Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030B78);
        c.RA = 0x80100C44u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.LoadWord(3, m, (c.S0 + 0x58u));
        c.A2 = 0x00000046u;
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.A3 = c.S0 + 0x48u;
        c.RA = 0x80100C5Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        goto L80100C94;
        L80100C64: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 6);
        c.RA = 0x80100C70u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L80100C84;
        L80100C78: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 6);
        c.RA = 0x80100C84u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
        L80100C84: ;
        if (c.V0 == 0u) {
            goto L80100C94;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80100C94u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C68);
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
        c.LoadWord(2, m, (c.S2 + 0x2Cu));
        c.A1 = c.V0 - 0x11E1u;
        c.V0 = 0x05000000u;
        c.V0 = (int)c.V0 < (int)c.A1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.StoreWord(5, m, (c.S2 + 0x2Cu));
            goto L80100D18;
        }
        c.StoreWord(5, m, (c.S2 + 0x2Cu));
        c.LoadWord(4, m, (c.S2 + 0x24u));
        c.RA = 0x80100D10u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001B750);
        c.StoreWord(2, m, (c.S2 + 0x28u));
        goto L80100E00;
        L80100D18: ;
        c.CopyRegister(4, 18);
        c.RA = 0x80100D20u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BA8);
        L80100D20: ;
        c.CopyRegister(4, 18);
        c.A1 = 0x00000E10u;
        c.LoadWord(2, m, c.A0);
        c.V1 = 0x00000003u;
        m.WriteU8((c.A0 + 0x4u), (byte)c.V1);
        c.V0 = c.V0 | 0x0122u;
        c.StoreWord(2, m, c.A0);
        c.RA = 0x80100D40u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        goto L80100E00;
        L80100D48: ;
        c.RA = 0x80100D50u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
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
        c.StoreWord(2, m, (c.A0 + 0x24u));
        c.V0 = 0x05800000u;
        c.StoreWord(2, m, (c.A0 + 0x2Cu));
        c.LoadWord(2, m, c.A0);
        c.V1 = 0xFFFFFFDDu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.A0);
        c.RA = 0x80100DA0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030B78);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0100u);
        c.S1 = c.S0 + 0x24u;
        c.RA = 0x80100DC8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.V0 = 0x80060000u;
        c.A2 = 0x0000008Eu;
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.CopyRegister(7, 17);
        c.RA = 0x80100DE0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x6u));
        if ((int)c.V0 >= 0) {
            c.CopyRegister(4, 17);
            goto L80100D18;
        }
        c.CopyRegister(4, 17);
        c.A1 = 0x00800000u;
        c.A1 = c.A1 | 0x0008u;
        c.RA = 0x80100DF8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004E414);
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
            c.A0 = c.S0 + 0x24u;
            goto L8010105C;
        }
        c.A0 = c.S0 + 0x24u;
        c.V0 = 0x0000005Fu;
        c.StoreWord(2, m, (c.SP + 0x10u));
        c.V0 = 0x00000004u;
        c.StoreWord(2, m, (c.SP + 0x14u));
        c.V0 = 0x80060000u;
        c.LoadWord(3, m, (c.V0 + 0x59FCu));
        c.V0 = 0x0000003Cu;
        c.A2 = 0x000001FDu;
        c.StoreWord(2, m, (c.SP + 0x18u));
        c.LoadWord(5, m, (c.V1 + 0x58u));
        c.A3 = 0x000001FCu;
        c.RA = 0x80100EF0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0108u);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x5u));
        c.CopyRegister(5, 0);
        c.RA = 0x80100EFCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E2C8);
        c.V0 = 0x80060000u;
        c.S3 = c.V0 + 0x5A18u;
        c.CopyRegister(4, 19);
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x2C0u;
        c.StoreWord(2, m, (c.S1 + 0x64u));
        c.LoadWord(2, m, c.S1);
        c.V1 = 0x03000000u;
        c.V0 = c.V0 | c.V1;
        c.V1 = 0xFFFFFFFDu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S1);
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x6u));
        c.A1 = 0x00000021u;
        c.V0 = c.V0 - 0x21u;
        c.V0 = c.S2 << (int)(c.V0 & 31u);
        c.V0 = ~(0u | c.V0);
        c.S2 = c.V0 & 0x0007u;
        c.RA = 0x80100F48u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
        c.CopyRegister(4, 19);
        c.A1 = 0x00000022u;
        c.S0 = 0u < c.V0 ? 1u : 0u;
        c.RA = 0x80100F58u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
        if (c.V0 != 0u) {
            c.S0 = c.S0 | 0x0002u;
            goto L80100F60;
        }
        L80100F60: ;
        c.CopyRegister(4, 19);
        c.A1 = 0x00000023u;
        c.RA = 0x80100F6Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030250);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
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
        c.V1 = c.S0 + 0x21u;
        m.WriteU8((c.S1 + 0xD2u), (byte)c.V1);
        c.V1 = c.S1 + 0x80u;
        m.WriteU8((c.S1 + 0x8u), (byte)0u);
        c.StoreWord(0, m, (c.S1 + 0x80u));
        c.StoreWord(2, m, (c.V1 + 0x4u));
        c.StoreWord(0, m, (c.V1 + 0x8u));
        c.RA = 0x80100FD4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L80101088;
        L80101070: ;
        c.CopyRegister(5, 6);
        c.RA = 0x80101078u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
        goto L80101088;
        L80101080: ;
        c.CopyRegister(4, 16);
        c.RA = 0x80101088u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DC94);
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
        c.S1 = c.S0 + 0x48u;
        c.LoadWord(2, m, (c.S0 + 0x48u));
        c.LoadWord(3, m, (c.S0 + 0x88u));
        c.A0 = c.S0 + 0x88u;
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x48u));
        c.LoadWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(3, m, (c.A0 + 0x4u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x4u));
        c.LoadWord(2, m, (c.S1 + 0x8u));
        c.LoadWord(3, m, (c.A0 + 0x8u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x8u));
        c.LoadWord(8, m, (c.S0 + 0x48u));
        c.LoadWord(9, m, (c.S0 + 0x4Cu));
        c.LoadWord(10, m, (c.S0 + 0x50u));
        c.StoreWord(8, m, (c.S0 + 0x24u));
        c.StoreWord(9, m, (c.S0 + 0x28u));
        c.StoreWord(10, m, (c.S0 + 0x2Cu));
        c.LoadWord(2, m, (c.S0 + 0x8Cu));
        c.V0 = c.V0 + 0x38u;
        c.V0 = c.V0 < 0x00000038u ? 1u : 0u;
        if (c.V0 == 0u) {
            c.A2 = 0x00000097u;
            goto L80101164;
        }
        c.A2 = 0x00000097u;
        c.V0 = 0x80060000u;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x5u));
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.CopyRegister(7, 17);
        c.RA = 0x80101150u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0110u);
        c.LoadWord(2, m, c.S0);
        c.V1 = 0xFFFFFFDFu;
        m.WriteU16((c.S0 + 0x6u), (ushort)0u);
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S0);
        L80101164: ;
        c.LoadWord(2, m, (c.S0 + 0x8Cu));
        c.CopyRegister(4, 16);
        c.V0 = c.V0 + 0x38u;
        c.StoreWord(2, m, (c.S0 + 0x8Cu));
        c.RA = 0x80101178u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002D1DC);
        c.LoadWord(4, m, (c.S0 + 0x48u));
        c.LoadWord(5, m, (c.S0 + 0x50u));
        c.RA = 0x80101188u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001B750);
        c.LoadWord(3, m, (c.S0 + 0x4Cu));
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.CopyRegister(4, 17);
            goto L80101230;
        }
        c.CopyRegister(4, 17);
        c.A1 = 0x00000030u;
        c.A2 = 0x0000012Cu;
        c.RA = 0x801011A4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004E128);
        c.CopyRegister(4, 2);
        c.A1 = c.A0 + 0x48u;
        c.A2 = c.A0 + 0x10u;
        c.RA = 0x801011B4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002D114);
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 0);
        c.RA = 0x801011C0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DE54);
        c.A2 = 0x000000A6u;
        c.V0 = 0x80060000u;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x5u));
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.CopyRegister(7, 17);
        c.RA = 0x801011D8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 16);
        c.A1 = 0x00000001u;
        c.RA = 0x801011E4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800309C8);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80039AF8);
        c.V0 = 0x000003E8u;
        m.WriteU16((c.S0 + 0xCu), (ushort)c.V0);
        c.CopyRegister(4, 16);
        L80101218: ;
        c.CopyRegister(5, 17);
        c.A2 = 0x0000000Cu;
        c.A3 = 0x000000A6u;
        c.RA = 0x80101228u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80042638);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x801014A4u: goto L801014A4;
            case 0x801012C8u: goto L801012C8;
            case 0x80101290u: goto L80101290;
            case 0x801014E8u: goto L801014E8;
            case 0x80101468u: goto L80101468;
            case 0x801012A4u: goto L801012A4;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
        }
        L80101290: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 6);
        c.RA = 0x8010129Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L801012B0;
        L801012A4: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 6);
        c.RA = 0x801012B0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
        L801012B0: ;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801014EC;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x801012C0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C68);
        c.CopyRegister(2, 0);
        goto L801014EC;
        L801012C8: ;
        c.LoadWord(2, m, c.S2);
        c.V0 = c.V0 & 0x0004u;
        if (c.V0 == 0u) {
            c.CopyRegister(4, 18);
            goto L80101444;
        }
        c.CopyRegister(4, 18);
        c.A1 = 0u | 0x8000u;
        c.RA = 0x801012E0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002C5F4);
        c.A1 = 0x0000012Bu;
        c.A2 = 0x00000098u;
        c.A3 = 0x00000008u;
        c.LoadWord(4, m, (c.S2 + 0x58u));
        c.CopyRegister(16, 2);
        c.RA = 0x801012F8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallObjectFactory(c, m, Vigilante82PC.func_8002C17C);
        c.CopyRegister(17, 2);
        c.V0 = 0x00000007u;
        m.WriteU8((c.S1 + 0x4u), (byte)c.V0);
        c.V0 = m.ReadU16((c.S2 + 0x6u));
        m.WriteU16((c.S1 + 0x6u), (ushort)c.V0);
        c.S3 = (uint)(short)m.ReadU16((c.S0 + 0x12u));
        c.S4 = c.S1 + 0x10u;
        c.RA = 0x80101318u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CEAC);
        c.CopyRegister(4, 17);
        c.V0 = 0x00000490u;
        c.LoadWord(8, m, (c.S1 + 0x24u));
        c.LoadWord(9, m, (c.S1 + 0x28u));
        c.LoadWord(10, m, (c.S1 + 0x2Cu));
        c.StoreWord(8, m, (c.S1 + 0x48u));
        c.StoreWord(9, m, (c.S1 + 0x4Cu));
        c.StoreWord(10, m, (c.S1 + 0x50u));
        m.WriteU16((c.S0 + 0x12u), (ushort)c.S3);
        c.StoreWord(2, m, c.S1);
        c.V0 = 0x0000012Cu;
        m.WriteU16((c.S1 + 0xCu), (ushort)c.V0);
        c.V0 = 0x80100000u;
        c.V0 = c.V0 + 0x10A8u;
        c.StoreWord(2, m, (c.S1 + 0x64u));
        c.RA = 0x80101388u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800305FC);
        m.WriteU16((c.S1 + 0x94u), (ushort)0u);
        c.RA = 0x80101390u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 11;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V0 = (uint)(short)m.ReadU16((c.S1 + 0x14u));
        c.S3 = c.V1 + 0x2800u;
        { var _r = (long)(int)c.V0 * (int)c.S3; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.S0 = c.S1 + 0x48u;
        c.A0 = c.LO;
        if ((int)c.A0 >= 0) {
            c.A1 = c.S1 + 0x88u;
            goto L801013C0;
        }
        c.A1 = c.S1 + 0x88u;
        c.A0 = c.A0 + 0xFFFu;
        L801013C0: ;
        c.V0 = (uint)(short)m.ReadU16((c.S1 + 0x1Au));
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
        c.V0 = (uint)(short)m.ReadU16((c.S1 + 0x20u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002A39C);
        c.CopyRegister(4, 16);
        c.A1 = 0x00000011u;
        c.RA = 0x80101414u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DE54);
        c.RA = 0x8010141Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.A2 = 0x0000008Du;
        m.WriteU8((c.S1 + 0x5u), (byte)c.V0);
        c.V0 = 0x80060000u;
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.CopyRegister(7, 16);
        c.RA = 0x8010143Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0110u);
        c.CopyRegister(2, 0);
        goto L801014EC;
        L80101444: ;
        c.CopyRegister(4, 18);
        c.RA = 0x8010144Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002C05C);
        c.CopyRegister(4, 18);
        c.RA = 0x80101454u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BF0);
        c.CopyRegister(4, 18);
        c.A1 = 0x00000021u;
        c.RA = 0x80101460u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(2, 0);
        goto L801014EC;
        L80101468: ;
        c.RA = 0x80101470u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 18);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.A1 = c.V1 << 4;
        c.A1 = c.A1 - c.V1;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x258u;
        c.RA = 0x80101494u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(4, 18);
        c.RA = 0x8010149Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C20);
        c.V0 = 0xFFFFFFFFu;
        goto L801014EC;
        L801014A4: ;
        c.RA = 0x801014ACu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 18);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.A1 = c.V1 << 4;
        c.A1 = c.A1 - c.V1;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x258u;
        c.RA = 0x801014D0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(4, 18);
        c.RA = 0x801014D8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C20);
        c.V0 = 0x8FFF196Cu;
        c.LoadWord(3, m, (c.S2 + 0x38u));
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x64u));
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
        c.LoadWord(4, m, (c.S1 + 0x38u));
        c.CopyRegister(19, 5);
        c.RA = 0x80101534u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallModelNodeFinder(c, m, Vigilante82PC.func_8002CE50);
        c.LoadWord(4, m, (c.S1 + 0x38u));
        c.CopyRegister(18, 2);
        c.RA = 0x80101540u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.SkipStaticModelDetach(c, m, Vigilante82PC.func_8002CCBC);
        c.CopyRegister(4, 2);
        c.A1 = 0x00000098u;
        c.RA = 0x8010154Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallObjectReallocator(c, m, Vigilante82PC.func_80052188);
        c.CopyRegister(16, 2);
        c.V0 = m.ReadU16((c.S1 + 0x6u));
        c.CopyRegister(4, 16);
        m.WriteU16((c.S0 + 0x6u), (ushort)c.V0);
        c.V0 = 0x00000007u;
        c.LoadWord(3, m, c.S2);
        c.LoadWord(6, m, (c.S2 + 0x4u));
        c.LoadWord(7, m, (c.S2 + 0x8u));
        c.LoadWord(8, m, (c.S2 + 0xCu));
        c.StoreWord(3, m, (c.S0 + 0x10u));
        c.StoreWord(6, m, (c.S0 + 0x14u));
        c.StoreWord(7, m, (c.S0 + 0x18u));
        c.StoreWord(8, m, (c.S0 + 0x1Cu));
        c.LoadWord(3, m, (c.S2 + 0x10u));
        c.LoadWord(6, m, (c.S2 + 0x14u));
        c.LoadWord(7, m, (c.S2 + 0x18u));
        c.LoadWord(8, m, (c.S2 + 0x1Cu));
        c.StoreWord(3, m, (c.S0 + 0x20u));
        c.StoreWord(6, m, (c.S0 + 0x24u));
        c.StoreWord(7, m, (c.S0 + 0x28u));
        c.StoreWord(8, m, (c.S0 + 0x2Cu));
        c.LoadWord(3, m, (c.S2 + 0x14u));
        c.LoadWord(6, m, (c.S2 + 0x18u));
        c.LoadWord(7, m, (c.S2 + 0x1Cu));
        c.StoreWord(3, m, (c.S0 + 0x48u));
        c.StoreWord(6, m, (c.S0 + 0x4Cu));
        c.StoreWord(7, m, (c.S0 + 0x50u));
        m.WriteU8((c.S0 + 0x4u), (byte)c.V0);
        c.V0 = 0x00000080u;
        c.StoreWord(2, m, c.S0);
        c.StoreWord(19, m, (c.S0 + 0x64u));
        c.RA = 0x801015CCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800305FC);
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
        c.S0 = c.S1 + 0x48u;
        c.LoadWord(2, m, (c.S1 + 0x48u));
        c.LoadWord(3, m, (c.S1 + 0x88u));
        c.A0 = c.S1 + 0x88u;
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S1 + 0x48u));
        c.LoadWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(3, m, (c.A0 + 0x4u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x4u));
        c.LoadWord(2, m, (c.S0 + 0x8u));
        c.LoadWord(3, m, (c.A0 + 0x8u));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x8u));
        c.LoadWord(9, m, (c.S1 + 0x48u));
        c.LoadWord(10, m, (c.S1 + 0x4Cu));
        c.LoadWord(11, m, (c.S1 + 0x50u));
        c.StoreWord(9, m, (c.S1 + 0x24u));
        c.StoreWord(10, m, (c.S1 + 0x28u));
        c.StoreWord(11, m, (c.S1 + 0x2Cu));
        c.LoadWord(2, m, (c.S1 + 0x8Cu));
        c.V0 = c.V0 + 0x38u;
        c.V0 = c.V0 < 0x00000038u ? 1u : 0u;
        if (c.V0 == 0u) {
            goto L801016AC;
        }
        c.RA = 0x80101684u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.A2 = 0x00000097u;
        c.V0 = 0x80060000u;
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.CopyRegister(7, 16);
        c.RA = 0x8010169Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0110u);
        c.LoadWord(2, m, c.S1);
        c.V1 = 0xFFFFFFDFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S1);
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
        c.A0 = c.S1 + 0x10u;
        m.WriteU16((c.A1 + 0x4u), (ushort)c.V0);
        c.RA = 0x801016DCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002A5EC);
        c.LoadWord(5, m, (c.S1 + 0x84u));
        if (c.A1 == 0u) {
            c.T0 = c.SP + 0x18u;
            goto L8010181C;
        }
        c.T0 = c.SP + 0x18u;
        c.LoadWord(3, m, (c.A1 + 0x48u));
        c.A1 = c.A1 + 0x48u;
        c.LoadWord(2, m, (c.S1 + 0x48u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002ABC4);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002ABC4);
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
            c.A0 = c.S1 + 0x48u;
            goto L80101864;
        }
        c.A0 = c.S1 + 0x48u;
        c.A1 = 0x00000013u;
        c.RA = 0x8010183Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DE54);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF74);
        L80101864: ;
        c.LoadWord(4, m, (c.S1 + 0x48u));
        c.LoadWord(5, m, (c.S1 + 0x50u));
        c.RA = 0x80101874u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001B750);
        c.LoadWord(3, m, (c.S1 + 0x4Cu));
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.S0 = c.S1 + 0x48u;
            goto L801018EC;
        }
        c.S0 = c.S1 + 0x48u;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 0);
        c.RA = 0x80101890u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DE54);
        c.RA = 0x80101898u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.V0 = 0x80060000u;
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.A2 = 0x000000A6u;
        c.CopyRegister(7, 16);
        c.RA = 0x801018B0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 17);
        c.A1 = 0x00000001u;
        c.RA = 0x801018BCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800309C8);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80042638);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80101B00u: goto L80101B00;
            case 0x80101AB8u: goto L80101AB8;
            case 0x80101950u: goto L80101950;
            case 0x80101B58u: goto L80101B58;
            case 0x80101AA8u: goto L80101AA8;
            case 0x80101B34u: goto L80101B34;
            case 0x8010196Cu: goto L8010196C;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
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
            c.A0 = c.S1 + 0x48u;
            goto L801019A0;
        }
        c.A0 = c.S1 + 0x48u;
        c.CopyRegister(5, 18);
        L801019A0: ;
        c.RA = 0x801019A8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF010Cu);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80029F6C);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S1 + 0x58u));
        c.A2 = 0x0000004Fu;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S0 + 0x24u;
        c.RA = 0x80101A74u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.RA = 0x80101A7Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 17);
        c.A1 = c.V0 << 3;
        c.A1 = c.A1 - c.V0;
        c.A1 = c.A1 << 5;
        c.A1 = c.A1 + c.V0;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x708u;
        c.RA = 0x80101AA0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(2, 0);
        goto L80101B5C;
        L80101AA8: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80101AB0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C20);
        c.V0 = 0xFFFFFFFFu;
        goto L80101B5C;
        L80101AB8: ;
        c.CopyRegister(4, 17);
        c.A2 = 0x000001A4u;
        c.LoadWord(5, m, (c.S1 + 0x58u));
        c.A3 = 0x00000008u;
        c.RA = 0x80101ACCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallObjectModelFactory(c, m, Vigilante82PC.func_8002C344);
        c.CopyRegister(4, 17);
        c.RA = 0x80101AD4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002D1DC);
        c.CopyRegister(4, 17);
        c.RA = 0x80101ADCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002C7D0);
        c.CopyRegister(4, 17);
        c.RA = 0x80101AE4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BF0);
        c.LoadWord(2, m, c.S1);
        c.V1 = 0xFFFFFFDFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S1);
        c.LoadWord(3, m, (c.S1 + 0x38u));
        c.V0 = 0x8FFF196Cu;
        goto L80101B0C;
        L80101B00: ;
        c.LoadWord(3, m, (c.S1 + 0x38u));
        if (c.V1 == 0u) {
            c.V0 = 0x8FFF196Cu;
            goto L80101B18;
        }
        c.V0 = 0x8FFF196Cu;
        L80101B0C: ;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x64u));
        goto L80101B58;
        L80101B18: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80101B20u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C20);
        c.CopyRegister(4, 17);
        c.A1 = 0x0000012Cu;
        c.RA = 0x80101B2Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(2, 0);
        goto L80101B5C;
        L80101B34: ;
        c.RA = 0x80101B3Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V0 = c.V0 & 0x0001u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80101B5C;
        }
        c.CopyRegister(2, 0);
        c.A0 = 0x00000080u;
        c.RA = 0x80101B50u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CBE8);
        c.StoreWord(17, m, (c.V0 + 0x58u));
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
        c.S0 = c.S1 + 0x48u;
        c.V0 = m.ReadU16((c.S1 + 0x42u));
        c.LoadWord(3, m, (c.S1 + 0x48u));
        c.LoadWord(4, m, (c.S1 + 0x88u));
        c.V0 = c.V0 + 0x88u;
        c.V1 = c.V1 + c.A0;
        c.A0 = c.S1 + 0x88u;
        m.WriteU16((c.S1 + 0x42u), (ushort)c.V0);
        c.StoreWord(3, m, (c.S1 + 0x48u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF74);
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
        c.LoadWord(2, m, (c.S1 + 0x48u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002ABC4);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002ABC4);
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
        c.LoadWord(4, m, (c.S1 + 0x48u));
        c.LoadWord(5, m, (c.S1 + 0x50u));
        c.RA = 0x80101D70u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001B750);
        c.LoadWord(3, m, (c.S1 + 0x4Cu));
        c.V0 = (int)c.V0 < (int)c.V1 ? 1u : 0u;
        if (c.V0 == 0u) {
            c.S0 = c.S1 + 0x48u;
            goto L80101DBC;
        }
        c.S0 = c.S1 + 0x48u;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 0);
        c.RA = 0x80101D8Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DE54);
        c.A2 = 0x000000A6u;
        c.V0 = 0x80060000u;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x5u));
        c.LoadWord(5, m, (c.V0 + 0x5F8u));
        c.CopyRegister(7, 16);
        c.RA = 0x80101DA4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 17);
        c.A1 = 0x00000001u;
        m.WriteU8((c.A0 + 0x5u), (byte)0u);
        c.RA = 0x80101DB4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800309C8);
        c.V0 = 0xFFFFFFFFu;
        goto L80101E18;
        L80101DBC: ;
        if (c.S2 == 0u) {
            c.CopyRegister(2, 0);
            goto L80101E18;
        }
        c.CopyRegister(2, 0);
        c.A0 = c.S1 + 0x24u;
        c.RA = 0x80101DCCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E478);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x5u));
        c.CopyRegister(5, 2);
        c.RA = 0x80101DD8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E2C8);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80042638);
        goto L80101E18;
        L80101E08: ;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S1 + 0x5u));
        c.RA = 0x80101E14u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DE78);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80102054u: goto L80102054;
            case 0x80102014u: goto L80102014;
            case 0x80101E7Cu: goto L80101E7C;
            case 0x801020CCu: goto L801020CC;
            case 0x80102004u: goto L80102004;
            case 0x801020A8u: goto L801020A8;
            case 0x80101E98u: goto L80101E98;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
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
            c.A0 = c.S1 + 0x48u;
            goto L80101ECC;
        }
        c.A0 = c.S1 + 0x48u;
        c.CopyRegister(5, 18);
        L80101ECC: ;
        c.RA = 0x80101ED4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF010Cu);
        c.A0 = c.S0 + 0x24u;
        c.StoreWord(2, m, (c.S0 + 0x84u));
        c.CopyRegister(5, 2);
        c.V0 = 0x00000064u;
        c.A1 = c.A1 + 0x24u;
        m.WriteU16((c.S0 + 0xCu), (ushort)c.V0);
        c.RA = 0x80101EF0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80029F6C);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S1 + 0x58u));
        c.A2 = 0x0000004Fu;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S0 + 0x24u;
        c.RA = 0x80101FA8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.RA = 0x80101FB0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        m.WriteU8((c.S0 + 0x5u), (byte)c.V0);
        c.LoadWord(2, m, (c.S1 + 0x58u));
        c.A2 = 0x00000049u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x80101FD0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0118u);
        c.RA = 0x80101FD8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 17);
        c.A1 = c.V0 << 3;
        c.A1 = c.A1 - c.V0;
        c.A1 = c.A1 << 5;
        c.A1 = c.A1 + c.V0;
        c.A1 = c.A1 << 3;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x708u;
        c.RA = 0x80101FFCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(2, 0);
        goto L801020D0;
        L80102004: ;
        c.CopyRegister(4, 17);
        c.RA = 0x8010200Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C20);
        c.V0 = 0xFFFFFFFFu;
        goto L801020D0;
        L80102014: ;
        c.CopyRegister(4, 17);
        c.A2 = 0x000001AFu;
        c.LoadWord(5, m, (c.S1 + 0x58u));
        c.A3 = 0x00000008u;
        c.RA = 0x80102028u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallObjectModelFactory(c, m, Vigilante82PC.func_8002C344);
        c.CopyRegister(4, 17);
        c.RA = 0x80102030u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002D1DC);
        c.CopyRegister(4, 17);
        c.RA = 0x80102038u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BF0);
        c.LoadWord(2, m, c.S1);
        c.V1 = 0xFFFFFFDFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S1);
        c.LoadWord(3, m, (c.S1 + 0x38u));
        c.V0 = 0x8FFF196Cu;
        goto L80102060;
        L80102054: ;
        c.LoadWord(3, m, (c.S1 + 0x38u));
        if (c.V1 == 0u) {
            c.V0 = 0x8FFF196Cu;
            goto L8010206C;
        }
        c.V0 = 0x8FFF196Cu;
        L80102060: ;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x64u));
        goto L801020CC;
        L8010206C: ;
        c.CopyRegister(4, 17);
        c.RA = 0x80102074u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030C20);
        c.RA = 0x8010207Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 17);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.A1 = c.V1 << 4;
        c.A1 = c.A1 - c.V1;
        c.A1 = c.A1 << 2;
        c.A1 = (uint)((int)c.A1 >> 15);
        c.A1 = c.A1 + 0x12Cu;
        c.RA = 0x801020A0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
        c.CopyRegister(2, 0);
        goto L801020D0;
        L801020A8: ;
        c.RA = 0x801020B0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V0 = c.V0 & 0x0001u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L801020D0;
        }
        c.CopyRegister(2, 0);
        c.A0 = 0x00000080u;
        c.RA = 0x801020C4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CBE8);
        c.StoreWord(17, m, (c.V0 + 0x58u));
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
        c.A1 = c.V1 + 0x7B4u;
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
        c.LoadWord(3, m, (c.S0 + 0x48u));
        c.V0 = m.ReadU16((c.S0 + 0x86u));
        c.V1 = c.V1 + c.A0;
        c.V0 = c.V0 & 0x0FFFu;
        c.V0 = c.V0 << 2;
        c.V0 = c.V0 + c.A1;
        c.StoreWord(3, m, (c.S0 + 0x24u));
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
        c.LoadWord(2, m, (c.S0 + 0x50u));
        c.LoadWord(5, m, (c.S0 + 0x8Cu));
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x2Cu));
        c.LoadWord(2, m, (c.S0 + 0x28u));
        c.V1 = m.ReadU16((c.S0 + 0x86u));
        c.V0 = c.V0 + c.A1;
        c.StoreWord(2, m, (c.S0 + 0x28u));
        c.LoadWord(2, m, (c.S0 + 0x98u));
        c.V1 = c.V1 + c.A0;
        c.LoadWord(4, m, (c.S0 + 0x9Cu));
        m.WriteU16((c.S0 + 0x86u), (ushort)c.V1);
        c.V1 = c.V1 + 0x400u;
        m.WriteU16((c.S0 + 0x42u), (ushort)c.V1);
        c.V0 = c.V0 + c.A0;
        if (c.A2 == 0u) {
            c.StoreWord(2, m, (c.S0 + 0x98u));
            goto L801021CC;
        }
        c.StoreWord(2, m, (c.S0 + 0x98u));
        c.CopyRegister(4, 16);
        c.RA = 0x801021CCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF44);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800309A0);
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
        if (RecompOne.Runtime.Sdk.V82DreamlandCompat.PreserveActorModel(c, m)) return;
        c.SP = c.SP - 0x20u;
        c.StoreWord(16, m, (c.SP + 0x10u));
        c.CopyRegister(16, 4);
        c.StoreWord(31, m, (c.SP + 0x18u));
        c.StoreWord(17, m, (c.SP + 0x14u));
        c.LoadWord(4, m, (c.S0 + 0x30u));
        c.CopyRegister(17, 5);
        c.RA = 0x80102220u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001FEB8);
        c.LoadWord(4, m, (c.S0 + 0x38u));
        c.RA = 0x8010222Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002C4B4);
        c.CopyRegister(4, 16);
        c.A2 = c.S1 & 0xFFFFu;
        c.LoadWord(5, m, (c.A0 + 0x58u));
        c.A3 = 0x00000008u;
        c.RA = 0x80102240u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallObjectModelFactory(c, m, Vigilante82PC.func_8002C344);
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
        c.LoadWord(2, m, (c.FP + 0x59FCu));
        c.A3 = 0x00000008u;
        c.LoadWord(4, m, (c.V0 + 0x58u));
        c.S1 = c.S1 + 0x1u;
        c.RA = 0x801022C0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallObjectFactory(c, m, Vigilante82PC.func_8002C17C);
        c.CopyRegister(16, 2);
        c.LoadWord(8, m, (c.S2 + 0x48u));
        c.LoadWord(9, m, (c.S2 + 0x4Cu));
        c.LoadWord(10, m, (c.S2 + 0x50u));
        c.StoreWord(8, m, (c.S0 + 0x48u));
        c.StoreWord(9, m, (c.S0 + 0x4Cu));
        c.StoreWord(10, m, (c.S0 + 0x50u));
        c.StoreWord(23, m, c.S0);
        c.T3 = c.S6 + 0x1E0u;
        c.T0 = m.ReadWordLeft(c.T0, c.T3);
        c.T0 = m.ReadWordRight(c.T0, (c.T3 + 0x3u));
        c.T1 = (uint)(short)m.ReadU16((c.T3 + 0x4u));
        m.WriteWordLeft((c.S0 + 0x80u), c.T0);
        m.WriteWordRight((c.S0 + 0x83u), c.T0);
        m.WriteU16((c.S0 + 0x84u), (ushort)c.T1);
        c.RA = 0x80102304u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        m.WriteU16((c.S0 + 0x86u), (ushort)c.V0);
        c.StoreWord(21, m, (c.S0 + 0x9Cu));
        c.RA = 0x80102310u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 16);
        c.V0 = c.V0 << 10;
        c.V0 = 0u - c.V0;
        c.V0 = (uint)((int)c.V0 >> 15);
        c.V0 = c.V0 - 0x400u;
        c.StoreWord(2, m, (c.A0 + 0x8Cu));
        c.StoreWord(20, m, (c.A0 + 0x64u));
        c.StoreWord(19, m, (c.A0 + 0x94u));
        c.RA = 0x80102334u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8003066C);
        c.V0 = (int)c.S1 < 3 ? 1u : 0u;
        if (c.V0 != 0u) {
            c.A1 = 0x000001F6u;
            goto L801022A8;
        }
        c.S0 = 0x80100000u;
        c.RA = 0x80102348u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(17, 2);
        c.S0 = c.S0 + 0x3210u;
        c.RA = 0x80102354u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 17);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 >> 15;
        c.V1 = c.V1 << 1;
        c.V1 = c.V1 + c.S0;
        c.S0 = c.S2 + 0x24u;
        c.LoadWord(2, m, (c.S2 + 0x58u));
        c.A2 = m.ReadU16(c.V1);
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 16);
        c.RA = 0x80102384u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 16);
        c.A1 = 0x00000011u;
        c.RA = 0x80102390u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004DE54);
        c.CopyRegister(4, 18);
        c.RA = 0x80102398u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800309A0);
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
        c.S1 = c.S0 + 0x48u;
        c.CopyRegister(4, 17);
        c.StoreWord(18, m, (c.SP + 0x38u));
        c.StoreWord(31, m, (c.SP + 0x3Cu));
        c.CopyRegister(18, 6);
        c.RA = 0x801023F0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800518DC);
        c.CopyRegister(5, 17);
        c.StoreWord(2, m, (c.S0 + 0x80u));
        c.CopyRegister(4, 2);
        c.V0 = 0x00000001u;
        c.A2 = c.SP + 0x10u;
        c.StoreWord(2, m, (c.S0 + 0x88u));
        c.RA = 0x8010240Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80051334);
        c.LoadWord(3, m, c.S0);
        c.V0 = c.V0 << 16;
        c.StoreWord(2, m, (c.S0 + 0x8Cu));
        c.StoreWord(18, m, (c.S0 + 0x84u));
        c.V1 = c.V1 | 0x0180u;
        c.StoreWord(3, m, c.S0);
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
        c.A2 = c.S0 + 0x24u;
        c.LoadWord(4, m, (c.S0 + 0x80u));
        c.A3 = c.SP + 0x10u;
        c.RA = 0x80102478u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800285E4);
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
        c.LoadWord(4, m, (c.S0 + 0x24u));
        c.LoadWord(5, m, (c.S0 + 0x2Cu));
        c.CopyRegister(6, 17);
        c.RA = 0x801024ACu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001B998);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800598D8);
        c.CopyRegister(4, 17);
        c.CopyRegister(5, 4);
        c.RA = 0x801024FCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8005991C);
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
        m.WriteU16((c.S0 + 0x10u), (ushort)c.V0);
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
        m.WriteU16((c.S0 + 0x16u), (ushort)c.V0);
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
        m.WriteU16((c.S0 + 0x1Cu), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x20u));
        c.V0 = 0u - c.V0;
        m.WriteU16((c.S0 + 0x12u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x22u));
        c.V0 = 0u - c.V0;
        m.WriteU16((c.S0 + 0x18u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x24u));
        c.V0 = 0u - c.V0;
        m.WriteU16((c.S0 + 0x1Eu), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x28u));
        m.WriteU16((c.S0 + 0x14u), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x2Au));
        m.WriteU16((c.S0 + 0x1Au), (ushort)c.V0);
        c.V0 = m.ReadU16((c.SP + 0x2Cu));
        m.WriteU16((c.S0 + 0x20u), (ushort)c.V0);
        c.LoadWord(2, m, (c.SP + 0x10u));
        { var _r = (long)(int)c.V0 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.V1 = c.LO;
        c.LoadWord(2, m, (c.SP + 0x18u));
        { var _r = (long)(int)c.V0 * (int)c.V0; c.LO = (uint)_r; c.HI = (uint)(_r >> 32); }
        c.T1 = c.LO;
        c.A0 = c.V1 + c.T1;
        c.RA = 0x80102604u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800597BC);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_800512A8);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x801026D0u: goto L801026D0;
            case 0x80102814u: goto L80102814;
            case 0x80102864u: goto L80102864;
            case 0x801026FCu: goto L801026FC;
            case 0x8010280Cu: goto L8010280C;
            case 0x801027E8u: goto L801027E8;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
        }
        L801026D0: ;
        c.CopyRegister(4, 18);
        c.RA = 0x801026D8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8010243Cu);
        if (c.S0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102868;
        }
        c.CopyRegister(2, 0);
        c.A0 = c.S2 + 0x24u;
        c.RA = 0x801026E8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E7A8);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x5u));
        c.CopyRegister(5, 2);
        c.RA = 0x801026F4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E2C8);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002FB70);
        c.A0 = c.S2 + 0x10u;
        c.A1 = c.SP + 0x30u;
        c.A2 = c.SP + 0x48u;
        c.RA = 0x8010272Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80024238);
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
        c.StoreWord(3, m, (c.SP + 0x58u));
        c.V0 = c.V0 << 6;
        c.StoreWord(2, m, (c.SP + 0x54u));
        c.RA = 0x80102778u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002B1FC);
        c.CopyRegister(4, 16);
        c.A1 = 0xFFFFFFCEu;
        c.CopyRegister(6, 17);
        c.A3 = 0x00000001u;
        c.RA = 0x8010278Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8003A020);
        c.A0 = c.S0 + 0x10u;
        c.CopyRegister(5, 17);
        c.S0 = c.SP + 0x60u;
        c.CopyRegister(6, 16);
        c.RA = 0x801027A0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80024148);
        c.CopyRegister(4, 16);
        c.A1 = 0x80800000u;
        c.A1 = c.A1 | 0x8008u;
        c.RA = 0x801027B0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004E414);
        c.RA = 0x801027B8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S2 + 0x58u));
        c.A2 = 0x00000045u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 16);
        c.RA = 0x801027D0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(2, 0);
        goto L80102868;
        L801027D8: ;
        c.CopyRegister(5, 16);
        c.RA = 0x801027E0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L801027F4;
        L801027E8: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 16);
        c.RA = 0x801027F4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.LoadWord(3, m, (c.S2 + 0x58u));
        c.A2 = 0x00000050u;
        m.WriteU8((c.S2 + 0x5u), (byte)c.V0);
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x8010284Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0118u);
        c.LoadWord(2, m, c.S2);
        c.V0 = c.V0 & 0x0008u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102868;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x80102864u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004C9C8);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x801028C4u: goto L801028C4;
            case 0x80102AB0u: goto L80102AB0;
            case 0x8010295Cu: goto L8010295C;
            case 0x801029F0u: goto L801029F0;
            case 0x80102B18u: goto L80102B18;
            case 0x80102B30u: goto L80102B30;
            case 0x80102AA8u: goto L80102AA8;
            case 0x80102A84u: goto L80102A84;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
        }
        L801028C4: ;
        if (c.S3 == 0u) {
            goto L80102918;
        }
        c.A0 = c.S2 + 0x24u;
        c.RA = 0x801028D4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E7A8);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x5u));
        c.CopyRegister(5, 2);
        c.RA = 0x801028E0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E2C8);
        c.LoadWord(3, m, c.S2);
        c.V0 = 0x00020000u;
        c.V0 = c.V1 & c.V0;
        if (c.V0 != 0u) {
            c.V1 = 0xFFFD0000u;
            goto L80102908;
        }
        c.V0 = 0xFFFE0000u;
        c.V0 = c.V0 | 0xFFFFu;
        c.V0 = c.V1 & c.V0;
        c.StoreWord(2, m, c.S2);
        c.V1 = 0xFFFD0000u;
        L80102908: ;
        c.LoadWord(2, m, c.S2);
        c.V1 = c.V1 | 0xFFFFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S2);
        L80102918: ;
        c.CopyRegister(4, 18);
        c.RA = 0x80102920u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8010243Cu);
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x5u));
        m.WriteU8((c.S2 + 0x8u), (byte)0u);
        c.RA = 0x80102934u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DE78);
        c.CopyRegister(4, 18);
        c.RA = 0x8010293Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BA8);
        c.CopyRegister(4, 18);
        c.A1 = 0x000000F0u;
        c.RA = 0x80102948u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.LoadWord(3, m, (c.S2 + 0x58u));
        c.A2 = 0x00000050u;
        m.WriteU8((c.S2 + 0x5u), (byte)c.V0);
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x801029A0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0118u);
        c.CopyRegister(4, 18);
        c.A1 = 0x000001B8u;
        c.RA = 0x801029ACu;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x801029B4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030B78);
        c.CopyRegister(4, 18);
        goto L801029E0;
        L801029BC: ;
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x5u));
        c.RA = 0x801029C8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DE78);
        c.CopyRegister(4, 18);
        c.A1 = 0x000001CAu;
        c.RA = 0x801029D4u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.CopyRegister(4, 18);
        c.RA = 0x801029DCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030BA8);
        c.CopyRegister(4, 18);
        L801029E0: ;
        c.A1 = 0x000000F0u;
        c.RA = 0x801029E8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80030CB0);
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
        c.LoadWord(2, m, c.S2);
        c.V1 = 0x00010000u;
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            goto L80102A60;
        }
        c.S0 = 0x80100000u;
        c.RA = 0x80102A28u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(17, 2);
        c.S0 = c.S0 + 0x3210u;
        c.RA = 0x80102A34u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.CopyRegister(4, 17);
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 >> 15;
        c.V1 = c.V1 << 1;
        c.V1 = c.V1 + c.S0;
        c.LoadWord(2, m, (c.S2 + 0x58u));
        c.A2 = m.ReadU16(c.V1);
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S2 + 0x24u;
        c.RA = 0x80102A60u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        L80102A60: ;
        c.LoadWord(2, m, c.S2);
        c.V1 = 0x00030000u;
        c.V0 = c.V0 | c.V1;
        c.StoreWord(2, m, c.S2);
        c.CopyRegister(4, 18);
        L80102A74: ;
        c.CopyRegister(5, 19);
        c.RA = 0x80102A7Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L80102A90;
        L80102A84: ;
        c.CopyRegister(4, 18);
        c.CopyRegister(5, 19);
        c.RA = 0x80102A90u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.A0 = c.V0 << 24;
        c.A0 = (uint)((int)c.A0 >> 24);
        c.LoadWord(3, m, (c.S2 + 0x58u));
        c.A2 = 0x00000050u;
        m.WriteU8((c.S2 + 0x5u), (byte)c.V0);
        c.LoadWord(5, m, (c.V1 + 0x8u));
        c.CopyRegister(7, 0);
        c.RA = 0x80102AE8u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0118u);
        c.LoadWord(2, m, c.S2);
        c.V1 = 0x00000001u;
        m.WriteU8((c.S2 + 0x8u), (byte)c.V1);
        c.V0 = c.V0 | 0x0080u;
        c.StoreWord(2, m, c.S2);
        c.V0 = c.V0 & 0x0008u;
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.CopyRegister(4, 18);
        c.RA = 0x80102B10u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004C9C8);
        c.CopyRegister(2, 0);
        goto L80102B34;
        L80102B18: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x8u));
        if (c.V0 == 0u) {
            c.CopyRegister(2, 0);
            goto L80102B34;
        }
        c.CopyRegister(2, 0);
        c.A0 = (uint)(sbyte)m.ReadU8((c.S2 + 0x5u));
        c.RA = 0x80102B30u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DE78);
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80102B90u: goto L80102B90;
            case 0x80102F98u: goto L80102F98;
            case 0x80102FC4u: goto L80102FC4;
            case 0x80102D70u: goto L80102D70;
            case 0x80102E88u: goto L80102E88;
            case 0x80102E64u: goto L80102E64;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
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
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x14u));
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
        c.V0 = (uint)(short)m.ReadU16((c.S0 + 0x20u));
        c.V1 = c.V0 << 1;
        c.V1 = c.V1 + c.V0;
        c.V0 = c.V1 << 7;
        c.V0 = c.V0 - c.V1;
        c.T0 = c.V0 << 1;
        c.LoadWord(2, m, (c.S0 + 0x24u));
        c.V1 = (uint)((int)c.A0 >> 12);
        c.V0 = c.V0 + c.V1;
        if ((int)c.T0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x24u));
            goto L80102C08;
        }
        c.StoreWord(2, m, (c.S0 + 0x24u));
        c.T0 = c.T0 + 0xFFFu;
        L80102C08: ;
        c.CopyRegister(4, 16);
        c.S1 = c.S0 + 0x24u;
        c.CopyRegister(5, 17);
        c.A2 = c.SP + 0x10u;
        c.CopyRegister(7, 0);
        c.LoadWord(2, m, (c.S0 + 0x2Cu));
        c.V1 = (uint)((int)c.T0 >> 12);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x2Cu));
        c.RA = 0x80102C30u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CFBC);
        c.StoreWord(2, m, (c.S0 + 0x28u));
        c.RA = 0x80102C38u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V0 = c.V0 & 0x007Fu;
        if (c.V0 != 0u) {
            c.A1 = 0x00000047u;
            goto L80102D34;
        }
        c.A1 = 0x00000047u;
        c.LoadWord(2, m, (c.S0 + 0x58u));
        c.LoadWord(4, m, (c.V0 + 0x8u));
        c.CopyRegister(6, 17);
        c.RA = 0x80102C54u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001E8B0);
        goto L80102D34;
        L80102C5C: ;
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x14u));
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
        c.V1 = (uint)(short)m.ReadU16((c.S0 + 0x20u));
        c.V0 = c.V1 << 3;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 << 4;
        c.V0 = c.V0 - c.V1;
        c.T0 = c.V0 << 4;
        c.LoadWord(2, m, (c.S0 + 0x24u));
        c.V1 = (uint)((int)c.A0 >> 12);
        c.V0 = c.V0 + c.V1;
        if ((int)c.T0 >= 0) {
            c.StoreWord(2, m, (c.S0 + 0x24u));
            goto L80102CAC;
        }
        c.StoreWord(2, m, (c.S0 + 0x24u));
        c.T0 = c.T0 + 0xFFFu;
        L80102CAC: ;
        c.CopyRegister(4, 16);
        c.A1 = c.S0 + 0x24u;
        c.A2 = c.SP + 0x10u;
        c.CopyRegister(7, 0);
        c.LoadWord(2, m, (c.S0 + 0x2Cu));
        c.V1 = (uint)((int)c.T0 >> 12);
        c.V0 = c.V0 + c.V1;
        c.StoreWord(2, m, (c.S0 + 0x2Cu));
        c.RA = 0x80102CD0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CFBC);
        c.LoadWord(3, m, (c.S0 + 0x28u));
        c.LoadWord(4, m, (c.S0 + 0x4Cu));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V0 = c.V0 << 2;
        c.V0 = (uint)((int)c.V0 >> 15);
        c.V0 = c.V0 + 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V0);
        c.V0 = 0x8FFF196Cu;
        c.LoadWord(3, m, (c.S0 + 0x38u));
        c.V0 = c.V0 - 0x184Cu;
        m.WriteU8((c.S0 + 0x8u), (byte)c.S2);
        c.StoreWord(2, m, (c.V1 + 0x64u));
        goto L80102D30;
        L80102D2C: ;
        c.StoreWord(2, m, (c.S0 + 0x4Cu));
        L80102D30: ;
        c.StoreWord(17, m, (c.S0 + 0x28u));
        L80102D34: ;
        c.LoadWord(3, m, c.S0);
        c.V0 = 0x00020000u;
        c.V0 = c.V1 & c.V0;
        if (c.V0 != 0u) {
            c.V1 = 0xFFFD0000u;
            goto L80102D5C;
        }
        c.V0 = 0xFFFE0000u;
        c.V0 = c.V0 | 0xFFFFu;
        c.V0 = c.V1 & c.V0;
        c.StoreWord(2, m, c.S0);
        c.V1 = 0xFFFD0000u;
        L80102D5C: ;
        c.LoadWord(2, m, c.S0);
        c.V1 = c.V1 | 0xFFFFu;
        c.V0 = c.V0 & c.V1;
        c.StoreWord(2, m, c.S0);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S0 + 0x58u));
        c.A2 = 0x00000048u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.A3 = c.S0 + 0x24u;
        c.RA = 0x80102DBCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001EEu;
        c.RA = 0x80102DC8u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.V0 = 0xFFFFF415u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.S1);
        c.StoreWord(2, m, (c.S0 + 0x4Cu));
        goto L80102E50;
        L80102DD8: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        if (c.V0 == 0u) {
            c.V1 = 0x00010000u;
            goto L80102E50;
        }
        c.V1 = 0x00010000u;
        c.LoadWord(2, m, c.S0);
        c.V0 = c.V0 & c.V1;
        if (c.V0 != 0u) {
            c.CopyRegister(4, 16);
            goto L80102E54;
        }
        c.CopyRegister(4, 16);
        c.RA = 0x80102DFCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.LoadWord(3, m, c.S0);
        c.A0 = 0x00030000u;
        c.V1 = c.V1 | c.A0;
        c.StoreWord(3, m, c.S0);
        c.V1 = c.V0 << 2;
        c.V1 = c.V1 + c.V0;
        c.V1 = c.V1 << 2;
        c.V1 = c.V1 + c.V0;
        c.V0 = c.V1 << 6;
        c.V1 = c.V1 + c.V0;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V0 = (int)c.V1 < 682 ? 1u : 0u;
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x42u));
        if (c.V0 != 0u) {
            c.V0 = c.A0 - 0x2AAu;
            goto L80102E3C;
        }
        c.V0 = c.A0 - 0x2AAu;
        c.V0 = c.A0 + 0x2AAu;
        L80102E3C: ;
        c.V0 = c.V0 + c.V1;
        c.V0 = c.V0 & 0x0FFFu;
        m.WriteU16((c.S0 + 0x42u), (ushort)c.V0);
        c.CopyRegister(4, 16);
        c.RA = 0x80102E50u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF44);
        L80102E50: ;
        c.CopyRegister(4, 16);
        L80102E54: ;
        c.CopyRegister(5, 18);
        c.RA = 0x80102E5Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L80102E70;
        L80102E64: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 18);
        c.RA = 0x80102E70u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V0 = c.V0 << 10;
        c.V1 = (uint)((int)c.V0 >> 15);
        c.A0 = (uint)(short)m.ReadU16((c.S0 + 0x42u));
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
        m.WriteU16((c.S0 + 0x42u), (ushort)c.V0);
        c.CopyRegister(4, 16);
        c.RA = 0x80102EFCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF44);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001E2u;
        c.RA = 0x80102F08u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.RA = 0x80102F10u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V1 = c.V0 << 2;
        c.V0 = 0xFFFFFFFEu;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.V1 + 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V1);
        c.V1 = 0x00000001u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V1);
        c.V1 = 0x8FFF196Cu;
        c.LoadWord(4, m, (c.S0 + 0x38u));
        c.V1 = c.V1 - 0x184Cu;
        c.StoreWord(3, m, (c.A0 + 0x64u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002AC5C);
        c.V1 = c.V0 << 2;
        c.V0 = 0xFFFFFFFEu;
        c.V1 = (uint)((int)c.V1 >> 15);
        c.V1 = c.V1 + 0x1u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V1);
        c.V1 = 0x8FFF196Cu;
        c.LoadWord(4, m, (c.S0 + 0x38u));
        c.V1 = c.V1 - 0x184Cu;
        m.WriteU8((c.S0 + 0x8u), (byte)0u);
        c.StoreWord(3, m, (c.A0 + 0x64u));
        goto L80102FC8;
        L80102F98: ;
        c.V0 = 0x00000001u;
        m.WriteU8((c.S0 + 0x8u), (byte)c.V0);
        c.LoadWord(2, m, c.S0);
        c.V1 = 0x00000004u;
        m.WriteU8((c.S0 + 0x9u), (byte)c.V1);
        c.LoadWord(3, m, (c.S0 + 0x38u));
        c.V0 = c.V0 | 0x0080u;
        c.StoreWord(2, m, c.S0);
        c.V0 = 0x8FFF196Cu;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x64u));
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
        switch (RecompOne.Runtime.Sdk.V82DreamlandCompat.NormalizeLinkedAddress(m, c.V0))
        {
            case 0x80103028u: goto L80103028;
            case 0x801031C8u: goto L801031C8;
            case 0x801031E4u: goto L801031E4;
            case 0x80103054u: goto L80103054;
            case 0x80103198u: goto L80103198;
            case 0x80103174u: goto L80103174;
            default: RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, c.V0); return;
        }
        L80103028: ;
        c.V0 = (uint)(sbyte)m.ReadU8((c.S0 + 0x8u));
        if (c.V0 != 0u) {
            c.CopyRegister(2, 0);
            goto L801031E8;
        }
        c.CopyRegister(2, 0);
        c.V0 = m.ReadU16((c.S0 + 0x42u));
        c.V0 = c.V0 + 0x4u;
        if (c.S4 == 0u) {
            m.WriteU16((c.S0 + 0x42u), (ushort)c.V0);
            goto L801031E4;
        }
        m.WriteU16((c.S0 + 0x42u), (ushort)c.V0);
        c.CopyRegister(4, 16);
        c.RA = 0x8010304Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002CF44);
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
        c.S2 = c.S0 + 0x24u;
        c.RA = 0x80103084u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.A2 = 0x0000004Eu;
        c.LoadWord(2, m, (c.S0 + 0x58u));
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 18);
        c.RA = 0x8010309Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 16);
        c.A1 = 0x000001A0u;
        c.RA = 0x801030A8u;
        Vigilante82PC.DreamActorDestroy(c, m);
        c.V0 = 0x8FFF196Cu;
        c.LoadWord(3, m, (c.S0 + 0x38u));
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x64u));
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8002A168);
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
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8003A020);
        c.CopyRegister(4, 18);
        c.A1 = 0x80800000u;
        c.A1 = c.A1 | 0x8008u;
        c.RA = 0x80103140u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8004E414);
        c.RA = 0x80103148u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_8001DD9C);
        c.CopyRegister(4, 2);
        c.LoadWord(2, m, (c.S0 + 0x58u));
        c.A2 = 0x00000045u;
        c.LoadWord(5, m, (c.V0 + 0x8u));
        c.CopyRegister(7, 18);
        c.RA = 0x80103160u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.Dispatch(c, m, 0x8FFF0114u);
        c.CopyRegister(4, 16);
        L80103164: ;
        c.CopyRegister(5, 20);
        c.RA = 0x8010316Cu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallEngine(c, m, Vigilante82PC.func_80032CF0);
        goto L80103180;
        L80103174: ;
        c.CopyRegister(4, 16);
        c.CopyRegister(5, 20);
        c.RA = 0x80103180u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.CallLegacyCooldown(c, m, Vigilante82PC.func_80032B90);
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
        c.V1 = 0x8FFF196Cu;
        c.LoadWord(4, m, (c.S0 + 0x38u));
        c.V1 = c.V1 - 0x184Cu;
        m.WriteU8((c.S0 + 0x8u), (byte)0u);
        c.StoreWord(3, m, (c.A0 + 0x64u));
        goto L801031E8;
        L801031C8: ;
        c.LoadWord(2, m, c.S0);
        c.LoadWord(3, m, (c.S0 + 0x38u));
        c.V0 = c.V0 | 0x0080u;
        c.StoreWord(2, m, c.S0);
        c.V0 = 0x8FFF196Cu;
        c.V0 = c.V0 - 0x184Cu;
        c.StoreWord(2, m, (c.V1 + 0x64u));
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
            [0x801002C0u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamContactActor),
            [0x8010063Cu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamLnd),
            [0x801009E8u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Butterfly),
            [0x80100A3Cu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.cobblebridge),
            [0x80100B84u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DBridge),
            [0x80100CACu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Rainbow),
            [0x80100E1Cu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.well),
            [0x801010A8u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.castle),
            [0x80101248u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Canon),
            [0x8010150Cu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamProjectileSpawn),
            [0x801015ECu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamGoodProjectile),
            [0x80101904u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.mushroom_good),
            [0x80101B78u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamBadProjectile),
            [0x80101E30u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.mushroom_bad),
            [0x801020ECu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamOrbitingActor),
            [0x80102200u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamActorDestroy),
            [0x80102254u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamActorBurst),
            [0x801023C8u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamActorPathStart),
            [0x8010243Cu] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.DreamActorPathStep),
            [0x80102690u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Knight),
            [0x80102880u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Footman_Walk),
            [0x80102B50u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Chicken_Walk),
            [0x80102FE0u] = RecompOne.Runtime.Sdk.V82DreamlandCompat.Wrap(Vigilante82PC.Piggy),
        };
}
