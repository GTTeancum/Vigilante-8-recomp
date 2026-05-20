// addr: 0x8004f4e8  name: SetDispMask

void SetDispMask(int mask)

{
  undefined4 uVar1;
  
  if (1 < DAT_80065026) {
    printf("SetDispMask(%d)...\n",mask);
  }
  if (mask == 0) {
    SYS_OBJ_2FE4(&DAT_80065090,0xffffffff,0x14);
  }
  uVar1 = 0x3000001;
  if (mask != 0) {
    uVar1 = 0x3000000;
  }
  SYS_OBJ_20DC(uVar1);
  return;
}

