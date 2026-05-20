// addr: 0x80100854  name: FUN_80100854

undefined4 FUN_80100854(uint *param_1,undefined4 param_2,int *param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  
  switch(param_2) {
  case 0:
    uVar4 = param_1[0xe];
    if (param_3 != (int *)0x0) {
      iVar3 = FUN_8001d624/*0x8001d624*/(uVar4);
      uVar5 = FUN_800449bc/*0x800449bc*/(iVar3 + 0x14);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
    }
    if (((char)param_1[2] != '\0') &&
       (iVar3 = *(int *)(uVar4 + 0x28) + -0x393, *(int *)(uVar4 + 0x28) = iVar3,
       *(int *)(uVar4 + 0x4c) <= iVar3)) {
      return 0;
    }
    iVar3 = *(int *)(uVar4 + 0x28) + 0x393;
    *(int *)(uVar4 + 0x28) = iVar3;
    if (iVar3 < -0x7fff) {
      return 0;
    }
    iVar3 = FUN_8001d624/*0x8001d624*/(uVar4);
    FUN_800447e8/*0x800447e8*/((int)*(char *)((int)param_1 + 5),*(undefined4 *)(param_1[0x16] + 8),1,
                    iVar3 + 0x14);
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020778/*0x80020778*/(param_1);
    cVar1 = '\x01' - (char)param_1[2];
    *(char *)(param_1 + 2) = cVar1;
    if (cVar1 == '\0') {
      return 0;
    }
    FUN_80020890/*0x80020890*/(param_1,300);
    break;
  default:
    goto switchD_80100890_caseD_1;
  case 2:
    goto switchD_80100890_caseD_2;
  case 3:
    break;
  case 4:
    goto switchD_80100890_caseD_4;
  case 8:
    goto switchD_80100890_caseD_8;
  }
  iVar3 = *param_3;
  cVar1 = *(char *)(iVar3 + 4);
  if ((((cVar1 == '\x02') && (param_3[3] == param_1[0xe])) && ((char)param_1[2] == '\0')) &&
     ((*param_1 & 0x80) != 0)) {
    iVar2 = FUN_8001d624/*0x8001d624*/();
    FUN_800447e8/*0x800447e8*/((int)*(char *)((int)param_1 + 5),*(undefined4 *)(param_1[0x16] + 8),1,
                    iVar2 + 0x14);
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020778/*0x80020778*/(param_1);
    *(undefined1 *)(param_1 + 2) = 1;
    FUN_80020890/*0x80020890*/(param_1,0x1e);
    cVar1 = *(char *)(iVar3 + 4);
  }
  if (cVar1 == '\a') {
    param_3 = (int *)(uint)*(ushort *)(iVar3 + 0xc);
switchD_80100890_caseD_8:
    iVar3 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar3 != 0) {
      *(undefined1 *)(param_1 + 2) = 0xff;
      if ((*param_1 & 0x80) != 0) {
        FUN_80020778/*0x80020778*/(param_1);
      }
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      *(undefined1 *)((int)param_1 + 5) = 0;
    }
    if (((char)param_1[2] == '\0') &&
       (uVar4 = (uint)*(byte *)((int)param_1 + 9) + (int)param_3,
       *(char *)((int)param_1 + 9) = (char)uVar4, 0x31 < (uVar4 & 0xff))) {
      *(undefined1 *)((int)param_1 + 9) = 0;
switchD_80100890_caseD_2:
      if ((param_1[0xe] != 0) && ((*param_1 & 0x80) == 0)) {
        FUN_80020744/*0x80020744*/(param_1);
        uVar5 = FUN_8004410c/*0x8004410c*/();
        iVar3 = FUN_8001d624/*0x8001d624*/(param_1[0xe]);
        FUN_800447e8/*0x800447e8*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),1,iVar3 + 0x14);
        cVar1 = FUN_8004410c/*0x8004410c*/();
        *(char *)((int)param_1 + 5) = cVar1;
        FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),2,0);
switchD_80100890_caseD_4:
        FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      }
    }
  }
switchD_80100890_caseD_1:
  return 0;
}

