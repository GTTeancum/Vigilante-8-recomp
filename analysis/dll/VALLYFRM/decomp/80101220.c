// addr: 0x80101220  name: FUN_80101220

undefined4 FUN_80101220(int param_1,uint param_2,undefined4 param_3)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  undefined4 uVar4;
  
  if (param_2 == 3) {
LAB_80101268:
    iVar3 = FUN_8002239c/*0x8002239c*/(param_1,param_3);
    if (iVar3 != 0) goto LAB_801012b0;
LAB_80101284:
    iVar3 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar3 != 0) goto LAB_801012b0;
  }
  else if ((3 < param_2) || (param_2 != 1)) {
    if (param_2 != 8) goto LAB_80101268;
    goto LAB_80101284;
  }
  uVar2 = FUN_8003fc94/*0x8003fc94*/(param_1);
  *(undefined1 *)(param_1 + 8) = uVar2;
LAB_801012b0:
  cVar1 = *(char *)(param_1 + 8);
  *(char *)(param_1 + 8) = cVar1 + -1;
  if (cVar1 == '\x01' && DAT_80101308 < 2) {
    DAT_80101308 = DAT_80101308 + 1;
    uVar4 = func_0x80021d6c(1000);
    FUN_8002036c/*0x8002036c*/(uVar4);
  }
  return 0;
}

