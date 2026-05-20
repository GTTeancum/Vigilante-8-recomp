// addr: 0x80019370  name: FUN_80019370

int FUN_80019370(int *param_1,void *param_2,uint param_3,int param_4,undefined2 param_5)

{
  byte bVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  short *psVar5;
  
  iVar4 = *param_1;
  bVar1 = *(byte *)(iVar4 + 5);
  *(undefined1 *)((int)param_2 + 3) = 1;
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
  *(uint *)((int)param_2 + 4) = *(ushort *)(param_1 + 4) & 0x9ff | 0xe1000400;
  iVar3 = param_1[1];
  *(undefined4 *)((int)param_2 + 8) = 0x4000000;
  *(int *)((int)param_2 + 0xc) = iVar3;
  *(undefined2 *)((int)param_2 + 0x16) = *(undefined2 *)((int)param_1 + 0x12);
  *(ushort *)((int)param_2 + 0x1a) = (ushort)*(byte *)(*param_1 + 6);
  psVar5 = (short *)(iVar4 + ((param_3 & 0xff) - (uint)bVar1) * 5 + 8);
  *(short *)((int)param_2 + 0x14) = *(short *)((int)param_1 + 0xe) + *psVar5;
  sVar2 = psVar5[2];
  *(undefined2 *)((int)param_2 + 0x12) = param_5;
  *(short *)((int)param_2 + 0x10) = (short)(char)sVar2 + (short)param_4;
  *(ushort *)((int)param_2 + 0x18) = (ushort)*(byte *)(psVar5 + 1);
  MargePrim(param_2,(void *)((int)param_2 + 8));
  return param_4 + (uint)*(byte *)((int)psVar5 + 3);
}

