// addr: 0x80018124  name: FUN_80018124

short * FUN_80018124(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6)

{
  bool bVar1;
  int iVar2;
  short *psVar3;
  uint uVar4;
  uint uVar5;
  short *psVar6;
  uint uVar7;
  
  uVar5 = param_3 - 1;
  uVar7 = param_4 - 1;
  psVar3 = psRam000006c4;
joined_r0x80018164:
  while (psVar6 = psVar3, psVar6 != (short *)0x0) {
    iVar2 = *(int *)(psVar6 + 4);
    if (iVar2 == 1) goto LAB_800182d8;
    if (iVar2 == 0) {
      if ((param_1 <= psVar6[2]) && (param_2 <= psVar6[3])) {
        uVar4 = (uint)*psVar6;
        bVar1 = param_5 - param_1 < (int)(uVar4 & uVar5);
        if (((!bVar1) || ((int)(param_1 + (-uVar4 & uVar5)) <= (int)psVar6[2])) &&
           (((int)((int)psVar6[1] & uVar7) <= param_6 - param_2 ||
            ((int)(param_2 + (-(int)psVar6[1] & uVar7)) <= (int)psVar6[3])))) {
          if (bVar1) {
            FUN_80017ec4(psVar6,-uVar4 & uVar5);
            psVar6 = *(short **)(psVar6 + 10);
          }
          if (param_1 < psVar6[2]) {
            FUN_80017ec4(psVar6,param_1);
            psVar6 = *(short **)(psVar6 + 8);
          }
          if (param_6 - param_2 < (int)((int)psVar6[1] & uVar7)) {
            FUN_80017f4c(psVar6,-(int)psVar6[1] & uVar7);
            psVar6 = *(short **)(psVar6 + 10);
          }
          if (param_2 < psVar6[3]) {
            FUN_80017f4c(psVar6,param_2);
            psVar6 = *(short **)(psVar6 + 8);
          }
          psVar6[4] = 1;
          psVar6[5] = 0;
          return psVar6;
        }
      }
      goto LAB_800182d8;
    }
    if (iVar2 == 2) goto LAB_80018304;
    psVar3 = psVar6;
    if ((iVar2 == 3) &&
       ((psVar3 = *(short **)(psVar6 + 8), *(int *)(psVar3 + 4) == 1 || (psVar3[3] < param_2)))) {
      psVar3 = *(short **)(psVar6 + 10);
      if (*(int *)(psVar3 + 4) == 1) goto LAB_800182d8;
      bVar1 = psVar3[3] < param_2;
      goto LAB_800183a4;
    }
  }
LAB_800183bc:
  FUN_80015368("Out of VRAM");
  return (short *)0x0;
LAB_80018304:
  psVar3 = *(short **)(psVar6 + 8);
  if ((*(int *)(psVar3 + 4) == 1) || (psVar3[2] < param_1)) {
    psVar3 = *(short **)(psVar6 + 10);
    if (*(int *)(psVar3 + 4) != 1) {
      bVar1 = psVar3[2] < param_1;
LAB_800183a4:
      if (!bVar1) goto joined_r0x80018164;
    }
LAB_800182d8:
    do {
      psVar3 = psVar6;
      if (*(int *)(psVar3 + 6) == 0) break;
      psVar6 = *(short **)(psVar3 + 6);
    } while (psVar3 == *(short **)(*(short **)(psVar3 + 6) + 10));
    if (*(int *)(psVar3 + 6) == 0) goto LAB_800183bc;
    psVar3 = *(short **)(*(int *)(psVar3 + 6) + 0x14);
  }
  goto joined_r0x80018164;
}

