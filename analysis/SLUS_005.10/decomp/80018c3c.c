// addr: 0x80018c3c  name: FUN_80018c3c

void FUN_80018c3c(void *param_1,int param_2)

{
  ushort uVar1;
  
  *(undefined1 *)((int)param_1 + 3) = 1;
  uVar1 = *(ushort *)(param_2 + 8);
  *(undefined1 *)((int)param_1 + 0xb) = 4;
                    /* Possible PsyQ macro: setSprt() + setShadeTex(sprt, 1) */
  *(undefined1 *)((int)param_1 + 0xf) = 0x65;
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
  *(uint *)((int)param_1 + 4) = uVar1 & 0x9ff | 0xe1000400;
  *(undefined2 *)((int)param_1 + 0x16) = *(undefined2 *)(param_2 + 10);
  *(undefined2 *)((int)param_1 + 0x18) = *(undefined2 *)(param_2 + 2);
  *(undefined2 *)((int)param_1 + 0x1a) = *(undefined2 *)(param_2 + 4);
  *(undefined2 *)((int)param_1 + 0x14) = *(undefined2 *)(param_2 + 6);
  MargePrim(param_1,(void *)((int)param_1 + 8));
  return;
}

