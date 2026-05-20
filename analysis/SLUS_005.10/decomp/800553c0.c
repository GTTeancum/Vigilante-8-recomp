// addr: 0x800553c0  name: PADCMD_OBJ_72C

undefined4 PADCMD_OBJ_72C(int param_1,byte *param_2,int param_3,int param_4)

{
  byte bVar1;
  undefined4 uVar2;
  int iVar3;
  int in_v1;
  byte *pbVar4;
  uint in_t0;
  uint in_t1;
  int in_t2;
  undefined1 in_t3;
  
  while( true ) {
    do {
      in_v1 = in_v1 + 1;
      param_3 = param_3 + 1;
      if (5 < in_v1) {
        in_t0 = in_t0 + 1;
        in_t2 = in_t2 + 5;
        if ((int)(uint)*(byte *)(param_1 + 0xe9) <= (int)in_t0) {
          *(undefined1 *)(param_1 + 0x46) = 0xfe;
          return 0;
        }
        pbVar4 = *(byte **)(param_1 + 0x20);
        param_4 = 0;
        iVar3 = 5;
        do {
          bVar1 = *pbVar4;
          pbVar4 = pbVar4 + 1;
          if (bVar1 == in_t0) {
            param_4 = param_4 + 1;
          }
          iVar3 = iVar3 + -1;
        } while (-1 < iVar3);
        in_t1 = (uint)*(byte *)(in_t2 + *(int *)(param_1 + 4) + 2);
        param_2 = *(byte **)(param_1 + 0x20);
        in_v1 = 0;
        param_3 = param_1;
        if (in_t1 == 0) {
          in_t1 = 1;
        }
      }
      bVar1 = *param_2;
      param_2 = param_2 + 1;
    } while (bVar1 != in_t0);
    if (param_4 < (int)in_t1) break;
    *(char *)(param_3 + 0x5d) = (char)in_t0;
  }
  *(undefined1 *)(param_3 + 0x5d) = in_t3;
  uVar2 = PADCMD_OBJ_72C();
  return uVar2;
}

