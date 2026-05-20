// addr: 0x80041c5c  name: FUN_80041c5c

void FUN_80041c5c(int param_1)

{
  int iVar1;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  
  iVar1 = FUN_8001db54(param_1,(uint)*(ushort *)(*(int *)(param_1 + 0x18) + 0x24) <<
                               (0x10 - *(ushort *)(*(int *)(param_1 + 0x18) + 0x26) & 0x1f));
  if ((iVar1 != 0) && (FUN_80043408(&DAT_8006f680,param_1,&local_40), local_38 < 0x200000)) {
    local_30 = DAT_8006f680;
    local_2c = DAT_8006f684;
    local_28 = DAT_8006f688;
    local_24 = DAT_8006f68c;
    local_20 = DAT_8006f690;
    local_1c = local_40;
    local_18 = local_3c;
    local_14 = local_38;
    FUN_8001be5c(*(undefined4 *)(param_1 + 0x18),&local_30,uRam0000060c);
  }
  return;
}

