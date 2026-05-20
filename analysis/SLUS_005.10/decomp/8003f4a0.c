// addr: 0x8003f4a0  name: FUN_8003f4a0

uint * FUN_8003f4a0(int *param_1,ushort param_2,MATRIX *param_3,int param_4)

{
  ushort uVar1;
  MATRIX *m1;
  uint *puVar2;
  uint *puVar3;
  code *pcVar4;
  int iVar5;
  undefined1 uVar6;
  uint uVar7;
  ushort *puVar8;
  MATRIX local_48;
  
  puVar3 = (uint *)0x0;
  do {
    if (param_2 == 0xffff) {
      return puVar3;
    }
    puVar8 = (ushort *)(*param_1 + (uint)param_2 * 0x1c + 0x1c);
    m1 = (MATRIX *)FUN_8001b07c(&local_48,puVar8);
    CompMatrixLV(param_3,m1,&local_48);
    puVar2 = puVar3;
    if (*puVar8 == 0x8502) {
      if (param_4 != 0) {
        puVar2 = (uint *)FUN_80040a80(DAT_800737d8,0x12,param_3->t);
        *puVar2 = *puVar2 | 0x20000;
        FUN_80020744(puVar2);
        FUN_800207c4(puVar2);
        FUN_80020890(puVar2,0x1e0);
        if (puVar3 != (uint *)0x0) {
          puVar2[0xd] = (uint)puVar3;
          puVar3[0xf] = (uint)puVar2;
        }
      }
      goto switchD_8003f5dc_caseD_1;
    }
    if ((*puVar8 == 0xffff) && (param_4 != 0)) {
LAB_8003f600:
      puVar2 = (uint *)FUN_8001ac44(param_1,param_2,0x80,8);
      puVar2[0x19] = (uint)&LAB_8003e868;
      if (puVar3 != (uint *)0x0) {
        puVar2[0xd] = (uint)puVar3;
        puVar3[0xf] = (uint)puVar2;
      }
      goto switchD_8003f5dc_caseD_1;
    }
    switch(*puVar8 >> 0xc) {
    case 0:
      if ((param_4 != 0) && (puVar8[0xb] != 0xaaaa)) goto LAB_8003f600;
      puVar3 = (uint *)FUN_8001aaa8(param_1,puVar8,0x94);
LAB_8003f684:
      uVar1 = FUN_80017160();
      *(ushort *)(puVar3 + 0x20) = uVar1 & 0xff;
      uVar1 = FUN_80017160();
      *(ushort *)((int)puVar3 + 0x82) = uVar1 & 0xff;
      uVar1 = FUN_80017160();
      *(ushort *)(puVar3 + 0x21) = uVar1 & 0xff;
      goto LAB_8003f6ac;
    case 8:
    case 9:
    case 0xe:
      if ((puVar8[0xb] == 0xaaaa) || (puVar8[0xb] == 0)) {
        FUN_8003f0b4(param_1,param_2,&local_48);
      }
      else {
        iVar5 = FUN_8001d470(0x80);
        *(undefined4 *)(iVar5 + 0x10) = local_48.m[0]._0_4_;
        *(undefined4 *)(iVar5 + 0x14) = local_48.m._4_4_;
        *(undefined4 *)(iVar5 + 0x18) = local_48.m[1]._2_4_;
        *(undefined4 *)(iVar5 + 0x1c) = local_48.m[2]._0_4_;
        *(undefined4 *)(iVar5 + 0x20) = local_48._16_4_;
        *(long *)(iVar5 + 0x24) = local_48.t[0];
        *(long *)(iVar5 + 0x28) = local_48.t[1];
        *(long *)(iVar5 + 0x2c) = local_48.t[2];
        *(int **)(iVar5 + 0x58) = param_1;
        *(ushort *)(iVar5 + 10) = param_2;
        *(undefined1 **)(iVar5 + 100) = &LAB_8003f45c;
        *(ushort *)(iVar5 + 0xc) = puVar8[0xb];
        FUN_80020890(iVar5,puVar8[0xb]);
      }
      break;
    case 0xd:
      *puVar8 = *puVar8 & 0xfff;
      puVar3 = (uint *)FUN_8001aaa8(param_1,puVar8,0x94);
      *puVar8 = *puVar8 | 0xd000;
      if (puVar8[0xb] != 0) goto LAB_8003f684;
      *(undefined2 *)(puVar3 + 0x20) = 0;
      *(undefined2 *)((int)puVar3 + 0x82) = 0;
      *(undefined2 *)(puVar3 + 0x21) = 0;
LAB_8003f6ac:
      uVar7 = *puVar3;
      *(undefined1 *)(puVar3 + 1) = 1;
      *puVar3 = uVar7 | 0x180;
      if (puVar3[0x17] == 0) {
        *puVar3 = uVar7 | 0x1a0;
      }
      if ((*puVar3 & 0x10) == 0) {
        pcVar4 = FUN_8003eab0;
      }
      else {
        pcVar4 = (code *)&LAB_8003e8a0;
      }
      puVar3[0x19] = (uint)pcVar4;
      puVar3[0x22] = (*(int *)(puVar8 + 2) << 8) >> 0xc;
      puVar3[0x23] = (*(int *)(puVar8 + 4) << 8) >> 0xc;
      puVar3[0x24] = (*(int *)(puVar8 + 6) << 8) >> 0xc;
      FUN_80043358(param_3,puVar3 + 0x22,puVar3 + 0x22);
      puVar3[4] = local_48.m[0]._0_4_;
      puVar3[5] = local_48.m._4_4_;
      puVar3[6] = local_48.m[1]._2_4_;
      puVar3[7] = local_48.m[2]._0_4_;
      puVar3[8] = local_48._16_4_;
      puVar3[9] = local_48.t[0];
      puVar3[10] = local_48.t[1];
      puVar3[0xb] = local_48.t[2];
      FUN_8001dc1c(puVar3);
      uVar6 = 3;
      if ((0x27ff < (int)puVar3[0x15]) && (uVar6 = 1, (int)puVar3[0x15] < 0x7800)) {
        uVar6 = 2;
      }
      *(undefined1 *)((int)puVar3 + 0x87) = uVar6;
      FUN_800202f4(puVar3);
    }
switchD_8003f5dc_caseD_1:
    param_2 = puVar8[0xc];
    puVar3 = puVar2;
  } while( true );
}

