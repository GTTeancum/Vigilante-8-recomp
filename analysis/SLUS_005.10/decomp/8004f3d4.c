// addr: 0x8004f3d4  name: SetGraphQueue

byte SetGraphQueue(uint param_1)

{
  byte bVar1;
  
  bVar1 = DAT_80065025;
  if (1 < DAT_80065026) {
    printf("SetGrapQue(%d)...\n",param_1);
  }
  if (param_1 != DAT_80065025) {
    SYS_OBJ_2700(1);
    DAT_80065025 = (byte)param_1;
    DMACallback(2,0);
  }
  return bVar1;
}

