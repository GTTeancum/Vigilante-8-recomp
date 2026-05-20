// addr: 0x8004f378  name: SetGraphDebug

int SetGraphDebug(int level)

{
  uint uVar1;
  
  uVar1 = (uint)DAT_80065026;
  DAT_80065026 = (byte)level;
  if ((level & 0xffU) != 0) {
    printf("SetGraphDebug:level:%d,type:%d reverse:%d\n",level & 0xff,(uint)DAT_80065024,0);
  }
  return uVar1;
}

