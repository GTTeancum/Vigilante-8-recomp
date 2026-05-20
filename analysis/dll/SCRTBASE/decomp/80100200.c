// addr: 0x80100200  name: FUN_80100200

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100200(int param_1,uint param_2,int *param_3)

{
  ushort uVar1;
  int *piVar2;
  int *piVar3;
  char cVar4;
  undefined4 uVar5;
  int iVar6;
  
  if (param_2 != 2) {
    if ((2 < param_2) || (iVar6 = -0x7ffa0000, param_2 != 1)) {
      if (param_2 == 0x11) goto LAB_801003c8;
      iVar6 = 1;
    }
    DAT_80102bcc = 0;
    uVar5 = FUN_8001ffd4/*0x8001ffd4*/(iVar6 + 0x5a50,0x100);
    iVar6 = func_0x8003d080(0x7f000000,uVar5);
    _DAT_80065a10 = (uint)(iVar6 != 0);
    *(undefined1 *)(param_1 + 8) = 0x1e;
  }
  uVar1 = *(ushort *)(param_1 + 0xc);
  *(ushort *)(param_1 + 0xc) = uVar1 + 1;
  if ((uVar1 & 3) == 0) {
    func_0x80023d00();
  }
  cVar4 = *(char *)(param_1 + 8) + -1;
  *(char *)(param_1 + 8) = cVar4;
  iVar6 = (int)cVar4;
  if (iVar6 == 0) {
    iVar6 = -0x7ffa0000;
    param_3 = (int *)0x1;
  }
  if (iVar6 < 0) {
    piVar3 = (int *)*_DAT_80065a18;
    param_3 = _DAT_80065a18;
    while (piVar2 = piVar3, piVar2 != (int *)0x0) {
      iVar6 = param_3[2];
      if (((*(char *)(iVar6 + 4) == '\x02') && (*(short *)(iVar6 + 0xc) != 0)) &&
         (*(int *)(iVar6 + 0x50) < 0x4b32000)) {
        FUN_8002002c/*0x8002002c*/(&DAT_80065a18,9);
      }
      param_3 = piVar2;
      piVar3 = (int *)*piVar2;
    }
    if (*(char *)(param_1 + 8) == -4) {
      iVar6 = FUN_80017160/*0x80017160*/(param_1);
      *(char *)(param_1 + 8) = (char)(iVar6 * 0x1e >> 0xf) + '\x1e';
      param_3 = (int *)0x0;
      FUN_8002123c/*0x8002123c*/(_DAT_80065a00,&LAB_801001ac,0);
    }
  }
  FUN_80020890/*0x80020890*/(param_1,0x3c);
  param_1 = 1;
LAB_801003c8:
  func_0x80022120(param_1,param_3);
  return 0;
}

