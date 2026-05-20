// addr: 0x80056030  name: PADPORTD_OBJ_45C

/* WARNING: Removing unreachable block (ram,0x80056084) */
/* WARNING: Removing unreachable block (ram,0x80056094) */
/* WARNING: Removing unreachable block (ram,0x800560a4) */
/* WARNING: Removing unreachable block (ram,0x800560a8) */

void PADPORTD_OBJ_45C(undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  byte *pbVar2;
  byte *pbVar3;
  byte bVar4;
  uint in_t0;
  int in_t1;
  int in_t2;
  int unaff_s0;
  
  do {
    do {
      if ((param_3 != 0) &&
         (iVar1 = DAT_800652a4 + (uint)*(byte *)(in_t2 + *(int *)(unaff_s0 + 4) + 3), iVar1 < 0x3d))
      {
        DAT_800652a4 = iVar1;
        PADPORTD_OBJ_4A8();
        return;
      }
      in_t0 = in_t0 + 1;
      in_t2 = in_t2 + 5;
      if ((int)(uint)*(byte *)(unaff_s0 + 0xe9) <= (int)in_t0) {
        PADPORTD_OBJ_5F4();
        return;
      }
      param_3 = 0;
      bVar4 = 1;
      if (*(char *)(in_t2 + *(int *)(unaff_s0 + 4) + 2) != '\0') {
        bVar4 = 0xff;
      }
      pbVar3 = (byte *)(unaff_s0 + 0x5d);
      pbVar2 = *(byte **)(unaff_s0 + 0x28);
      iVar1 = 0;
    } while (in_t1 == 0);
    do {
      if ((*pbVar3 == in_t0) && ((*pbVar2 & bVar4) != 0)) {
        param_3 = 1;
        break;
      }
      pbVar3 = pbVar3 + 1;
      iVar1 = iVar1 + 1;
      pbVar2 = pbVar2 + 1;
    } while (iVar1 < in_t1);
  } while( true );
}

