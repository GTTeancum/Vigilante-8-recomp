// addr: 0x8005607c  name: PADPORTD_OBJ_4A8

void PADPORTD_OBJ_4A8(undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined1 *puVar2;
  byte *pbVar3;
  byte *pbVar4;
  bool bVar5;
  byte bVar6;
  uint in_t0;
  int in_t1;
  int in_t2;
  undefined1 in_t3;
  int unaff_s0;
  
  do {
    if (param_3 != 0) {
      pbVar4 = (byte *)(unaff_s0 + 0x5d);
      puVar2 = (undefined1 *)(unaff_s0 + 0x57);
      iVar1 = 0;
      if (in_t1 != 0) {
        do {
          bVar6 = *pbVar4;
          pbVar4 = pbVar4 + 1;
          if (bVar6 == in_t0) {
            *puVar2 = in_t3;
          }
          iVar1 = iVar1 + 1;
          puVar2 = puVar2 + 1;
        } while (iVar1 < in_t1);
      }
    }
    do {
      in_t0 = in_t0 + 1;
      in_t2 = in_t2 + 5;
      if ((int)(uint)*(byte *)(unaff_s0 + 0xe9) <= (int)in_t0) {
        PADPORTD_OBJ_5F4();
        return;
      }
      bVar5 = false;
      bVar6 = 1;
      if (*(char *)(in_t2 + *(int *)(unaff_s0 + 4) + 2) != '\0') {
        bVar6 = 0xff;
      }
      pbVar3 = (byte *)(unaff_s0 + 0x5d);
      pbVar4 = *(byte **)(unaff_s0 + 0x28);
      iVar1 = 0;
      if (in_t1 != 0) {
        do {
          if ((*pbVar3 == in_t0) && ((*pbVar4 & bVar6) != 0)) {
            bVar5 = true;
            break;
          }
          pbVar3 = pbVar3 + 1;
          iVar1 = iVar1 + 1;
          pbVar4 = pbVar4 + 1;
        } while (iVar1 < in_t1);
      }
    } while (!bVar5);
    iVar1 = DAT_800652a4 + (uint)*(byte *)(in_t2 + *(int *)(unaff_s0 + 4) + 3);
    if (iVar1 < 0x3d) {
      DAT_800652a4 = iVar1;
      PADPORTD_OBJ_4A8();
      return;
    }
    param_3 = 0;
  } while( true );
}

