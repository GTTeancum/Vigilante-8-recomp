// addr: 0x80018470  name: FUN_80018470

short * FUN_80018470(int param_1,int param_2)

{
  uint uVar1;
  short *psVar2;
  short *psVar3;
  
  psVar3 = psRam000006c4;
  do {
    while( true ) {
      if (psVar3 == (short *)0x0) {
        return (short *)0x0;
      }
      uVar1 = *(uint *)(psVar3 + 4);
      if (uVar1 != 1) break;
      if ((*psVar3 == param_1) && (psVar3[1] == param_2)) {
        return psVar3;
      }
LAB_800184ec:
      do {
        psVar2 = psVar3;
        if (*(int *)(psVar2 + 6) == 0) break;
        psVar3 = *(short **)(psVar2 + 6);
      } while (psVar2 == *(short **)(*(short **)(psVar2 + 6) + 10));
      if (*(int *)(psVar2 + 6) == 0) {
        return (short *)0x0;
      }
      psVar3 = *(short **)(*(int *)(psVar2 + 6) + 0x14);
    }
    if (uVar1 == 0) goto LAB_800184ec;
    if (uVar1 < 4) {
      psVar3 = *(short **)(psVar3 + 8);
    }
  } while( true );
}

