// addr: 0x801011a0  name: FUN_801011a0

/* WARNING: Removing unreachable block (ram,0x80101294) */
/* WARNING: Removing unreachable block (ram,0x80101254) */
/* WARNING: Removing unreachable block (ram,0x80101274) */
/* WARNING: Removing unreachable block (ram,0x8010123c) */

void FUN_801011a0(char *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  int *piVar2;
  undefined1 auStack_40 [16];
  undefined1 auStack_30 [8];
  char local_28;
  char local_27;
  char local_26;
  
  FUN_80043224/*0x80043224*/(param_3,auStack_40);
  GTE_RotateSV/*0x8004316c*/(param_4,auStack_30);
  NormalColorCol/*0x8004d658*/(auStack_30,param_2,&local_28);
  for (piVar2 = (int *)*DAT_80107d90; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
  }
  cVar1 = -1;
  if (local_28 != -1) {
    cVar1 = local_28;
  }
  *param_1 = cVar1;
  cVar1 = -1;
  if (local_27 != -1) {
    cVar1 = local_27;
  }
  param_1[1] = cVar1;
  cVar1 = -1;
  if (local_26 != -1) {
    cVar1 = local_26;
  }
  param_1[2] = cVar1;
  return;
}

